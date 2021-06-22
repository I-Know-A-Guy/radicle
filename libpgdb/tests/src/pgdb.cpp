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
			ASSERT_EQ(pqdb_connect(conninfo, &conn), 0);
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
	const char* info = pqdb_ping_info(conninfo, &status);
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

