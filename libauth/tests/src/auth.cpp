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
 * @todo Fix integration test which fails on auth_save_session because timestamp binary is not readable by postgresql.
 */

#include <string.h>

#include <gtest/gtest.h>

#include "radicle/auth/db.h"
#include "radicle/tests/auth/auth_fixture.hpp"
#include "radicle/auth.h"

/*

#define FAKE_SESSION_ID 123

PGDB_FETCH_FAKE(FetchSessionFake) {
	PGDB_FAKE_RESULT_1("id");
	PGDB_FAKE_INT(FAKE_SESSION_ID);
	PGDB_FAKE_FINISH();
}

TEST_F(RadicleAuthTests, TestOwnedSession) {
	install_status_tuples_ok();
	install_hook(PGDB_CREATE_FETCH_HOOK(FetchSessionFake));

	auth_cookie_t* cookie = NULL;
	uint32_t session = 0;
	ASSERT_EQ(auth_make_owned_session(NULL, common_uuid, common_string, &cookie, &session), AUTH_OK);
	EXPECT_TRUE(cookie != NULL);
	EXPECT_EQ(session, FAKE_SESSION_ID);

	auth_cookie_free(&cookie);
}
*/

/**
 * @brief Tests if correct login works with username and cookies.
 */

/*
TEST_F(RadicleConnectedAuthTests, IntegrationAuth) {
	string_t* email = manage_string("test-user@radicle.com");
	string_t* password = manage_string("12345678");
	auth_account_t* account = manage_account(email->ptr, password->ptr, ROLE_USER, false);
	ASSERT_EQ(auth_register(conn, account), AUTH_OK);
	ASSERT_TRUE(account->uuid != NULL);

	string_t* token = NULL;
	ASSERT_EQ(auth_create_token(conn, account->uuid, REGISTRATION, &token), 0);
	ASSERT_TRUE(token != NULL);
	take_string(token);

	uuid_t* registration_owner = NULL;
	token_type_t token_type;
	ASSERT_EQ(auth_verify_token(conn, token, &registration_owner, &token_type), 0);
	ASSERT_TRUE(registration_owner != NULL);
	ASSERT_EQ(token_type, REGISTRATION);
	take_uuid(registration_owner);

	ASSERT_EQ(memcmp(account->uuid->bin, registration_owner->bin, 16), 0);

	ASSERT_EQ(auth_update_account_verification_status(conn, registration_owner, true), 0);

	auth_cookie_t* register_cookie = NULL;
	string_t* key = manage_string("secret-key!");
	uint32_t session_id = 0;
	ASSERT_EQ(auth_make_owned_session(conn, account->uuid, key, &register_cookie, &session_id), AUTH_OK);
	take_cookie(register_cookie);

	ASSERT_EQ(auth_log_access(conn, session_id, test_request_log), AUTH_OK);

	auth_account_t* signed_in_account = NULL;
	ASSERT_EQ(auth_sign_in(conn, email, password, &signed_in_account), AUTH_OK);
	take_account(signed_in_account);
	ASSERT_TRUE(signed_in_account != NULL);

	EXPECT_EQ(memcmp(account->uuid->bin, signed_in_account->uuid->bin, 16), 0);

	auth_cookie_t* signed_in_cookie = NULL;
	ASSERT_EQ(auth_make_owned_session(conn, signed_in_account->uuid, key, &signed_in_cookie, &session_id), AUTH_OK);
	take_cookie(signed_in_cookie);
	ASSERT_TRUE(signed_in_cookie != NULL);

	ASSERT_EQ(auth_log_access(conn, session_id, test_request_log), AUTH_OK);
	
	uint32_t cookie_session_id = 0;
	auth_account_t* cookie_account = NULL;
	ASSERT_EQ(auth_verify_cookie(conn, key, signed_in_cookie->cookie, &cookie_session_id, &cookie_account), AUTH_OK) << signed_in_cookie->cookie->ptr;
	take_account(cookie_account);

	ASSERT_TRUE(cookie_session_id != 0);
	ASSERT_TRUE(cookie_account != NULL);

	EXPECT_EQ(memcmp(account->uuid->bin, cookie_account->uuid->bin, 16), 0);

	ASSERT_EQ(auth_log_access(conn, cookie_session_id, test_request_log), AUTH_OK);

	ASSERT_EQ(auth_update_password(conn, signed_in_account->uuid, manage_string("new password!")), 0);

}
*/
