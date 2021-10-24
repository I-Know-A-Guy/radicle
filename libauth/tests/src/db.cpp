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
 */

#include <libpq-fe.h>
#include <string.h>

#include <gtest/gtest.h>

#include "radicle/auth/types.h"
#include "radicle/tests/auth/auth_fixture.hpp"
#include "radicle/auth/db.h"

static char FAKE_UUID[16] = {0x1f, 0x2f, 0x2f, 0x2f, 0x02, 0x2f, 0x2f, 0x4b, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x70, 0x2f};

PGDB_FAKE_FETCH(FetchEmptyData) {
	PGDB_FAKE_EMPTY_RESULT(PGRES_TUPLES_OK);
}

PGDB_FAKE_FETCH(FetchWrongColumns) {
	PGDB_FAKE_RESULT_1(PGRES_TUPLES_OK, "wrong-column");
	PGDB_FAKE_INT(1234);
	PGDB_FAKE_FINISH();
}

PGDB_FAKE_FETCH(FetchAccountUuid) {
	PGDB_FAKE_RESULT_1(PGRES_TUPLES_OK, "uuid");
	PGDB_FAKE_UUID(FAKE_UUID);
	PGDB_FAKE_FINISH();
}

TEST_F(RadicleAuthTests, TestSaveAccountSuccess) {
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(FetchAccountUuid));
	uuid_t* uuid = NULL;
	ASSERT_EQ(auth_save_account(NULL, common_account, &uuid), 0);
	ASSERT_TRUE(uuid != NULL);
	uuid_free(&uuid);
}

TEST_F(RadicleAuthTests, TestSaveAccountFailure) {
	install_status_fatal_error();
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(FetchAccountUuid));
	uuid_t* uuid = NULL;
	ASSERT_EQ(auth_save_account(NULL, common_account, &uuid), 1);
	ASSERT_TRUE(uuid == NULL);
}

TEST_F(RadicleAuthTests, TestSaveAccountEmptyData) {
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(FetchEmptyData));
	uuid_t* uuid = NULL;
	ASSERT_EQ(auth_save_account(NULL, common_account, &uuid), 1);
	ASSERT_TRUE(uuid == NULL);
}

TEST_F(RadicleAuthTests, TestSaveAccountWrongColumnds) {
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(FetchWrongColumns));
	uuid_t* uuid = NULL;
	ASSERT_EQ(auth_save_account(NULL, common_account, &uuid), 1);
	ASSERT_TRUE(uuid == NULL);
}

TEST_F(RadicleAuthTests, TestUpdateAccountEmailSuccess) {
	install_execute_always_success();
	ASSERT_EQ(auth_update_account_email(NULL, common_uuid, common_string), 0);
}

TEST_F(RadicleAuthTests, TestUpdateAccountEmailError) {
	install_pg_exec_hook();
	install_status_fatal_error();
	ASSERT_EQ(auth_update_account_email(NULL, common_uuid, common_string), 1);
}

TEST_F(RadicleAuthTests, TestUpdateAccountPasswordSuccess) {
	install_execute_always_success();
	ASSERT_EQ(auth_update_account_password(NULL, common_uuid, common_string), 0);
}

TEST_F(RadicleAuthTests, TestUpdateAccountPasswordError) {
	install_pg_exec_hook();
	install_status_fatal_error();
	ASSERT_EQ(auth_update_account_password(NULL, common_uuid, common_string), 1);
}

PGDB_FAKE_FETCH(FetchSessionId) {
	PGDB_FAKE_RESULT_1(PGRES_TUPLES_OK, "id");
	PGDB_FAKE_INT(5);
	PGDB_FAKE_FINISH();
}

TEST_F(RadicleAuthTests, TestSaveSessionSuccess) {
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(FetchSessionId));
	uint32_t id = 0;
	ASSERT_EQ(auth_save_session(NULL, common_uuid, common_string, 0, common_string, &id), 0);
	EXPECT_EQ(id, 5);
}

TEST_F(RadicleAuthTests, TestSaveSessionError) {
	install_status_fatal_error();
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(FetchSessionId));
	uint32_t id = 0;
	ASSERT_EQ(auth_save_session(NULL, common_uuid, common_string, 0, common_string, &id), 1);
	EXPECT_EQ(id, 0);
}

TEST_F(RadicleAuthTests, TestSaveSessionWrongColumns) {
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(FetchWrongColumns));
	uint32_t id = 0;
	ASSERT_EQ(auth_save_session(NULL, common_uuid, common_string, 0, common_string, &id), 1);
	EXPECT_EQ(id, 0);
}

