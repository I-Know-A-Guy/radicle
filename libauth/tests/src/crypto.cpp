/* LIBRADICLE - The Radicle Library
 * Copyright (C) 2021 Nils Egger <nilsxegger@gmail.com>
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <https://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>

#include "radicle/auth/types.h"
#include "radicle/auth/crypto.h"
#include "radicle/types/string.h"

TEST(AuthCryptoTests, TestRandomBase64) {
	string_t* buffer = NULL;
	ASSERT_EQ(auth_generate_random_base64(256, &buffer), 0);
	EXPECT_GE(buffer->length, 256);
	EXPECT_EQ(strlen(buffer->ptr), buffer->length);
	string_free(&buffer);
}

class AuthCryptoBase64Test: public ::testing::Test {
	public:
		string_t* raw = NULL;
		string_t* encoded = NULL;
		string_t* buffer = NULL;
	protected:
		void SetUp() override {
			raw = string_from_literal("F4'z@4[rw}96v\":sbGgr+DH#Yf~2$ffq");
			encoded = string_from_literal("RjQnekA0W3J3fTk2diI6c2JHZ3IrREgjWWZ+MiRmZnE=");
		}

		void TearDown() override {
			string_free(&raw);
			string_free(&encoded);
			string_free(&buffer);
		}
};

TEST_F(AuthCryptoBase64Test, TestEncode) {
	ASSERT_EQ(base64_encode((unsigned char*)raw->ptr, raw->length, &buffer), 0);
	EXPECT_STREQ(buffer->ptr, encoded->ptr);
	EXPECT_EQ(buffer->length, strlen(buffer->ptr));
}

TEST_F(AuthCryptoBase64Test, TestDecode) {
	ASSERT_EQ(base64_decode(encoded, &buffer), 0);
	EXPECT_EQ(buffer->length, strlen(buffer->ptr));
	EXPECT_STREQ(buffer->ptr, raw->ptr);
}

class AuthCryptoHmacTest: public ::testing::Test {
	public:
		string_t* raw = NULL;
		string_t* encoded = NULL;
		string_t* key = NULL;
		string_t* key_without_salt = NULL;
		string_t* key_salt = NULL;
		string_t* buffer = NULL;
	protected:
		void SetUp() override {
			raw = string_from_literal("9G'E-cd?wr\"g<_],Cg;YS:NBeF%f.2(td!9\"<gC8aT+,J$W^QHnf$v2Ju?'s+6=]");
			encoded = string_from_literal("49fbf1c7091b0d942a526d883327c017d9d858225468f17df0b6dff9dfaed530f35793eeb644012f910ad010c5bda78ccbd1a2089975b29582ce87eca9c67427");
			key = string_from_literal("r'5)t]=e(ZKC$[)6LZ'5jt/m[EN<>9#xeySuutr/Fd7HG{5C,RZvJPexRssUseq\\");
			key_without_salt = string_from_literal("r'5)t]=e(ZKC$[)6LZ'5jt/m[EN<>9#xeySuutr/Fd7HG{5C,RZvJPexRss");
			key_salt = string_from_literal("Useq\\");
		}

		void TearDown() override {
			string_free(&raw);
			string_free(&encoded);
			string_free(&key);
			string_free(&buffer);
		}

};

TEST_F(AuthCryptoHmacTest, HmacSignTest) {
	ASSERT_EQ(hmac_sign((unsigned char*)raw->ptr, raw->length, key, &buffer), 0);
	EXPECT_EQ(buffer->length, strlen(buffer->ptr));
	EXPECT_STREQ(buffer->ptr, encoded->ptr);
}

TEST_F(AuthCryptoHmacTest, HmacVerifySuccessTest) {
	ASSERT_EQ(hmac_verify(key, encoded, raw), 0);
}

TEST_F(AuthCryptoHmacTest, HmacVerifyFailTest) {
	ASSERT_EQ(hmac_verify(key, encoded, encoded), 1);
}

TEST_F(AuthCryptoHmacTest, HmacVerifySaltedTest) {
	ASSERT_EQ(hmac_verify_salted(key_without_salt, key_salt, encoded, raw), 0);
}

class AuthCryptoCookieTest: public ::testing::Test {
	protected: 

	string_t* key = NULL; 
	auth_cookie_t* cookie = NULL;

	string_t* raw_cookie = NULL;
	auth_cookie_t* decoded_cookie = NULL;

	void SetUp() override {
		key = string_from_literal("Secret Key");
		raw_cookie = string_from_literal("EWq3TGqBhdD8O3ei5sg4RDcfw/OfagYEJSr5hTRPqIsKxcqv2Vs8LrJrWwjz3RtS6Lddp8qHk5X51dGDHPYtK8RbZyYshOF0Le/9ie+xaT0KA8VTqEsJKV+AY/71tkGzx/Y3/zyMwxl09nQnLcxJUH6Qfduqqk/DuLYP6KNHvpQ=-f334d26968d450f91224799c2d82cace7132ee5a804cf4e38b853f9f3672df0aeb29f09109e748e86d1bd39f05f4c7c2b6b32cb9774675dff09f0ec07b68253f");
		decoded_cookie = auth_cookie_new_empty();
		decoded_cookie->token = string_from_literal("EWq3TGqBhdD8O3ei5sg4RDcfw/OfagYEJSr5hTRPqIsKxcqv2Vs8LrJrWwjz3RtS6Lddp8qHk5X51dGDHPYtK8RbZyYshOF0Le/9ie+xaT0KA8VTqEsJKV+AY/71tkGzx/Y3/zyMwxl09nQnLcxJUH6Qfduqqk/DuLYP6KNHvpQ=");
		decoded_cookie->signature = string_from_literal("f334d26968d450f91224799c2d82cace7132ee5a804cf4e38b853f9f3672df0aeb29f09109e748e86d1bd39f05f4c7c2b6b32cb9774675dff09f0ec07b68253f");
	}

	void TearDown() override {
		string_free(&key);
		string_free(&raw_cookie);
		auth_cookie_free(&cookie);
		auth_cookie_free(&decoded_cookie);
	};


};

TEST_F(AuthCryptoCookieTest, GenerateCryptoTest) {
	ASSERT_EQ(auth_generate_session_cookie(key, &cookie), 0);
	EXPECT_EQ(cookie->cookie->length, strlen(cookie->cookie->ptr));
	EXPECT_EQ(cookie->token->length, strlen(cookie->token->ptr));
	EXPECT_EQ(cookie->signature->length, strlen(cookie->signature->ptr));
	EXPECT_EQ(cookie->salt->length, strlen(cookie->salt->ptr));
}

TEST_F(AuthCryptoCookieTest, SplitCookieTest) {
	ASSERT_EQ(auth_split_cookie(raw_cookie, &cookie), 0);
	EXPECT_EQ(cookie->token->length, strlen(cookie->token->ptr));
	EXPECT_STREQ(cookie->token->ptr, decoded_cookie->token->ptr);
	EXPECT_EQ(cookie->signature->length, strlen(cookie->signature->ptr));
	EXPECT_STREQ(cookie->signature->ptr, decoded_cookie->signature->ptr);
}

class AuthCryptoPasswordTest: public ::testing::Test {
	protected:
		string_t* raw = NULL;
		string_t* encoded = NULL;

		void SetUp() override {
			raw = string_from_literal("password");
		}

		void TearDown() override {
			string_free(&raw);
			string_free(&encoded);
		}
};

TEST_F(AuthCryptoPasswordTest, Argon2HashTest) {
	ASSERT_EQ(auth_hash_password(raw, &encoded), 0);
	EXPECT_EQ(encoded->length, strlen(encoded->ptr));
}
