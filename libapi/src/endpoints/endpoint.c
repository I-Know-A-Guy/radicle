/**
 * @file
 */

#include <jansson.h>
#include <libpq-fe.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ulfius.h>

#include "radicle/auth.h"
#include "radicle/auth/db.h"
#include "radicle/auth/types.h"
#include "radicle/pgdb.h"
#include "radicle/print.h"
#include "radicle/api/endpoints/endpoint.h"
#include "radicle/api/instance.h"
#include "radicle/types/string.h"
#include "radicle/api/endpoints/internal_codes.h"
#include "radicle/types/uuid.h"

int socket_info(const struct sockaddr* address, string_t** buffer, unsigned int* port) {
	*buffer = calloc(1, sizeof(string_t));
	(*buffer)->length = NI_MAXHOST;
	(*buffer)->ptr = calloc(NI_MAXHOST, sizeof(char));

	if (getnameinfo(address, sizeof(*address), (*buffer)->ptr, NI_MAXHOST, NULL, 0, NI_NUMERICHOST)) {
		ERROR("Failed to translate ip.\n");
	} 

	*port = ((struct sockaddr_in*)address)->sin_port;
	return 0;
}

int api_callback_endpoint_init(const struct _u_request * request, struct _u_response * response, void * user_data) {

	
                
	if(user_data == NULL) {
		DEBUG("Missing user_data for %s.\n", request->http_url);
		return U_CALLBACK_ERROR;
	}
	api_instance_t* instance = (api_instance_t*)user_data;
	api_endpoint_t* endpoint = calloc(1, sizeof(api_endpoint_t));

	endpoint->request_log = auth_request_log_new();
	endpoint->request_log->date = time(NULL);

	if(socket_info(request->client_address, &endpoint->request_log->ip, &endpoint->request_log->port)) {
		ERROR("Failed to convert ip to text.\n");
		endpoint->request_log->ip = string_from_literal("?.?.?.?");
	}
	endpoint->request_log->url = string_from_literal(request->url_path);

	response->shared_data = endpoint;

	if(pgdb_claim_connection(instance->queue, &endpoint->conn))
		return RESPOND(503, "Service is currently unavailable, please try again later.", PGDB_UNABLE_TO_CLAIM);

	/* No free connection. */
	if(endpoint->conn == NULL) 
		return RESPOND(503, "Service is currently unavailable, please try again later.", PGDB_UNABLE_TO_CLAIM);

	return U_CALLBACK_CONTINUE;
}

int api_callback_endpoint_load_json_body(const struct _u_request* request, struct _u_response * response, void * user_data) {
	api_endpoint_t* endpoint = response->shared_data;
	if(request->binary_body_length == 0)
		return RESPOND(400, "JSON body is expected for this endpoint.", VALIDATION_MISSING_JSON_BODY);

	json_error_t error;
	endpoint->json_body = ulfius_get_json_body_request(request, &error);
	// there was either no json received, or there was an error unpacking the json.
	if(endpoint->json_body == NULL) {
		return RESPOND(400, "JSON body is expected for this endpoint.", VALIDATION_MISSING_JSON_BODY);
	} else if(endpoint->json_body->type != JSON_OBJECT)
		return RESPOND(400, "JSON body is expected for this endpoint.", VALIDATION_MISSING_JSON_BODY);

	return U_CALLBACK_CONTINUE;
}

int api_auth_callback_check_blacklist(const struct _u_request * request, struct _u_response * response, void * user_data) {
	api_instance_t* instance = user_data;
	api_endpoint_t* endpoint = response->shared_data;

	bool blacklisted = false;
	if(auth_blacklist_lookup_ip(endpoint->conn->connection, endpoint->request_log->ip, &blacklisted))
		return RESPOND(500, DEFAULT_500_MSG, ERROR_BLACKLIST_LOOKUP);

	if(blacklisted) {
		/** @todo save blacklist access */
		/** @todo add contact mail */
		return RESPOND(403, "Your ip has been blocked. If you believe this has been misjudged, please reach out to <mail>.", FORBIDDEN_BLACKLIST_IP);
	}

	return U_CALLBACK_CONTINUE;
}

int api_auth_callback_check_ip_for_malicious_activity(const struct _u_request * request, struct _u_response * response, void * user_data) {
	api_instance_t* instance = user_data;
	api_endpoint_t* endpoint = response->shared_data;
	

	list_t* results = NULL;
	if(auth_session_lookup_ip(endpoint->conn->connection, endpoint->request_log->ip, time(NULL) - instance->session_access_lookup_delta, &results)) {
		return RESPOND(500, DEFAULT_500_MSG, ERROR_SESSION_ACCESS_LOOKUP);
	}

	/**
	 * @todo this is currently very badly implemented, since this could
	 * simply be done by a SQL count query.
	 */
	if(results != NULL) {
		int counter = 0;
		list_t* iter = results;
		while(iter != NULL) {
			auth_session_access_entry_t* entry = (auth_session_access_entry_t*)iter->data;
			
			/** @todo maybe check how many resopnsed were 500 or
			 * whatever. */

			counter++;
			iter = iter->next;
		} 

		if(counter >= instance->max_session_accesses_in_lookup_delta) {
			uint32_t id;
			if(auth_blacklist_ip(endpoint->conn->connection,
					       	endpoint->request_log->ip, time(NULL),
					       	time(NULL) + instance->max_session_accesses_breach_penalty_in_s, &id)) {
				return RESPOND(500, DEFAULT_500_MSG, ERROR_SAVING_BLACKLIST);
			}	

			/**
			 * @todo save blacklist access
			 */

			return RESPOND(403, "Your ip has been blocked.", FORBIDDEN_BLACKLIST_IP);
		}

	}

	return U_CALLBACK_CONTINUE;
}

