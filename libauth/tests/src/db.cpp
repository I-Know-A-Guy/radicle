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
 * @todo Create more tests per test, Test OK, No data, FAtal error, and wrong
 * column names
 */

#include <string.h>

#include <gtest/gtest.h>

#include "radicle/tests/auth/auth_fixture.hpp"
#include "radicle/auth/db.h"

char FAKE_ACCOUNT_UUID[16] = {0x1f, 0x2f, 0x2f, 0x2f, 0x02, 0x2f, 0x2f, 0x4b, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x70, 0x2f};

PGDB_FETCH_FAKE(FetchEmptyData) {
	PGDB_FAKE_EMPTY_RESULT(PGRES_TUPLES_OK);
}

PGDB_FETCH_FAKE(FetchAccountWrongColumns) {
	PGDB_FAKE_RESULT_1(PGRES_TUPLES_OK, "not-uuid");
	PGDB_FAKE_UUID(FAKE_ACCOUNT_UUID);
	PGDB_FAKE_FINISH();
}

PGDB_FETCH_FAKE(FetchAccountUuid) {
	PGDB_FAKE_RESULT_1(PGRES_TUPLES_OK, "uuid");
	PGDB_FAKE_UUID(FAKE_ACCOUNT_UUID);
	PGDB_FAKE_FINISH();
}

TEST_F(RadicleAuthTests, TestSaveAccountSuccess) {
	install_hook(PGDB_CREATE_FETCH_HOOK(FetchAccountUuid));
	uuid_t* uuid = NULL;
	ASSERT_EQ(auth_save_account(NULL, common_account, &uuid), 0);
	ASSERT_TRUE(uuid != NULL);
	uuid_free(&uuid);
}

TEST_F(RadicleAuthTests, TestSaveAccountFailure) {
	install_status_fatal_error();
	install_hook(PGDB_CREATE_FETCH_HOOK(FetchAccountUuid));
	uuid_t* uuid = NULL;
	ASSERT_EQ(auth_save_account(NULL, common_account, &uuid), 1);
	ASSERT_TRUE(uuid == NULL);
}

TEST_F(RadicleAuthTests, TestSaveAccountEmptyData) {
	/** @todo Test if function correctly returns when there is no data */
	install_hook(PGDB_CREATE_FETCH_HOOK(FetchEmptyData));
	uuid_t* uuid = NULL;
	ASSERT_EQ(auth_save_account(NULL, common_account, &uuid), 1);
	ASSERT_TRUE(uuid == NULL);
}

TEST_F(RadicleAuthTests, TestSaveAccountWrongColumnds) {
	/** @todo Test if function correctly returns when there is no data */
	install_hook(PGDB_CREATE_FETCH_HOOK(FetchAccountWrongColumns));
	uuid_t* uuid = NULL;
	ASSERT_EQ(auth_save_account(NULL, common_account, &uuid), 1);
	ASSERT_TRUE(uuid == NULL);
}

TEST_F(RadicleAuthTests, TestUpdateAccountPassword) {
	install_execute_param_always_success();
	ASSERT_EQ(auth_update_account_password(NULL, common_uuid, common_string), 0);
}

TEST_F(RadicleAuthTests, TestSaveSession) {
	install_fetch_id_hook();
	uint32_t id = 0;
	ASSERT_EQ(auth_save_session(NULL, common_uuid, common_string, 0, common_string, &id), 0);
	EXPECT_GT(id, 0);
}

TEST_F(RadicleAuthTests, TestSaveSessionAccess) {
	install_execute_param_always_success();
	uint32_t session_id = 1;
	ASSERT_EQ(auth_save_session_access(NULL, session_id, test_request_log), 0);
}

TEST_F(RadicleAuthTests, TestSaveToken) {
	install_execute_param_always_success();
	ASSERT_EQ(auth_save_token(NULL, common_uuid, common_string, REGISTRATION), 0);
}

TEST_F(RadicleAuthTests, TestVerifyToken) {
	install_fetch_token_hook();
	uuid_t* owner = NULL;
	token_type_t token;
	ASSERT_EQ(auth_verify_token(NULL, common_string, &owner, &token), 0);
	EXPECT_TRUE(owner != NULL);
	EXPECT_NE(token, -1);
	uuid_free(&owner);
}

TEST_F(RadicleAuthTests, TestUpdateAccountVerification) {
	install_execute_param_always_success();
	ASSERT_EQ(auth_update_account_verification_status(NULL, common_uuid, true), 0);
}

TEST_F(RadicleAuthTests, TestGetAccountByEmail) {
	install_fetch_account_hook();
	auth_account_t* queried = NULL;
	ASSERT_EQ(auth_get_account_by_email(NULL, common_string, &queried) ,0);
	EXPECT_TRUE(queried != NULL);
	auth_account_free(&queried);
}

TEST_F(RadicleAuthTests, TestGetSessionAccountByCookie) {
	install_fetch_session_hook();
	uint32_t session_id = 0;
	string_t* session_salt = NULL;
	auth_account_t* account = NULL;
	ASSERT_EQ(auth_get_session_by_cookie(NULL, common_string, &session_id, &session_salt, &account), 0);
	EXPECT_TRUE(session_id != 0);
	EXPECT_TRUE(session_salt != NULL);
	EXPECT_TRUE(account != NULL);
	string_free(&session_salt);
	auth_account_free(&account);
}
