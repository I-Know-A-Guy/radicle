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

/**
 * @file 
 * @brief Contains all tests associated to auth.h
 * @author Nils Egger
 * @todo Missing tests which assure that functions return correctly if wrong account combinations, account is not verified or etc are given.
 */

#include <string.h>

#include <gtest/gtest.h>

#include "radicle/tests/auth/auth_fixture.hpp"
#include "radicle/auth.h"

TEST_F(RadicleAuthTests, TestAuthRegister) {
	install_execute_param_always_success();
	install_fetch_complete_hook();
	install_crypto_hooks();
	auth_account_t* account = manage_account("TestRegisterAccount", "asdf", "user", false);
	auth_cookie_t* cookie = NULL;
	ASSERT_EQ(auth_register(conn, account, test_requester, common_string, &cookie), 0);
	auth_cookie_free(&cookie);
}

/*
TEST_F(RadicleAuthTests, TestRegisterAccount) {
	string_t* key = manage_string("this is my secure key.");
	auth_account_t* account = manage_account("TestRegisterAccount", "asdf", "user", false);
	auth_cookie_t* cookie = NULL;

	ASSERT_EQ(auth_register(conn, account, test_requester, key, &cookie), 0);

	EXPECT_TRUE(*account->uuid->bin != 0);
	test_string_len(cookie->cookie);
	test_string_len(cookie->token);
	test_string_len(cookie->signature);

	auth_cookie_free(&cookie);
}

TEST_F(RadicleAuthTests, TestSignIn) {
	string_t* key = manage_string("this is my secure key.");
	string_t* email = manage_string("TestSignIn");
	string_t* password = manage_string("password");
	auth_account_t* account;
	auth_cookie_t* cookie = NULL;

	register_test_account(email->ptr, password->ptr, true, key, &account, &cookie);

	auth_account_t* logged;
	auth_cookie_t* logged_cookie;
	ASSERT_EQ(auth_sign_in(conn, email, password, test_requester, key, &logged, &logged_cookie), 0);

	EXPECT_EQ(memcmp(logged->uuid->bin, account->uuid->bin, 16), 0);
	EXPECT_STREQ(logged->role->ptr, "user");
	EXPECT_EQ(logged->verified, true);

	auth_account_free(&logged);
	auth_cookie_free(&logged_cookie);
}

*/


