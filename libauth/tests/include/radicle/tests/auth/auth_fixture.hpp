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
 * @todo Move basic pgdb fake functions to a pgdb testing library
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
#include "radicle/tests/subhook.h"
#include "radicle/tests/radicle_fixture.hpp"


/**
 * @brief Fake function for pgdb_execute_param which simply returns success without doing anyhting.
 *
 * @returns Returns 0.
 */
int pgdb_execute_param_fake(PGconn* conn, const char* stmt, const pgdb_params_t* params);

/**
 * @brief Fake function for pgdb_fetch_param which should return an uuid.
 *
 * @returns Returns 0 and sets fills result with a single row containing a NILL uuid.
 */
int pgdb_fetch_param_fake_uuid(PGconn* conn, const char* stmt, const pgdb_params_t* params, pgdb_result_t** result);

/**
 * @brief Fake function for pgdb_fetch_param which should return an id.
 *
 * @returns Returns 0 and sets fills result with a single row containing a column called id with value set to 1.
 */
int pgdb_fetch_param_fake_id(PGconn* conn, const char* stmt, const pgdb_params_t* params, pgdb_result_t** result);

/**
 * @brief Fake function which returns every possible column. 
 *
 * @returns Returns 0 and fills PGresult with a fake table.
 */
int pgdb_fetch_param_fake_complete(PGconn* conn, const char* stmt, const pgdb_params_t* params, pgdb_result_t** result);


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
 * @todo add documentation for ownerships.
 */
class RadicleAuthTests: public RadicleTests {

	std::vector<auth_account_t*> accounts;
	std::vector<auth_cookie_t*> cookies;

	protected:
		PGconn* conn;
		auth_requester_t* test_requester;

		void SetUp() override {
			conn = PQconnectdb("host=localhost port=5432 dbname=mydb connect_timeout=10");
			test_requester = auth_requester_newl("127.0.0.1", "/i/am/a/test");
			RadicleTests::SetUp();
		}

		void TearDown() override {
			PQfinish(conn);
			for(std::vector<auth_account_t*>::iterator iter = accounts.begin(); iter != accounts.end(); iter++) {
				auth_account_free(iter.base());
			}
			for(std::vector<auth_cookie_t*>::iterator iter = cookies.begin(); iter != cookies.end(); iter++) {
				auth_cookie_free(iter.base());
			}
			auth_requester_free(&test_requester);
			RadicleTests::TearDown();
		}

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

		subhook_t install_fetch_uuid_hook() {
			subhook_t buf = subhook_new((void*)pgdb_fetch_param, (void*)pgdb_fetch_param_fake_uuid, SUBHOOK_64BIT_OFFSET);
			install_hook(buf);
			return buf;
		}

		subhook_t install_fetch_id_hook() {
			subhook_t buf = subhook_new((void*)pgdb_fetch_param, (void*)pgdb_fetch_param_fake_id, SUBHOOK_64BIT_OFFSET);
			install_hook(buf);
			return buf;
		}

		subhook_t install_fetch_complete_hook() {
			subhook_t buf = subhook_new((void*)pgdb_fetch_param, (void*)pgdb_fetch_param_fake_complete, SUBHOOK_64BIT_OFFSET);
			install_hook(buf);
			return buf;
		}

		subhook_t install_execute_param_always_success() {
			subhook_t buf = subhook_new((void*)pgdb_execute_param, (void*)pgdb_execute_param_fake, SUBHOOK_64BIT_OFFSET);
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

/** @} */
/** @} */
