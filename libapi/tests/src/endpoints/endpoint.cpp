/**
 * @file
 */

#include <gtest/gtest.h>
#include <libpq-fe.h>
#include <ulfius.h>

#include "radicle/auth/crypto.h"
#include "radicle/auth/db.h"
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

TEST_F(APITests, TestEndpointCheckForSessionNoSessionId) {
	api_instance_t* instance = manage_instance();
	_u_request* request = manage_request();
	_u_response* response = manage_response();
	api_endpoint_t* endpoint = create_endpoint(response);

	ASSERT_EQ(api_callback_endpoint_check_for_session(request, response, instance), U_CALLBACK_CONTINUE);
	EXPECT_FALSE(endpoint->authenticated);
}

PGDB_FAKE_FETCH_STORY(FetchEmptySessionId) {
	// get_session_by_cookie
	PGDB_FAKE_STORY_BRANCH(FetchEmptySessionId, 0);
		PGDB_FAKE_EMPTY_RESULT(PGRES_TUPLES_OK);
	PGDB_FAKE_STORY_BRANCH_END();

	// insert session id returing session id
	PGDB_FAKE_STORY_BRANCH(FetchEmptySessionId, 1);
		PGDB_FAKE_RESULT_1(PGRES_TUPLES_OK, "id");
		PGDB_FAKE_INT(5);
		PGDB_FAKE_FINISH();
	PGDB_FAKE_STORY_BRANCH_END();

	return NULL;
}

#define FAKE_COOKIE_STRING "xdrHw2vjKmpu5hPRae3MXeyf5CA6P248ikhb2xHxjbduWtdW29apXvQDhARDnHcdAHwLPLHZ3ffTEccdmCLqGvH9JAhkeZuZYijPH2ijtT3bHdXhupbgxNQAjLhhRkWt-xdrHw2vjKmpu5hPRae3MXeyf5CA6P248ikhb2xHxjbduWtdW29apXvQDhARDnHcdAHwLPLHZ3ffTEccdmCLqGvH9JAhkeZuZYijPH2ijtT3bHdXhupbgxNQAjLhhRkWt"

TEST_F(APITests, TestEndpointCheckForSessionInvalidSessionId) {
	api_instance_t* instance = manage_instance();
	_u_request* request = manage_request();
	_u_response* response = manage_response();
	api_endpoint_t* endpoint = create_endpoint(response);

	PGDB_FAKE_INIT_FETCH_STORY(FetchEmptySessionId);
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(FetchEmptySessionId));

	u_map_put(request->map_cookie, "session-id", FAKE_COOKIE_STRING);
	ASSERT_EQ(api_callback_endpoint_check_for_session(request, response, instance), U_CALLBACK_COMPLETE);
	EXPECT_EQ(response->status, 400);
}

static char FAKE_UUID[16] = {0x1f, 0x2f, 0x2f, 0x2f, 0x02, 0x2f, 0x2f, 0x4b, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x2f, 0x70, 0x2f};

PGDB_FAKE_FETCH_STORY(FetchSessionIdWrongSalt) {
	// get_session_by_cookie
	PGDB_FAKE_STORY_BRANCH(FetchSessionIdWrongSalt, 0);
		PGDB_FAKE_RESULT_8(PGRES_TUPLES_OK, "id", "salt", "uuid", "email", "role", "verified", "active", "created");
		PGDB_FAKE_INT(5);
		PGDB_FAKE_C_STR("session-salt");
		PGDB_FAKE_UUID(FAKE_UUID);
		PGDB_FAKE_C_STR("email");
		PGDB_FAKE_C_STR(auth_account_role_to_str(ROLE_USER));
		PGDB_FAKE_BOOL(true);
		PGDB_FAKE_BOOL(false);
		PGDB_FAKE_TIMESTAMP(1000000);
		PGDB_FAKE_FINISH();
	PGDB_FAKE_STORY_BRANCH_END();

	// insert session id returing session id
	PGDB_FAKE_STORY_BRANCH(FetchSessionIdWrongSalt, 1);
		PGDB_FAKE_RESULT_1(PGRES_TUPLES_OK, "id");
		PGDB_FAKE_INT(5);
		PGDB_FAKE_FINISH();
	PGDB_FAKE_STORY_BRANCH_END();

	return NULL;
}

