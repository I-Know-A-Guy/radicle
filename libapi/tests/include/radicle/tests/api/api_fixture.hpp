#include "radicle/api/endpoints/endpoint.h"
#include "radicle/api/instance.h"
#include "radicle/pgdb.h"
#include "radicle/tests/pgdb_hooks.hpp"
#include <ulfius.h>

#define API_EMPTY_RESULT(name)\
       	PGDB_FAKE_FETCH(name) {\
	PGDB_FAKE_EMPTY_RESULT(PGRES_TUPLES_OK);\
}

#define API_FAKE_VERIFIED_ACCOUNT(name, index) \
	PGDB_FAKE_STORY_BRANCH(name, index);\
		PGDB_FAKE_RESULT_6(PGRES_TUPLES_OK, "uuid", "password", "role", "verified", "active", "created");\
		char name ## index ## fake_uuid[16] = {0x1};\
		PGDB_FAKE_UUID(name ## index ## fake_uuid);\
		PGDB_FAKE_C_STR("password");\
		PGDB_FAKE_C_STR(auth_account_role_to_str(ROLE_USER));\
		PGDB_FAKE_BOOL(true);\
		PGDB_FAKE_BOOL(true);\
		PGDB_FAKE_TIMESTAMP(time(NULL));\
		PGDB_FAKE_FINISH();\
	PGDB_FAKE_STORY_BRANCH_END();

#define API_FAKE_SESSION(name, index)\
	PGDB_FAKE_STORY_BRANCH(name, index);\
		PGDB_FAKE_RESULT_1(PGRES_TUPLES_OK, "id");\
		PGDB_FAKE_INT(1234);\
		PGDB_FAKE_FINISH();\
	PGDB_FAKE_STORY_BRANCH_END();

class APITests: public RadiclePGDBHooks {

	std::vector<api_instance_t*> instances; 
	std::vector<_u_request*> requests; 
	std::vector<_u_response*> responses; 

	protected:

	api_instance_t* instance;
	_u_request* request;
	_u_response* response;
	api_endpoint_t* endpoint;


	void SetUp() override {
		instance = manage_instance();
		request = manage_request();
		response = manage_response();
		endpoint = create_endpoint(response);
		RadiclePGDBHooks::SetUp();
	}

	void TearDown() override {
		RadiclePGDBHooks::TearDown();
		for(std::vector<api_instance_t*>::iterator iter = instances.begin(); iter != instances.end(); iter++) {
			api_instance_free(iter.base());
		}

		for(std::vector<_u_request*>::iterator iter = requests.begin(); iter != requests.end(); iter++) {
			ulfius_clean_request(*iter.base());
		}

		for(std::vector<_u_response*>::iterator iter = responses.begin(); iter != responses.end(); iter++) {
			ulfius_clean_response(*iter.base());
		}
	}

	api_instance_t* manage_instance() {
		api_instance_t* instance  = (api_instance_t*)calloc(1, sizeof(api_instance_t));
		instance->queue = pgdb_connection_queue_new("conn info", 10, 10);
		instance->signature_key = string_from_literal("signature key");
		instance->session_cookie = (api_cookie_config_t*)calloc(1, sizeof(api_cookie_config_t));
		instance->session_cookie->max_age = 0;
		instance->session_cookie->same_site = U_COOKIE_SAME_SITE_NONE;
		instance->verification_reroute_url = string_from_literal("Reroute Url");
		instance->password_reset_url = string_from_literal("pw reset url");
		return instance;
	}

	_u_request* manage_request() {
		struct _u_request* request = (struct _u_request*)calloc(1, sizeof(struct _u_request));
		ulfius_init_request(request);

		u_map_put(request->map_header, "content-type", "application/json");

		request->client_address = (struct sockaddr*)calloc(1, sizeof(sockaddr_in));
		request->client_address->sa_family = AF_INET;
		((struct sockaddr_in*)(request->client_address))->sin_port = 0;
		inet_pton(AF_INET, "127.0.0.1", &((struct sockaddr_in*)(request->client_address))->sin_addr);

		request->url_path = strdup("testing");
		requests.push_back(request);
		return request;
	}

	_u_response* manage_response() {
		struct _u_response* response = (struct _u_response*)calloc(1, sizeof(struct _u_response));
		ulfius_init_response(response);
		responses.push_back(response);
		return response;
	}

	api_endpoint_t* create_endpoint(_u_response* response) {
		api_endpoint_t* endpoint = (api_endpoint_t*)calloc(1, sizeof(api_endpoint_t));
		endpoint->conn = (pgdb_connection_t*)calloc(1, sizeof(pgdb_connection_t));
		endpoint->request_log = auth_request_log_new();
		response->shared_data = endpoint;
		return endpoint;
	}

	void authenticate_endpoint() {
		endpoint->account = (auth_account_t*)calloc(1, sizeof(auth_account_t));
		unsigned char uuid[16] = {0x0};
		endpoint->account->uuid = uuid_new(uuid);
		endpoint->account->email = string_from_literal("account-email");
		endpoint->authenticated = true;
	}
};
