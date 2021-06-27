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

#include "radicle/pgdb.h"

class PGDBTest: public ::testing::Test {
	public:

		const char* conninfo = "host=localhost port=5432 dbname=ikag user=postgres password=postgres connect_timeout=10";
		PGconn* conn = NULL;

	protected:

		/*
		 * Could be replaced by 'static void SetUpTestSuite()' to speed up tests.
		 * But if one test fails to completely clean a conn, the next test could fail aswell.
		 * Like started transactions which havent been commited.
		 */
		void SetUp() override {
			ASSERT_EQ(pgdb_connect(conninfo, &conn), 0);
		}

		void TearDown() override {
			PQfinish(conn);
		}

};

TEST_F(PGDBTest, TestFixtureSetup) {
	// Do nothing since this will test the SetUp function
}

TEST_F(PGDBTest, TestPingInfo) {
	PGPing status;
	const char* info = pgdb_ping_info(conninfo, &status);
	ASSERT_EQ(status, PQPING_OK) << info;
}

TEST(PGDBParamsTest, TestParamsNew) {
	pgdb_params_t* params = pgdb_params_new(3);
	EXPECT_EQ(params->count, 3);
	EXPECT_TRUE(params->types != NULL);
	EXPECT_TRUE(params->values != NULL);
	EXPECT_TRUE(params->lengths != NULL);
	EXPECT_TRUE(params->formats != NULL);
	pgdb_params_free(&params);
}

TEST(PGDBParamsTest, TestBindInt) {
	pgdb_params_t* params = pgdb_params_new(1);
	pgdb_bind_int32(5, 0, params);
	EXPECT_EQ(params->lengths[0], sizeof(uint32_t));
	pgdb_params_free(&params);
}

