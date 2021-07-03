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

#include "radicle/tests/auth/auth_fixture.hpp"
#include "radicle/auth.h"

/**
 * @brief Tests if correct login works with username and cookies.
 */
TEST_F(RadicleConnectedAuthTests, IntegrationAuth) {
	string_t* email = manage_string("test-user@radicle.com");
	string_t* password = manage_string("12345678");
	auth_account_t* account = manage_account(email->ptr, password->ptr, "user", true);
	ASSERT_EQ(auth_register(conn, account), AUTH_OK);
	ASSERT_TRUE(account->uuid != NULL);

	auth_cookie_t* register_cookie = NULL;
	string_t* key = manage_string("secret-key!");
	uint32_t session_id = 0;
	ASSERT_EQ(auth_make_owned_session(conn, account->uuid, key, &register_cookie, &session_id), AUTH_OK);
	take_cookie(register_cookie);

	ASSERT_EQ(auth_log_access(conn, session_id, test_requester, "register"), AUTH_OK);

	auth_account_t* signed_in_account = NULL;
	ASSERT_EQ(auth_sign_in(conn, email, password, &signed_in_account), AUTH_OK);
	take_account(signed_in_account);
	ASSERT_TRUE(signed_in_account != NULL);

	EXPECT_EQ(memcmp(account->uuid->bin, signed_in_account->uuid->bin, 16), 0);

	auth_cookie_t* signed_in_cookie = NULL;
	ASSERT_EQ(auth_make_owned_session(conn, signed_in_account->uuid, key, &signed_in_cookie, &session_id), AUTH_OK);
	take_cookie(signed_in_cookie);
	ASSERT_TRUE(signed_in_cookie != NULL);

	ASSERT_EQ(auth_log_access(conn, session_id, test_requester, "credentials-sign-in"), AUTH_OK);
	
	auth_session_t* cookie_session = NULL;
	auth_account_t* cookie_account = NULL;
	ASSERT_EQ(auth_verify_cookie(conn, key, signed_in_cookie->cookie, &cookie_session, &cookie_account), AUTH_OK) << signed_in_cookie->cookie->ptr;
	take_session(cookie_session);
	take_account(cookie_account);

	ASSERT_TRUE(cookie_session != NULL);
	ASSERT_TRUE(cookie_account != NULL);

	EXPECT_EQ(memcmp(account->uuid->bin, cookie_account->uuid->bin, 16), 0);

	ASSERT_EQ(auth_log_access(conn, cookie_session->id, test_requester, "cookie-sign-in"), AUTH_OK);
}