int api_callback_endpoint_check_for_session(const struct _u_request* request, struct _u_response * response, void * user_data) {
	api_instance_t* instance = user_data;
	api_endpoint_t* endpoint = response->shared_data;

	if(u_map_has_key(request->map_cookie, "session-id")) {
		string_t* cookie_raw = string_from_literal(u_map_get(request->map_cookie, "session-id"));
		int error = auth_verify_cookie(endpoint->conn->connection, instance->signature_key, cookie_raw, &endpoint->session, &endpoint->account);
		if(error == AUTH_ACCOUNT_NOT_ACTIVE) {
			string_free(&cookie_raw);
			return RESPOND(403, "Your account has been deactivated.", VALIDATION_ACCOUNT_DEACTIVATED);
		} else if(error == AUTH_ERROR) {
			string_free(&cookie_raw);
			return RESPOND(500, "There is something wrong with your cookie.", VALIDATION_INVALID_COOKIE);
		} else if(error) {
			string_free(&cookie_raw);
			return RESPOND(400, "Cookie is invalid.", VALIDATION_INVALID_COOKIE);
		} else if(endpoint->account != NULL && error == AUTH_OK) {
			endpoint->authenticated = true;
		}
		string_free(&cookie_raw);
	}
	return U_CALLBACK_CONTINUE;
}

int api_callback_endpoint_check_for_authentication(const struct _u_request* request, struct _u_response * response, void * user_data) {
	api_instance_t* instance = user_data;
	api_endpoint_t* endpoint = response->shared_data;

	if(!endpoint->authenticated)
		return RESPOND(401, "Authentication required.", VALIDATION_NOT_AUTHENTICATED);

	return U_CALLBACK_CONTINUE;
}

int api_callback_endpoint_check_for_verified_email(const struct _u_request* request, struct _u_response * response, void * user_data) {
	api_instance_t* instance = user_data;
	api_endpoint_t* endpoint = response->shared_data;

	if(!endpoint->authenticated || !endpoint->account->verified)
		return RESPOND(401, "Authentication and verification required.", VALIDATION_NOT_AUTHENTICATED);

	return U_CALLBACK_CONTINUE;
}

int api_endpoint_log(const struct _u_request* request, api_endpoint_t* endpoint, const unsigned int http_status, const int internal_status) {
	
	endpoint->request_log->response_code = http_status;
	endpoint->request_log->internal_status = internal_status;
	auth_request_log_calculate_response_time(endpoint->request_log);
		
	if(endpoint->session != 0) {
		if(endpoint->conn != NULL && auth_log_access(endpoint->conn->connection, endpoint->session, endpoint->request_log)) {
			INFO("%s:%d %d %d %s\n", endpoint->request_log->ip->ptr, endpoint->request_log->port, http_status, internal_status, internal_errors_msg(internal_status));
		} else if(endpoint->conn == NULL)
			INFO("%s:%d %d %d %s\n", endpoint->request_log->ip->ptr, endpoint->request_log->port, http_status, internal_status, internal_errors_msg(internal_status));

	} else
		INFO("%s:%d %d %d %s\n", endpoint->request_log->ip->ptr, endpoint->request_log->port, http_status, internal_status, internal_errors_msg(internal_status));

	return 0;
}

/**
 * @brief If session id is already set, no new session will be created, otherwise, depending if endpoint->account is set, either
 * a owner or unowned session is created.
 */
int api_endpoint_manage_session(struct _u_response * response, api_instance_t* instance, api_endpoint_t* endpoint) {

	if(endpoint->conn != NULL && endpoint->session == 0 || endpoint->refresh_cookie) {
		auth_cookie_t* cookie = NULL;
		if(!endpoint->authenticated && auth_make_free_session(endpoint->conn->connection, instance->signature_key, &cookie, &endpoint->session)) {
			ERROR("Unable to create free session.\n");
			/**
			 * This error isnt as bad as the one below, since the
			 * cookie is not really required.
			 */
			return 0;
		} else if(endpoint->authenticated && auth_make_owned_session(endpoint->conn->connection, endpoint->account->uuid, instance->signature_key, &cookie, &endpoint->session)) {
			ERROR("Unable to create owned session.\n");
			return 0;
		}

		ulfius_add_same_site_cookie_to_response(response, "session-id", cookie->cookie->ptr,
			NULL, instance->session_cookie->max_age, NULL, "/", instance->session_cookie->secure, instance->session_cookie->http_only, instance->session_cookie->same_site); 
		auth_cookie_free(&cookie);
	}
	return 0;
}

