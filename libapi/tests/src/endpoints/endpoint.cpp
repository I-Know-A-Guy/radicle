/**
 * @file
 * @todo first implement all tests for functions which do not depend on
 * different functions
 */

#include <gtest/gtest.h>
#include <libpq-fe.h>
#include <ulfius.h>

#include "radicle/pgdb.h"
#include "radicle/tests/api/api_fixture.hpp"
#include "radicle/api/endpoints/endpoint.h"
#include "radicle/types/string.h"
#include "subhook.h"

TEST_F(APITests, TestEndpointError) {
	_u_request* request = manage_request();
	_u_response* response = manage_response();
	
	ASSERT_EQ(api_callback_endpoint_init(request, response, NULL), U_CALLBACK_ERROR);
}

api_instance_t* api_instance_new() {
	api_instance_t* instance  = (api_instance_t*)calloc(1, sizeof(api_instance_t));
	instance->queue = pgdb_connection_queue_new("conn info", 10, 10);
	return instance;
}

/**
 * @brief Obviously the connection will not be possible using a fake conn info
 * string, hence we trick the programm to think it worked.
 */
ConnStatusType PQstatus_fake(const PGconn *conn) {
	return CONNECTION_OK; 
}

TEST_F(APITests, TestEndpointInitSuccess) {

	install_hook(subhook_new((void*)PQstatus, (void*)PQstatus_fake, SUBHOOK_64BIT_OFFSET));

	api_instance_t* instance = api_instance_new();

	_u_request* request = manage_request();
	_u_response* response = manage_response();
	
	ASSERT_EQ(api_callback_endpoint_init(request, response, instance), U_CALLBACK_CONTINUE);
}

ConnStatusType PQstatus_failure(const PGconn *conn) {
	return CONNECTION_BAD; 
}

/*
PGDB_FAKE_FETCH_STORY(RespondStory) {
	PGDB_FAKE_STORY_BRANCH(RespondStory, 0); // Session Id
		PGDB_FAKE_RESULT_1(PGRES_TUPLES_OK, "id");
		PGDB_FAKE_INT(100);
		PGDB_FAKE_FINISH();
	PGDB_FAKE_STORY_BRANCH_END();
}
*/

TEST_F(APITests, TestEndpointInitSuccessFailure) {

	install_hook(subhook_new((void*)PQstatus, (void*)PQstatus_failure, SUBHOOK_64BIT_OFFSET));

	install_execute_always_success();

	api_instance_t* instance = api_instance_new();

	_u_request* request = manage_request();
	_u_response* response = manage_response();
	
	ASSERT_EQ(api_callback_endpoint_init(request, response, instance), U_CALLBACK_COMPLETE);

	EXPECT_EQ(response->status, 503);
	

	/**
	 * @todo fake initialize api instance
	 * @todo fake pgdb_connect (for pgdb_claim_connection) 
	 * @todo fake auth_make_free_session or auth_make_owned_session
	 * @todo fake auth_log_access
	 */
}
