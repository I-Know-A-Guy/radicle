#include "radicle/tests/pgdb_hooks.hpp"
#include <ulfius.h>

class APITests: public RadiclePGDBHooks {

	std::vector<_u_request*> requests; 
	std::vector<_u_response*> responses; 

	protected:

	void TearDown() {
		RadiclePGDBHooks::TearDown();
		for(std::vector<_u_request*>::iterator iter = requests.begin(); iter != requests.end(); iter++) {
			ulfius_clean_request(*iter.base());
		}

		for(std::vector<_u_response*>::iterator iter = responses.begin(); iter != responses.end(); iter++) {
			ulfius_clean_response(*iter.base());
		}
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