void api_request_log(const struct _u_request* request, auth_request_log_t* log, uuid_t* uuid, unsigned int status) {
	const char* uuid_c = NULL;
	string_t* uuid_string = NULL;
	if(uuid != NULL) {
		uuid_string = uuid_to_str(uuid);
		uuid_c = uuid_string->ptr;
	}
	struct tm* tmdate = localtime(&log->date);
	char date[100];
	strftime(date, 100, "%d/%m/%Y:%H:%M:%S %z", tmdate);
	// URL, identd, user uuid, date, method, url, http version, status, time
	DEBUG("%s - %s [%s] \"%s %s %s\" %d %d\n", log->ip->ptr, uuid_c, date,
		       	request->http_verb, request->http_url, request->http_protocol, status, log->response_time);
	string_free(&uuid_string);
}

int api_endpoint_respond(const struct _u_request* request, struct _u_response * response, api_instance_t* instance, unsigned int http_status, json_t* body, const int internal_status) {
	DEBUG("%s\n", internal_errors_msg(internal_status));
	api_endpoint_t* endpoint = response->shared_data;
	if(endpoint != NULL) {
		// Only fails if it wasnt possible to create new cookie for
		// login
		if(api_endpoint_manage_session(response, instance, endpoint)) {
			http_status = 500;
			json_decref(body);
			body = api_response_object("Failed to create new session.");
		}

		api_endpoint_log(request, endpoint, http_status, internal_status);

		if(endpoint->account != NULL) 
			api_request_log(request, endpoint->request_log, endpoint->account->uuid, http_status);
		else 
			api_request_log(request, endpoint->request_log, NULL, http_status);

		api_endpoint_free(endpoint);
	}

	if(instance->mirror_origin) {
		const char* origin = u_map_get_case(request->map_header, "Origin");
		if(origin != NULL) {
			//ulfius_add_header_to_response(response, "Access-Control-Allow-Origin", origin);
			u_map_put(response->map_header, "Access-Control-Allow-Origin", origin);
		}
	}

	json_object_set(body, "status", json_integer(http_status));
	ulfius_set_json_body_response(response, http_status, body);
	json_decref(body);
	return U_CALLBACK_COMPLETE;
}

int api_default_options_callback(const struct _u_request * request, struct _u_response * response, void * user_data) {
	return api_endpoint_respond(request, response, user_data, 200, NULL, OPTION_RESPONSE);
}

json_t* api_response_object(const char* message) {
	return json_pack("{ss}", "message", message);
}

json_t* api_response_object_ok() {
	return json_pack("{ss}", "message", "ok");
}

void api_endpoint_free(api_endpoint_t* endpoint) {
	if(endpoint == NULL) return;
	pgdb_release_connection(&endpoint->conn);
	if(endpoint->json_body != NULL) {
		json_decref(endpoint->json_body);
	}
	auth_account_free(&endpoint->account);
	auth_request_log_free(&endpoint->request_log);
	free(endpoint);
}

void api_add_endpoint(struct _u_instance* instance, const char* method, const char* url, int (* callback_function)(const struct _u_request * request, // Input parameters (set by the framework)
                                                         struct _u_response * response,     // Output parameters (set by the user)
                                                         void * user_data), api_instance_t* api_instance, bool authenticated, bool verified, bool jsonBody) {
	ulfius_add_endpoint_by_val(instance, "OPTIONS", url, NULL, 0, &api_default_options_callback, api_instance);

	int counter = 0;

	ulfius_add_endpoint_by_val(instance, method, url, NULL, counter++, &api_callback_endpoint_init, api_instance);
	ulfius_add_endpoint_by_val(instance, method, url, NULL, counter++, &api_auth_callback_check_blacklist, api_instance);
	ulfius_add_endpoint_by_val(instance, method, url, NULL, counter++, &api_auth_callback_check_ip_for_malicious_activity, api_instance);
	ulfius_add_endpoint_by_val(instance, method, url, NULL, counter++, &api_callback_endpoint_check_for_session, api_instance);
	if(authenticated && !verified)
		ulfius_add_endpoint_by_val(instance, method, url, NULL, counter++, &api_callback_endpoint_check_for_authentication, api_instance);
	else if(verified) 
		ulfius_add_endpoint_by_val(instance, method, url, NULL, counter++, &api_callback_endpoint_check_for_verified_email, api_instance);

	if(jsonBody)
		ulfius_add_endpoint_by_val(instance, method, url, NULL, counter++, &api_callback_endpoint_load_json_body, api_instance);
	ulfius_add_endpoint_by_val(instance, method, url, NULL, counter++, callback_function, api_instance);
}

void api_endpoint_safe_rollback(const struct _u_request* request, struct _u_response * response) {
	api_endpoint_t* endpoint = response->shared_data;
	if(pgdb_transaction_rollback(endpoint->conn->connection)) {
		PQreset(endpoint->conn->connection);
		api_endpoint_log(request, endpoint, 0, ERROR_TRANSACTION_ROLLBACK);
	}
}