TEST_F(RadicleAuthTests, TestSaveSessionEmptyData) {
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(FetchEmptyData));
	uint32_t id = 0;
	ASSERT_EQ(auth_save_session(NULL, common_uuid, common_string, 0, common_string, &id), 1);
	EXPECT_EQ(id, 0);
}

TEST_F(RadicleAuthTests, TestSaveSessionAccessSuccess) {
	install_status_command_ok();
	install_pg_exec_hook();
	uint32_t session_id = 1;
	ASSERT_EQ(auth_save_session_access(NULL, session_id, test_request_log), 0);
}

TEST_F(RadicleAuthTests, TestSaveSessionAccessError) {
	install_status_fatal_error();
	install_pg_exec_hook();
	uint32_t session_id = 1;
	ASSERT_EQ(auth_save_session_access(NULL, session_id, test_request_log), 1);
}

TEST_F(RadicleAuthTests, TestSaveTokenSuccess) {
	install_status_command_ok();
	install_pg_exec_hook();
	ASSERT_EQ(auth_save_token(NULL, common_uuid, common_string, REGISTRATION, NULL), 0);
}

TEST_F(RadicleAuthTests, TestSaveTokenError) {
	install_status_fatal_error();
	install_pg_exec_hook();
	ASSERT_EQ(auth_save_token(NULL, common_uuid, common_string, REGISTRATION, NULL), 1);
}

PGDB_FAKE_FETCH(FetchVerifyToken) {
	PGDB_FAKE_RESULT_1(PGRES_TUPLES_OK, "owner");
	PGDB_FAKE_UUID(FAKE_UUID);
	PGDB_FAKE_FINISH();
}

TEST_F(RadicleAuthTests, TestVerifyToken) {
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(FetchVerifyToken));
	uuid_t* owner = NULL;
	ASSERT_EQ(auth_verify_token(NULL, common_string, REGISTRATION, &owner, NULL), 0);
	ASSERT_TRUE(owner != NULL);
	EXPECT_EQ(memcmp(owner->bin, FAKE_UUID, 16), 0);
	uuid_free(&owner);
}

TEST_F(RadicleAuthTests, TestVerifyTokenError) {
	install_status_fatal_error();
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(FetchVerifyToken));
	uuid_t* owner = NULL;
	ASSERT_EQ(auth_verify_token(NULL, common_string, REGISTRATION, &owner, NULL), 1);
	EXPECT_TRUE(owner == NULL);
}

TEST_F(RadicleAuthTests, TestVerifyTokenEmptyData) {
	install_status_fatal_error();
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(FetchEmptyData));
	uuid_t* owner = NULL;
	ASSERT_EQ(auth_verify_token(NULL, common_string, REGISTRATION, &owner, NULL), 1);
	EXPECT_TRUE(owner == NULL);
}

TEST_F(RadicleAuthTests, TestVerifyTokenWrongColumns) {
	install_status_fatal_error();
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(FetchWrongColumns));
	uuid_t* owner = NULL;
	ASSERT_EQ(auth_verify_token(NULL, common_string, REGISTRATION, &owner, NULL), 1);
	EXPECT_TRUE(owner == NULL);
}

TEST_F(RadicleAuthTests, TestUpdateAccountVerificationSuccess) {
	install_status_command_ok();
	install_pg_exec_hook();
	ASSERT_EQ(auth_update_account_verification_status(NULL, common_uuid, true), 0);
}

TEST_F(RadicleAuthTests, TestUpdateAccountVerificationError) {
	install_status_fatal_error();
	install_pg_exec_hook();
	ASSERT_EQ(auth_update_account_verification_status(NULL, common_uuid, true), 1);
}

PGDB_FAKE_FETCH(FetchAccount) {
	PGDB_FAKE_RESULT_6(PGRES_TUPLES_OK, "uuid", "password", "role", "verified", "active", "created");
	PGDB_FAKE_UUID(FAKE_UUID);
	PGDB_FAKE_C_STR("password-hash");
	PGDB_FAKE_C_STR(auth_account_role_to_str(ROLE_USER));
	PGDB_FAKE_BOOL(true);
	PGDB_FAKE_BOOL(true);
	PGDB_FAKE_TIMESTAMP(1000000);
	PGDB_FAKE_FINISH();
}

TEST_F(RadicleAuthTests, TestGetAccountByEmailSuccess) {
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(FetchAccount));
	auth_account_t* queried = NULL;
	ASSERT_EQ(auth_get_account_by_email(NULL, common_string, &queried) ,0);
	ASSERT_TRUE(queried != NULL);
	EXPECT_EQ(memcmp(queried->uuid->bin, FAKE_UUID, 16), 0);
	EXPECT_EQ(memcmp(queried->password->ptr, "password-hash", queried->password->length), 0);
	EXPECT_EQ(queried->role, ROLE_USER);
	EXPECT_TRUE(queried->verified);
	EXPECT_TRUE(queried->active);
	EXPECT_EQ(queried->created, 1000000);
	auth_account_free(&queried);
}

