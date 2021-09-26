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

/**
 * @brief Obviously the connection will not be possible using a fake conn info
 * string, hence we trick the programm to think it worked.
 */
ConnStatusType PQstatus_fake(const PGconn *conn) {
	return CONNECTION_OK; 
}

TEST_F(APITests, TestEndpointInitSuccess) {

	install_hook(subhook_new((void*)PQstatus, (void*)PQstatus_fake, SUBHOOK_64BIT_OFFSET));

	api_instance_t* instance = manage_instance();

	_u_request* request = manage_request();
	_u_response* response = manage_response();
	
	ASSERT_EQ(api_callback_endpoint_init(request, response, instance), U_CALLBACK_CONTINUE);
}

ConnStatusType PQstatus_failure(const PGconn *conn) {
	return CONNECTION_BAD; 
}

TEST_F(APITests, TestEndpointInitFailureConnection) {

	install_hook(subhook_new((void*)PQstatus, (void*)PQstatus_failure, SUBHOOK_64BIT_OFFSET));

	api_instance_t* instance = manage_instance();

	_u_request* request = manage_request();
	_u_response* response = manage_response();
	
	ASSERT_EQ(api_callback_endpoint_init(request, response, instance), U_CALLBACK_COMPLETE);

	EXPECT_EQ(response->status, 503);
}

int pgdb_claim_connection_fake_full(pgdb_connection_queue_t* queue, pgdb_connection_t** conn) {
	*conn = NULL;
	return 0;
}

TEST_F(APITests, TestEndpointInitNoFreeConnections) {

	install_hook(subhook_new((void*)PQstatus, (void*)PQstatus_fake, SUBHOOK_64BIT_OFFSET));
	install_hook(subhook_new((void*)pgdb_claim_connection, (void*)pgdb_claim_connection_fake_full, SUBHOOK_64BIT_OFFSET));

	api_instance_t* instance = manage_instance();

	_u_request* request = manage_request();
	_u_response* response = manage_response();
	
	ASSERT_EQ(api_callback_endpoint_init(request, response, instance), U_CALLBACK_COMPLETE);

	EXPECT_EQ(response->status, 503);
}

TEST_F(APITests, TestEndpointLoadJsonNoBody) {
	api_instance_t* instance = manage_instance();
	_u_request* request = manage_request();
	_u_response* response = manage_response();

	ASSERT_EQ(api_callback_endpoint_load_json_body(request, response, instance), U_CALLBACK_COMPLETE);
	EXPECT_EQ(response->status, 400);
}

TEST_F(APITests, TestEndpointLoadJsonInvalid) {
	api_instance_t* instance = manage_instance();
	_u_request* request = manage_request();
	_u_response* response = manage_response();
	create_endpoint(response);

	char body[] = "{\"json\":\"value}";
	ulfius_set_string_body_request(request, body);

	ASSERT_EQ(api_callback_endpoint_load_json_body(request, response, instance), U_CALLBACK_COMPLETE);
	EXPECT_EQ(response->status, 400);
}

TEST_F(APITests, TestEndpointLoadJsonSuccess) {
	api_instance_t* instance = manage_instance();
	_u_request* request = manage_request();
	_u_response* response = manage_response();
	create_endpoint(response);

	char body[] = "{\"json\":\"value\"}";
	ulfius_set_string_body_request(request, body);

	ASSERT_EQ(api_callback_endpoint_load_json_body(request, response, instance), U_CALLBACK_CONTINUE);
	api_endpoint_t* endpoint = (api_endpoint_t*)response->shared_data;
	ASSERT_TRUE(endpoint->json_body != NULL);
	json_t* value = json_object_get(endpoint->json_body, "json");
	ASSERT_TRUE(value != NULL);
	EXPECT_STREQ(json_string_value(value), "value");

	api_endpoint_free((api_endpoint_t*)response->shared_data);
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

/**
	 * @todo fake initialize api instance
	 * @todo fake pgdb_connect (for pgdb_claim_connection) 
	 * @todo fake auth_make_free_session or auth_make_owned_session
	 * @todo fake auth_log_access
	 */