TEST_F(APITests, TestEndpointCheckForSessionInvalidSessionHash) {
	api_instance_t* instance = manage_instance();
	_u_request* request = manage_request();
	_u_response* response = manage_response();
	api_endpoint_t* endpoint = create_endpoint(response);

	PGDB_FAKE_INIT_FETCH_STORY(FetchSessionIdWrongSalt);
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(FetchSessionIdWrongSalt));

	u_map_put(request->map_cookie, "session-id", FAKE_COOKIE_STRING);
	ASSERT_EQ(api_callback_endpoint_check_for_session(request, response, instance), U_CALLBACK_COMPLETE);
	EXPECT_EQ(response->status, 400);
}


int hmac_verify_salted_fake(const string_t* key, const string_t* salt, const string_t* signature, const string_t* input) {
	return 0;
}

TEST_F(APITests, TestEndpointCheckForSessionAccountDeactivated) {

	PGDB_FAKE_INIT_FETCH_STORY(FetchSessionIdWrongSalt);
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(FetchSessionIdWrongSalt));
	install_hook(subhook_new((void*)hmac_verify_salted, (void*)hmac_verify_salted_fake, SUBHOOK_64BIT_OFFSET));

	api_instance_t* instance = manage_instance();
	_u_request* request = manage_request();
	_u_response* response = manage_response();
	api_endpoint_t* endpoint = create_endpoint(response);

	u_map_put(request->map_cookie, "session-id", FAKE_COOKIE_STRING);
	ASSERT_EQ(api_callback_endpoint_check_for_session(request, response, instance), U_CALLBACK_COMPLETE);
	EXPECT_EQ(response->status, 403);
}

TEST_F(APITests, TestEndointCheckForAuthSuccess) {
	api_instance_t* instance = manage_instance();
	_u_request* request = manage_request();
	_u_response* response = manage_response();
	api_endpoint_t* endpoint = create_endpoint(response);

	endpoint->authenticated = true;

	ASSERT_EQ(api_callback_endpoint_check_for_authentication(request, response, instance), U_CALLBACK_CONTINUE);

	api_endpoint_free(endpoint);
}

TEST_F(APITests, TestEndointCheckForAuthFailure) {
	api_instance_t* instance = manage_instance();
	_u_request* request = manage_request();
	_u_response* response = manage_response();
	api_endpoint_t* endpoint = create_endpoint(response);

	endpoint->authenticated = false;

	ASSERT_EQ(api_callback_endpoint_check_for_authentication(request, response, instance), U_CALLBACK_COMPLETE);
}

TEST_F(APITests, TestEndointCheckForVerifiedEmailFailureNotSignedIn) {
	api_instance_t* instance = manage_instance();
	_u_request* request = manage_request();
	_u_response* response = manage_response();
	api_endpoint_t* endpoint = create_endpoint(response);

	endpoint->authenticated = false;

	ASSERT_EQ(api_callback_endpoint_check_for_verified_email(request, response, instance), U_CALLBACK_COMPLETE);
}

TEST_F(APITests, TestEndointCheckForVerifiedEmailFailureNotVerified) {
	api_instance_t* instance = manage_instance();
	_u_request* request = manage_request();
	_u_response* response = manage_response();
	api_endpoint_t* endpoint = create_endpoint(response);

	endpoint->authenticated = true;
	endpoint->account = (auth_account_t*)calloc(1, sizeof(auth_account_t));
	endpoint->account->verified = false;

	ASSERT_EQ(api_callback_endpoint_check_for_verified_email(request, response, instance), U_CALLBACK_COMPLETE);
}

TEST_F(APITests, TestEndointCheckForVerifiedEmailSuccess) {
	api_instance_t* instance = manage_instance();
	_u_request* request = manage_request();
	_u_response* response = manage_response();
	api_endpoint_t* endpoint = create_endpoint(response);

	endpoint->authenticated = true;
	endpoint->account = (auth_account_t*)calloc(1, sizeof(auth_account_t));
	endpoint->account->verified = true;

	ASSERT_EQ(api_callback_endpoint_check_for_verified_email(request, response, instance), U_CALLBACK_CONTINUE);

	api_endpoint_free(endpoint);
}

PGDB_FAKE_FETCH_STORY(BlacklistFetch) {

	PGDB_FAKE_STORY_BRANCH(BlacklistFetch, 0);
		PGDB_FAKE_RESULT_1(PGRES_TUPLES_OK, "anonymous");
		PGDB_FAKE_INT(1);
		PGDB_FAKE_FINISH();
	PGDB_FAKE_STORY_BRANCH_END();

	API_FAKE_SESSION(BlacklistFetch, 1);

	return NULL;
}