TEST_F(RadicleAuthTests, TestGetAccountByEmailError) {
	install_status_fatal_error();
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(FetchAccount));
	auth_account_t* queried = NULL;
	ASSERT_EQ(auth_get_account_by_email(NULL, common_string, &queried), 1);
	ASSERT_TRUE(queried == NULL);
}

TEST_F(RadicleAuthTests, TestGetAccountByEmailEmptyData) {
	install_status_fatal_error();
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(FetchEmptyData));
	auth_account_t* queried = NULL;
	ASSERT_EQ(auth_get_account_by_email(NULL, common_string, &queried), 1);
	ASSERT_TRUE(queried == NULL);
}

TEST_F(RadicleAuthTests, TestGetAccountByEmailWrongColumns) {
	install_status_fatal_error();
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(FetchWrongColumns));
	auth_account_t* queried = NULL;
	ASSERT_EQ(auth_get_account_by_email(NULL, common_string, &queried), 1);
	ASSERT_TRUE(queried == NULL);
}

PGDB_FAKE_FETCH(FetchSessionAccount) {
	PGDB_FAKE_RESULT_8(PGRES_TUPLES_OK, "id", "salt", "uuid", "email", "role", "verified", "active", "created");
	PGDB_FAKE_INT(5);
	PGDB_FAKE_C_STR("session-salt");
	PGDB_FAKE_UUID(FAKE_UUID);
	PGDB_FAKE_C_STR("email");
	PGDB_FAKE_C_STR(auth_account_role_to_str(ROLE_USER));
	PGDB_FAKE_BOOL(true);
	PGDB_FAKE_BOOL(true);
	PGDB_FAKE_TIMESTAMP(1000000);
	PGDB_FAKE_FINISH();
}

TEST_F(RadicleAuthTests, TestGetSessionAccountByCookie) {
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(FetchSessionAccount));
	uint32_t session_id = 0;
	string_t* session_salt = NULL;
	auth_account_t* account = NULL;
	ASSERT_EQ(auth_get_session_by_cookie(NULL, common_string, &session_id, &session_salt, &account), 0);
	ASSERT_TRUE(session_salt != NULL);
	ASSERT_TRUE(account != NULL);

	EXPECT_EQ(session_id, 5);
	EXPECT_STREQ(session_salt->ptr, "session-salt");
	EXPECT_EQ(memcmp(account->uuid->bin, FAKE_UUID, 16), 0);
	EXPECT_STREQ(account->email->ptr, "email");
	EXPECT_EQ(account->role, ROLE_USER);
	EXPECT_TRUE(account->verified);
	EXPECT_TRUE(account->active);
	EXPECT_EQ(account->created, 1000000);

	string_free(&session_salt);
	auth_account_free(&account);
}

TEST_F(RadicleAuthTests, TestGetSessionAccountByCookieError) {
	install_status_fatal_error();
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(FetchSessionAccount));
	uint32_t session_id = 0;
	string_t* session_salt = NULL;
	auth_account_t* account = NULL;
	ASSERT_EQ(auth_get_session_by_cookie(NULL, common_string, &session_id, &session_salt, &account), 1);
	ASSERT_TRUE(session_salt == NULL);
	ASSERT_TRUE(account == NULL);
	ASSERT_EQ(session_id, 0);
}

TEST_F(RadicleAuthTests, TestGetSessionAccountByCookieEmptyData) {
	install_status_fatal_error();
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(FetchEmptyData));
	uint32_t session_id = 0;
	string_t* session_salt = NULL;
	auth_account_t* account = NULL;
	ASSERT_EQ(auth_get_session_by_cookie(NULL, common_string, &session_id, &session_salt, &account), 1);
	ASSERT_TRUE(session_salt == NULL);
	ASSERT_TRUE(account == NULL);
	ASSERT_EQ(session_id, 0);
}

TEST_F(RadicleAuthTests, TestGetSessionAccountByCookieWrongColumns) {
	install_status_fatal_error();
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(FetchWrongColumns));
	uint32_t session_id = 0;
	string_t* session_salt = NULL;
	auth_account_t* account = NULL;
	ASSERT_EQ(auth_get_session_by_cookie(NULL, common_string, &session_id, &session_salt, &account), 1);
	ASSERT_TRUE(session_salt == NULL);
	ASSERT_TRUE(account == NULL);
	ASSERT_EQ(session_id, 0);
}

