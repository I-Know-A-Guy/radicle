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

#include <gtest/gtest.h>

#include <libpq-fe.h>

#include "radicle/tests/pgdb_hooks.hpp"
#include "radicle/pgdb.h"

PGconn* PQconnectdbFake(const char *conninfo) {
	return NULL;
}

ConnStatusType PQstatusSuccess(const PGconn *conn) {
	return CONNECTION_OK;
}

ConnStatusType PQstatusError(const PGconn *conn) {
	return CONNECTION_BAD;
}

TEST_F(RadicleTests, TestFixtureSetupSuccess) {
	install_hook(subhook_new((void*)PQconnectdb, (void*)PQconnectdbFake, SUBHOOK_64BIT_OFFSET));
	install_hook(subhook_new((void*)PQstatus, (void*)PQstatusSuccess, SUBHOOK_64BIT_OFFSET));
	PGconn* conn = NULL;	
	ASSERT_EQ(pgdb_connect("conn info", &conn), 0);
}

TEST_F(RadicleTests, TestFixtureSetupError) {
	install_hook(subhook_new((void*)PQconnectdb, (void*)PQconnectdbFake, SUBHOOK_64BIT_OFFSET));
	install_hook(subhook_new((void*)PQstatus, (void*)PQstatusError, SUBHOOK_64BIT_OFFSET));
	PGconn* conn = NULL;	
	ASSERT_EQ(pgdb_connect("conn info", &conn), 1);
}

TEST_F(RadiclePGDBHooks, TestExecute) {
	subhook_t status_hook = install_status_command_ok();
	install_pg_exec_hook();
	const char* command = "INSERT INTO Accounts(id) VALUES (1);";
	ASSERT_EQ(pgdb_execute(NULL, command), 0);

	remove_hook(status_hook);
	status_hook = install_status_fatal_error();

	ASSERT_EQ(pgdb_execute(NULL, command), 1);
}

TEST_F(RadiclePGDBHooks, TestExecuteParam) {
	subhook_t status_hook = install_status_command_ok();
	install_pg_exec_param_hook();
	const char* stmt= "INSERT INTO Accounts(id) VALUES ($1::int);";
	pgdb_params_t* params = pgdb_params_new(1);
	pgdb_bind_uint32(1, params);	

	EXPECT_EQ(pgdb_execute_param(NULL, stmt, params), 0);

	remove_hook(status_hook);
	status_hook = install_status_fatal_error();

	EXPECT_EQ(pgdb_execute_param(NULL, stmt, params), 1);
	
	pgdb_params_free(&params);
}

TEST_F(RadiclePGDBHooks, TestFetchParam) {
	subhook_t status_hook = install_status_tuples_ok();
	install_pg_exec_param_hook();
	const char* stmt= "SELECT * FROM Accounts WHERE id=$1::int;";
	pgdb_params_t* params = pgdb_params_new(1);
	pgdb_bind_uint32(1, params);	

	pgdb_result_t* result;
	EXPECT_EQ(pgdb_fetch_param(NULL, stmt, params, &result), 0);

	remove_hook(status_hook);
	status_hook = install_status_fatal_error();

	EXPECT_EQ(pgdb_fetch_param(NULL, stmt, params, &result), 1);

	pgdb_result_free(&result);
	pgdb_params_free(&params);
}

TEST(PGDBParamsTest, TestParamsNew) {
	pgdb_params_t* params = pgdb_params_new(3);
	EXPECT_EQ(params->count, 3);
	EXPECT_TRUE(params->types != NULL);
	EXPECT_TRUE(params->values != NULL);
	EXPECT_TRUE(params->lengths != NULL);
	EXPECT_TRUE(params->formats != NULL);
	pgdb_params_free(&params);
	ASSERT_TRUE(params == NULL);
}

TEST_F(RadicleTests, TestBinds) {
	pgdb_params_t* params = pgdb_params_new(8);
	
	pgdb_bind_null(params);
	EXPECT_EQ(params->lengths[0], 0);

	pgdb_bind_uint32(1, params);
	EXPECT_EQ(params->lengths[1], sizeof(uint32_t));

	pgdb_bind_uint64(1, params);
	EXPECT_EQ(params->lengths[2], sizeof(uint64_t));

	pgdb_bind_text(common_string, params);
	EXPECT_EQ(params->lengths[3], strlen(common_string->ptr));

	pgdb_bind_c_str(common_string->ptr, params);
	EXPECT_EQ(params->lengths[4], strlen(common_string->ptr));

	pgdb_bind_uuid(common_uuid, params);
	EXPECT_EQ(params->lengths[5], 16);

	pgdb_bind_bool(false, params);
	EXPECT_EQ(params->lengths[6], sizeof(bool));

	pgdb_bind_timestamp(1, params);
	EXPECT_EQ(params->lengths[7], sizeof(time_t));

	pgdb_params_free(&params);
}

TEST(PGDBResultTest, TestNewResult) {
	pgdb_result_t* result = pgdb_result_new(NULL);
	ASSERT_TRUE(result != NULL);
	pgdb_result_free(&result);
	ASSERT_TRUE(result == NULL);
}

TEST_F(RadiclePGDBHooks, TestCreateLimit) {
	// TODO subhook for pgdb_connect
	install_pgdb_connect_fake();	
	pgdb_connection_queue_t* queue = pgdb_connection_queue_new("", 1, 0);

	pgdb_connection_t* claimed = NULL;
	EXPECT_EQ(pgdb_claim_connection(queue, &claimed), 0);
	EXPECT_TRUE(claimed != NULL);

	pgdb_connection_t* not_claimed = NULL;
	EXPECT_EQ(pgdb_claim_connection(queue, &not_claimed), 0);
	EXPECT_TRUE(not_claimed == NULL);

	pgdb_release_connection(&claimed);
	EXPECT_TRUE(claimed == NULL);
	EXPECT_EQ(pgdb_claim_connection(queue, &claimed), 0);
	EXPECT_TRUE(claimed != NULL);

	pgdb_connection_queue_free(&queue);
}

