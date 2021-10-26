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
#include "radicle/auth/types.h"
#include "radicle/auth/crypto.h"
#include "radicle/auth/db.h"
#include "radicle/auth.h"
#include "radicle/tests/pgdb_hooks.hpp"
#include "radicle/types/uuid.h"

/**
 * @brief Class for tests which use any of the Auth functions.
 */
class RadicleAuthTests: public RadiclePGDBHooks {

	std::vector<auth_account_t*> accounts; /**< Account pointers which will be freed on TearDown() */
	std::vector<auth_file_t*> files; /**< File pointers which will be freed on TearDown() */
	std::vector<auth_cookie_t*> cookies; /**< Cookie pointers which will be freed on TearDown() */

	protected:
		auth_account_t* common_account = NULL;
		auth_request_log_t* test_request_log; /**< Common requester which can be used for unit tests. */

		/**
		 * @brief Initializes test_requester and calls RadicleTests::SetUp()
		 */
		void SetUp() override {
			common_account = manage_account("email", "password", ROLE_USER, false);
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
			for(std::vector<auth_file_t*>::iterator iter = files.begin(); iter != files.end(); iter++) {
				auth_file_free(iter.base());
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
		auth_account_t* manage_account(const char* email, const char* password, auth_account_role_t role, const bool verified) {
			string_t* email_buf = manage_string(email);
			string_t* password_buf = manage_string(password);
			auth_account_t* buf = auth_account_new(NULL,
					email_buf,
					password_buf,
					role,
				       	true,
				       	verified,
				       	0);
			accounts.push_back(buf);
			return buf;
		}

		auth_file_t* manage_file() {
			auth_file_t* file = (auth_file_t*) calloc(1, sizeof(auth_file_t));
			file->name = string_copy(common_string);
			file->path = string_copy(common_string);
			file->owner = uuid_copy(common_uuid);
			file->type = IMAGE_JPEG;
			files.push_back(file);
			return file;
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
};

/** @} */
/** @} */
