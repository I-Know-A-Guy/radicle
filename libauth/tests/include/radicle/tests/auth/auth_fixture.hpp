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
 * @brief Contains RadicleAuthTests fixture for testing.
 * @author Nils Egger
 * @addtogroup testing 
 * @{
 * @addtogroup auth_testing Auth Testing
 * @{ 
 */

#include <gtest/gtest.h>
#include <libpq-fe.h>
#include <vector>

#include "radicle/pgdb.h"
#include "radicle/auth/setup.h"
#include "radicle/auth/types.h"
#include "radicle/auth/crypto.h"
#include "radicle/auth/db.h"
#include "radicle/auth.h"
#include "radicle/tests/pgdb_hooks.hpp"

/**
 * @brief Fake function which returns every possible column of a account. 
 *
 * @returns Returns 0 and fills PGresult with a fake table.
 */
int pgdb_fetch_param_fake_account(PGconn* conn, const char* stmt, const pgdb_params_t* params, pgdb_result_t** result);

/**
 * @brief Fake function which returns an object containing one row with column
 * account.
 */
int pgdb_fetch_param_fake_token(PGconn* conn, const char* stmt, const pgdb_params_t* params, pgdb_result_t** result);

/**
 * @brief Fake function which returns every column of a owned session. 
 *
 * @returns Returns 0 and fills PGresult with a fake table.
 */
int pgdb_fetch_param_fake_owned_session(PGconn* conn, const char* stmt, const pgdb_params_t* params, pgdb_result_t** result);

/**
 * @brief Generates a fixed random base64.
 *
 * @returns Returns fPtjd+zvvzMafpvYuJC10Q==
 */
int auth_generate_random_base64_fake(const int rand_bytes, string_t** buffer);

/**
 * @brief Returns a fixed argon2i hash.
 *
 * @returns Returns $argon2i$v=19$m=16,t=2,p=1$VXpBNkVOT2h0U3BKV0NHYQ$5lylwjo684I2q5cj2NWhwA
 */
int auth_hash_password_fake(const string_t* password, string_t** buffer);

/**
 * @brief Alwyas returns 0 for success.
 *
 * @returns Returns 0
 */
int auth_verify_password_fake(const string_t* encoded, const string_t* password);

/**
 * @brief Returns a fixed hex string.
 *
 * @returns Returns e65ca5c23a3af38236ab9723d8d5a1c0.
 */
int hmac_sign_fake(const unsigned char* input, const size_t input_length, const string_t* key, string_t** buffer);

/**
 * @brief Class for tests which use any of the Auth functions.
 */
class RadicleAuthTests: public RadiclePGDBHooks {

	std::vector<auth_account_t*> accounts; /**< Account pointers which will be freed on TearDown() */
	std::vector<auth_cookie_t*> cookies; /**< Cookie pointers which will be freed on TearDown() */

	protected:
		PGconn* conn = NULL; /**< Fake connection to database. */
		auth_request_log_t* test_request_log; /**< Common requester which can be used for unit tests. */

		/**
		 * @brief Initializes test_requester and calls RadicleTests::SetUp()
		 */
		void SetUp() override {
			test_request_log = auth_request_log_new();
			test_request_log->ip = string_from_literal("127.0.0.1");
			test_request_log->url = string_from_literal("/i/am/a/test");
			RadicleTests::SetUp();
		}

		/**
		 * @brief Frees accounts, cookies, session, test_requester and call RadicleTests::TearDown()
		 */
		void TearDown() override {
			for(std::vector<auth_account_t*>::iterator iter = accounts.begin(); iter != accounts.end(); iter++) {
				auth_account_free(iter.base());
			}
			for(std::vector<auth_cookie_t*>::iterator iter = cookies.begin(); iter != cookies.end(); iter++) {
				auth_cookie_free(iter.base());
			}
			auth_request_log_free(&test_request_log);
			RadicleTests::TearDown();
		}

		/**
		 * @brief Creates new account object and takes ownership.
		 *
		 * @param email Email of account
		 * @param password Password of account, will no be hashed.
		 * @param role Role of account
		 * @param verified Whetever email should count as verified.
		 *
		 * @return Returns pointer to account object.
		 */
		auth_account_t* manage_account(const char* email, const char* password, const char* role, const bool verified) {
			string_t* email_buf = manage_string(email);
			string_t* password_buf = manage_string(password);
			string_t* role_buf = manage_string(role);
			auth_account_t* buf = auth_account_new(NULL,
					email_buf,
					password_buf,
					role_buf,
				       	true,
				       	verified,
				       	0);
			accounts.push_back(buf);
			return buf;
		}

		/**
		 * @brief Takes ownership of account and frees it on TearDown()
		 *
		 * @param account Pointer to account to take ownership of.
		 */
		void take_account(auth_account_t* account) {
			if(account == NULL) return;
			accounts.push_back(account);
		}

		/**
		 * @brief Takes ownership of cookie and frees it on TearDown()
		 *
		 * @param cookie Pointer to cookie to take ownership of.
		 */
		void take_cookie(auth_cookie_t* cookie) {
			if(cookie == NULL) return;
			cookies.push_back(cookie);
		}

		/**
		 * @brief Replaces \ref pgdb_fetch_param with \ref pgdb_fetch_param_fake_account
		 *
		 * @return Returns pointer to subhook so it can be uninstalled.
		 */
		subhook_t install_fetch_account_hook() {
			subhook_t buf = subhook_new((void*)pgdb_fetch_param, (void*)pgdb_fetch_param_fake_account, SUBHOOK_64BIT_OFFSET);
			install_hook(buf);
			return buf;
		}

		/**
		 * @brief Replaces \ref pgdb_fetch_param with \ref pgdb_fetch_param_fake_owned_session 
		 *
		 * @return Returns pointer to subhook so it can be uninstalled.
		 */
		subhook_t install_fetch_session_hook() {
			subhook_t buf = subhook_new((void*)pgdb_fetch_param, (void*)pgdb_fetch_param_fake_owned_session, SUBHOOK_64BIT_OFFSET);
			install_hook(buf);
			return buf;
		}

		subhook_t install_fetch_token_hook() {
			subhook_t buf = subhook_new((void*)pgdb_fetch_param, (void*)pgdb_fetch_param_fake_token, SUBHOOK_64BIT_OFFSET);
			install_hook(buf);
			return buf;
		}

		/**
		 * @brief Removes all real hashing and random data generation.
		 */
		void install_crypto_hooks() {
			install_hook(subhook_new((void*)auth_generate_random_base64, (void*)auth_generate_random_base64_fake, SUBHOOK_64BIT_OFFSET));	
			install_hook(subhook_new((void*)auth_hash_password, (void*)auth_hash_password_fake, SUBHOOK_64BIT_OFFSET));	
			install_hook(subhook_new((void*)auth_verify_password, (void*)auth_verify_password_fake, SUBHOOK_64BIT_OFFSET));	
			install_hook(subhook_new((void*)hmac_sign, (void*)hmac_sign_fake, SUBHOOK_64BIT_OFFSET));	
		}
};

/**
 * @brief Test suite which initializes database for authentication use.
 */
class RadicleConnectedAuthTests: public RadicleAuthTests {

	protected:

		/**
		 * @brief Initializes database and connection.
		 */
		static void SetUpTestSuite() {
			const char* conninfo = "host=localhost port=5432 dbname=ikag user=postgres password=postgres connect_timeout=10";
			PGconn* temp_conn;
			ASSERT_EQ(pgdb_connect(conninfo, &temp_conn), 0);
			EXPECT_EQ(auth_create_db_tables(temp_conn), 0);
			PQfinish(temp_conn);
		}

		/**
		 * @brief Connect to the database
		 */
		void SetUp() override {
			RadicleAuthTests::SetUp();
			const char* conninfo = "host=localhost port=5432 dbname=ikag user=postgres password=postgres connect_timeout=10";
			ASSERT_EQ(pgdb_connect(conninfo, &conn), 0);
		}

		/**
		 * @brief Clears the connection to the database.
		 */
		void TearDown() override {
			RadicleAuthTests::TearDown();
			PQfinish(conn);
		}

};

/** @} */
/** @} */