PGDB_FAKE_FETCH(SaveBlacklist) {
	PGDB_FAKE_RESULT_1(PGRES_TUPLES_OK, "id");
	PGDB_FAKE_INT(123);
	PGDB_FAKE_FINISH();
}

TEST_F(RadicleAuthTests, TestSaveBlacklistSuccess) {
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(SaveBlacklist));
	uint32_t id;
	ASSERT_EQ(auth_blacklist_ip(NULL, common_string, time(NULL), time(NULL), &id), 0);
}

TEST_F(RadicleAuthTests, TestSaveBlacklistFailure) {
	install_status_fatal_error();
	uint32_t id;
	ASSERT_EQ(auth_blacklist_ip(NULL, common_string, time(NULL), time(NULL), &id), 1);
}

TEST_F(RadicleAuthTests, TestSaveBlacklistAccessSuccess) {
	install_execute_always_success();
	ASSERT_EQ(auth_save_blacklist_access(NULL, 1, time(NULL), common_string), 0);
}

TEST_F(RadicleAuthTests, TestSaveBlacklistAccessFailure) {
	install_pg_exec_param_hook();
	install_status_fatal_error();
	ASSERT_EQ(auth_save_blacklist_access(NULL, 1, time(NULL), common_string), 1);
}

TEST_F(RadicleAuthTests, TestSaveBlacklistEmptyData) {
	install_status_fatal_error();
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(FetchEmptyData));
	uint32_t id;
	ASSERT_EQ(auth_blacklist_ip(NULL, common_string, time(NULL), time(NULL), &id), 1);
}

TEST_F(RadicleAuthTests, TestSaveBlacklistWrongColumn) {
	install_status_fatal_error();
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(FetchWrongColumns));
	uint32_t id;
	ASSERT_EQ(auth_blacklist_ip(NULL, common_string, time(NULL), time(NULL), &id), 1);
}

PGDB_FAKE_FETCH(BlacklistLookup) {
	PGDB_FAKE_RESULT_1(PGRES_TUPLES_OK, "id");
	PGDB_FAKE_INT(1);
	PGDB_FAKE_FINISH();
}

TEST_F(RadicleAuthTests, TestLookupBlackListSuccess) {
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(BlacklistLookup));
	uint32_t id;
	ASSERT_EQ(auth_blacklist_lookup_ip(NULL, common_string, &id), 0);
	EXPECT_EQ(id, 1);
}

TEST_F(RadicleAuthTests, TestLookupBlackListSuccess2) {
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(FetchEmptyData));
	uint32_t id;
	ASSERT_EQ(auth_blacklist_lookup_ip(NULL, common_string, &id), 0);
	EXPECT_EQ(id, 0);
}

TEST_F(RadicleAuthTests, TestLookupBlackListFailure) {
	install_status_fatal_error();
	uint32_t id;
	ASSERT_EQ(auth_blacklist_lookup_ip(NULL, common_string, &id), 1);
}

PGDB_FAKE_FETCH(SessionAccessLookup) {
	PGDB_FAKE_RESULT_3(PGRES_TUPLES_OK, "internal_status", "response_code", "owner");

	PGDB_FAKE_INT(100);
	PGDB_FAKE_INT(200);
	PGDB_FAKE_UUID(FAKE_UUID);

	PGDB_FAKE_NEXT_ROW();

	PGDB_FAKE_INT(300);
	PGDB_FAKE_INT(500);
	PGDB_FAKE_UUID(FAKE_UUID);

	PGDB_FAKE_FINISH();
}

TEST_F(RadicleAuthTests, TestLookupSessionAccessesSuccess) {

	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(SessionAccessLookup));
	
	list_t* result = NULL;

	ASSERT_EQ(auth_session_lookup_ip(NULL, common_string, time(NULL), &result), 0);
	ASSERT_TRUE(result != NULL);

	take_list(result, &auth_session_access_entry_free);

	auth_session_access_entry_t* first = (auth_session_access_entry_t*)result->data;
	EXPECT_EQ(first->internal_status,  100);
	EXPECT_EQ(first->response_code,  200);
	EXPECT_EQ(memcmp(first->owner->bin, FAKE_UUID, 16), 0);

	ASSERT_TRUE(result->next != NULL);
	auth_session_access_entry_t* second = (auth_session_access_entry_t*)result->next->data;
	EXPECT_EQ(second->internal_status,  300);
	EXPECT_EQ(second->response_code,  500);
	EXPECT_EQ(memcmp(second->owner->bin, FAKE_UUID, 16), 0);
}