TEST_F(APITests, TestAuthCheckBlacklistSuccessBlacklisted) {

	PGDB_FAKE_INIT_FETCH_STORY(BlacklistFetch);
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(BlacklistFetch));

	api_instance_t* instance = manage_instance();
	_u_request* request = manage_request();
	_u_response* response = manage_response();
	api_endpoint_t* endpoint = create_endpoint(response);

	ASSERT_EQ(api_auth_callback_check_blacklist(request, response, instance), U_CALLBACK_COMPLETE);

	EXPECT_EQ(response->status, 403);
}


API_EMPTY_RESULT(EmptyBlacklistFetch);

TEST_F(APITests, TestAuthCheckBlacklistSuccessFree) {

	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(EmptyBlacklistFetch));

	api_instance_t* instance = manage_instance();
	_u_request* request = manage_request();
	_u_response* response = manage_response();
	api_endpoint_t* endpoint = create_endpoint(response);

	ASSERT_EQ(api_auth_callback_check_blacklist(request, response, instance), U_CALLBACK_CONTINUE);

	api_endpoint_free(endpoint);
}

API_EMPTY_RESULT(IpNoneMalicious);

TEST_F(APITests, TestAuthCallbackCheckIpForMaliciousActivityEmpty) {

	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(IpNoneMalicious));

	api_instance_t* instance = manage_instance();
	instance->max_session_accesses_in_lookup_delta = 1;
	instance->max_session_accesses_penalty_in_s = 100;
	instance->max_session_accesses_lookup_delta_in_s = 100;

	_u_request* request = manage_request();
	_u_response* response = manage_response();
	api_endpoint_t* endpoint = create_endpoint(response);

	ASSERT_EQ(api_auth_callback_check_ip_for_malicious_activity(request, response, instance), U_CALLBACK_CONTINUE);

	api_endpoint_free(endpoint);
}

PGDB_FAKE_FETCH_STORY(IpMaliciousActivity) {
	PGDB_FAKE_STORY_BRANCH(IpMaliciousActivity, 0);
		PGDB_FAKE_RESULT_3(PGRES_TUPLES_OK, "owner", "internal_status", "response_code");

		PGDB_FAKE_UUID(FAKE_UUID);
		PGDB_FAKE_INT(123);
		PGDB_FAKE_INT(500);

		PGDB_FAKE_NEXT_ROW();

		PGDB_FAKE_UUID(FAKE_UUID);
		PGDB_FAKE_INT(123);
		PGDB_FAKE_INT(500);

		PGDB_FAKE_NEXT_ROW();

		PGDB_FAKE_UUID(FAKE_UUID);
		PGDB_FAKE_INT(123);
		PGDB_FAKE_INT(500);

		PGDB_FAKE_FINISH();
	PGDB_FAKE_STORY_BRANCH_END();

	API_FAKE_SESSION(IpMaliciousActivity, 1);

	return NULL;
}

static int auth_blacklist_ip_counter = 0;
int auth_blacklist_ip_fake_counter(PGconn* conn, const string_t* ip, const time_t date, const time_t ban_lift, uint32_t* id) {
	auth_blacklist_ip_counter++;
	return 0;
}

TEST_F(APITests, TestAuthCallbackCheckIpForMaliciousActivityMalicious) {

	PGDB_FAKE_INIT_FETCH_STORY(IpMaliciousActivity);
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(IpMaliciousActivity));
	install_hook(subhook_new((void*)auth_blacklist_ip, (void*)auth_blacklist_ip_fake_counter, SUBHOOK_64BIT_OFFSET));

	api_instance_t* instance = manage_instance();
	instance->max_session_accesses_in_lookup_delta = 2;
	instance->max_session_accesses_penalty_in_s = 100;
	instance->max_session_accesses_lookup_delta_in_s = 100;

	_u_request* request = manage_request();
	_u_response* response = manage_response();
	api_endpoint_t* endpoint = create_endpoint(response);

	ASSERT_EQ(api_auth_callback_check_ip_for_malicious_activity(request, response, instance), U_CALLBACK_COMPLETE);

	EXPECT_EQ(auth_blacklist_ip_counter, 1);
}


