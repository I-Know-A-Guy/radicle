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
	ASSERT_EQ(auth_save_session(conn, common_uuid, common_string, 0, common_string, &id), 0);
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
