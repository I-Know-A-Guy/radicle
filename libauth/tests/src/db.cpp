#include <string.h>

#include <gtest/gtest.h>

#include "radicle/tests/auth/auth_fixture.hpp"
#include "radicle/auth/db.h"

TEST_F(RadicleAuthTests, TestSaveAccount) {
	install_fetch_uuid_hook();
	auth_account_t* account = manage_account(
			"test_save_account@email",
			"password",
			"user",
			false
			);
	ASSERT_EQ(auth_save_account(conn, account, &account->uuid), 0);
}

TEST_F(RadicleAuthTests, TestSaveRegistration) {
	install_execute_param_always_success();
	ASSERT_EQ(auth_save_registration(conn, common_uuid, common_string), 0);
}

TEST_F(RadicleAuthTests, TestSaveSession) {
	install_fetch_id_hook();
	uint32_t id = 0;
	ASSERT_EQ(auth_save_session(conn, common_uuid, common_string, common_string, &id), 0);
	EXPECT_GT(id, 0);
}

TEST_F(RadicleAuthTests, TestSaveSessionAccess) {
	install_execute_param_always_success();
	uint32_t session_id = 1;
	ASSERT_EQ(auth_save_session_access(conn, session_id, test_requester), 0);
}

TEST_F(RadicleAuthTests, TestGetAccountByEmail) {
	install_fetch_complete_hook();
	auth_account_t* queried = NULL;
	ASSERT_EQ(auth_get_account_by_email(conn, common_string, &queried) ,0);
	auth_account_free(&queried);
}
