#include "radicle/api/instance.h"
#include "radicle/tests/pgdb_hooks.hpp"
#include <ulfius.h>

class APITests: public RadiclePGDBHooks {

	std::vector<api_instance_t*> instances; 
	std::vector<_u_request*> requests; 
	std::vector<_u_response*> responses; 

	protected:

	void TearDown() {
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
		return instance;
	}

	_u_request* manage_request() {
		struct _u_request* request = (struct _u_request*)calloc(1, sizeof(struct _u_request));
		ulfius_init_request(request);

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
};