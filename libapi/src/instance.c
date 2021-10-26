/**
 * @file
 */

#include <jansson.h>
#include <openssl/pkcs7.h>
#include <string.h>
#include <arpa/inet.h>
#include <ulfius.h>
#include <signal.h>
#include <sys/wait.h>

#include "radicle/api/instance.h"
#include "radicle/api/endpoints/internal_codes.h"
#include "radicle/api/endpoints/endpoint.h"
#include "radicle/pgdb.h"
#include "radicle/config.h"

api_cookie_config_t* api_cookie_config_copy(const api_cookie_config_t* original) {
	api_cookie_config_t* copy = calloc(1, sizeof(api_cookie_config_t));
	copy->http_only = original->http_only;
	copy->secure = original->secure;
	copy->max_age = original->max_age;
	copy->same_site = original->same_site;
	return copy;
}

void api_cookie_config_free(api_cookie_config_t** cookie_config) {
	if(*cookie_config == NULL) return;
	free(*cookie_config);
	*cookie_config = NULL;
}

int api_config_get_bool(json_t* object, const char* key, bool* ptr) {
	json_t* data = json_object_get(object, key);
	if(data == NULL) {
		ERROR("Missing key %s.\n", key);
		return 1;
	} else if(!json_is_boolean(data)) {
		ERROR("Expected boolean for key %s.\n", key);
		return 1;
	}
	*ptr = json_boolean_value(data);
	return 0;
}

int api_config_get_string(json_t* object, const char* key, string_t** ptr) {
	json_t* data = json_object_get(object, key);
	if(data == NULL) {
		ERROR("Missing key %s.\n", key);
		return 1;
	} else if(!json_is_string(data)) {
		ERROR("Expected string for key %s.\n", key);
		return 1;
	}
	*ptr = string_from_literal(json_string_value(data));
	return 0;
}

int api_config_get_number(json_t* object, const char* key, int* ptr) {
	json_t* data = json_object_get(object, key);
	if(data == NULL) {
		ERROR("Missing key %s.\n", key);
		return 1;
	} else if(!json_is_number(data)) {
		ERROR("Expected number for key %s.\n", key);
		return 1;
	}
	*ptr = json_number_value(data);
	return 0;
}

int api_cookie_config_load(json_t* object, const char* key, api_cookie_config_t** cookie_config) {
	json_t* data = json_object_get(object, key);

	if(data == NULL) {
		ERROR("Missing key %s.\n", key);
		return 1;
	} else if(!json_is_object(data)) {
		ERROR("Expected object for key %s.\n", key);
		return 1;
	}
	
	*cookie_config = calloc(1, sizeof(api_cookie_config_t));

	if(api_config_get_bool(data, "http_only", &(*cookie_config)->http_only)) {
		ERROR("Missing http_only for cookie %s.\n", key);
		api_cookie_config_free(cookie_config);
		return 1;
	}

	if(api_config_get_bool(data, "secure", &(*cookie_config)->secure)) {
		ERROR("Missing secure for cookie %s.\n", key);
		api_cookie_config_free(cookie_config);
		return 1;
	}

	string_t* same_site = NULL;
	if(api_config_get_string(data, "same_site", &same_site)) {
		ERROR("Missing same_site for cookie %s.\n", key);
		api_cookie_config_free(cookie_config);
		return 1;
	}

	if(strcmp(same_site->ptr, "strict") == 0) {
		(*cookie_config)->same_site = U_COOKIE_SAME_SITE_STRICT;	
	} else if(strcmp(same_site->ptr, "lax") == 0) {
		(*cookie_config)->same_site = U_COOKIE_SAME_SITE_LAX;	
	} else if(strcmp(same_site->ptr, "none") == 0) {
		(*cookie_config)->same_site = U_COOKIE_SAME_SITE_NONE;	
	} else {
		string_free(&same_site);
		ERROR("same_site for cookie %s must be one of strict, lax or none.\n", key);
		api_cookie_config_free(cookie_config);
		return 1;

	}
	string_free(&same_site);

	if(api_config_get_number(data, "max_age", &(*cookie_config)->max_age)) {
		ERROR("Missing max_age for cookie %s.\n", key);
		api_cookie_config_free(cookie_config);
		return 1;
	}

	return 0;
}

int api_sendgrid_instance_load(json_t* object, const char* key, sendgrid_instance_t** sendgrid) {
	json_t* data = json_object_get(object, key);
	if(data == NULL) {
		ERROR("Missing key %s.\n", key);
		return 1;
	} else if(!json_is_object(data)) {
		ERROR("Expected object for key %s.\n", key);
		return 1;
	}

	*sendgrid = calloc(1, sizeof(sendgrid_instance_t));

	if(api_config_get_string(data, "api_key", &(*sendgrid)->apiKey)) {
		sendgrid_instance_free(sendgrid);
		return 1;
	}

	if(api_config_get_string(data, "sender", &(*sendgrid)->sender)) {
		sendgrid_instance_free(sendgrid);
		return 1;
	}

	if(api_config_get_string(data, "verification_template", &(*sendgrid)->verification_template)) {
		sendgrid_instance_free(sendgrid);
		return 1;
	}

	if(api_config_get_string(data, "duplicate_mail_template", &(*sendgrid)->duplicate_mail_tempate)) {
		sendgrid_instance_free(sendgrid);
		return 1;
	}

	if(api_config_get_string(data, "password_reset_template", &(*sendgrid)->password_reset_template)) {
		sendgrid_instance_free(sendgrid);
		return 1;
	}

	if(api_config_get_string(data, "no_associated_account_template", &(*sendgrid)->no_associated_account_template)) {
		sendgrid_instance_free(sendgrid);
		return 1;
	}

	if(api_config_get_string(data, "change_email_template", &(*sendgrid)->email_change_template)) {
		sendgrid_instance_free(sendgrid);
		return 1;
	}

	return 0;
}


int api_instance_load_from_file(const char* file, api_instance_t** config) {
	json_t* data = NULL;
	if(radicle_load_config(file, &data)) {
		return 1;
	}

	if(!json_is_object(data)) {
		json_decref(data);
		ERROR("Config root is not a json object.\n");
		return 1;
	}

	*config = calloc(1, sizeof(api_instance_t));

	string_t* ip;
	int port;
	if(api_config_get_string(data, "ip", &ip)) {
		json_decref(data);
		api_instance_free(config);
		return 1;
	}

	if(api_config_get_number(data, "port", &port)) {
		string_free(&ip);
		json_decref(data);
		api_instance_free(config);
		return 1;
	}

	if(api_bind(ip->ptr, port, &(*config)->socket)) {
		ERROR("Invalid ip and port. %s:%d\n", ip->ptr, port);
		string_free(&ip);
		json_decref(data);
		api_instance_free(config);
		return 1;
	};
	string_free(&ip);

	if(api_config_get_string(data, "conn_info", &(*config)->conn_info)) {
		json_decref(data);
		api_instance_free(config);
		return 1;
	}

	if(api_config_get_string(data, "signature_key", &(*config)->signature_key)) {
		json_decref(data);
		api_instance_free(config);
		return 1;
	}

	if(api_config_get_bool(data, "https", &(*config)->https)) {
		json_decref(data);
		api_instance_free(config);
		return 1;
	}

	if((*config)->https) {
		if(api_config_get_string(data, "ssl_private_key", &(*config)->ssl_private_key_file)) {
			json_decref(data);
			api_instance_free(config);
			return 1;
		}

		if(api_config_get_string(data, "ssl_certificate", &(*config)->ssl_certificate_file)) {
			json_decref(data);
			api_instance_free(config);
			return 1;
		}
	}

	if(api_config_get_number(data, "max_post_param_size", &(*config)->max_post_param_size)) {
		json_decref(data);
		api_instance_free(config);
		return 1;
	}

	if(api_config_get_number(data, "max_post_body_size", &(*config)->max_post_body_size)) {
		json_decref(data);
		api_instance_free(config);
		return 1;
	}
	
	if(api_config_get_bool(data, "mirror_origin", &(*config)->mirror_origin)) {
		json_decref(data);
		api_instance_free(config);
		return 1;
	}

	if(api_config_get_string(data, "default_cors_origin", &(*config)->default_access_control_allow_origin)) {
		json_decref(data);
		api_instance_free(config);
		return 1;
	}

	if(api_config_get_string(data, "default_cors_credentials", &(*config)->default_access_control_allow_credentials)) {
		json_decref(data);
		api_instance_free(config);
		return 1;
	}

	if(api_config_get_string(data, "default_cors_methods", &(*config)->default_access_control_allow_methods)) {
		json_decref(data);
		api_instance_free(config);
		return 1;
	}

	if(api_config_get_string(data, "default_cors_headers", &(*config)->default_access_control_allow_headers)) {
		json_decref(data);
		api_instance_free(config);
		return 1;
	}

	if(api_config_get_string(data, "verification_url", &(*config)->verification_url)) {
		json_decref(data);
		api_instance_free(config);
		return 1;
	}

	if(api_config_get_string(data, "verification_reroute_url", &(*config)->verification_reroute_url)) {
		json_decref(data);
		api_instance_free(config);
		return 1;
	}

	if(api_config_get_string(data, "password_reset_url", &(*config)->password_reset_url)) {
		json_decref(data);
		api_instance_free(config);
		return 1;
	}

	if(api_config_get_string(data, "no_associated_account_url", &(*config)->no_associated_account_url)) {
		json_decref(data);
		api_instance_free(config);
		return 1;
	}

	if(api_config_get_string(data, "change_email_url", &(*config)->change_email_url)) {
		json_decref(data);
		api_instance_free(config);
		return 1;
	}

	if(api_config_get_number(data, "max_session_accesses_in_lookup_delta", &(*config)->max_session_accesses_in_lookup_delta)) {
		json_decref(data);
		api_instance_free(config);
		return 1;
	}

	if(api_config_get_number(data, "max_session_accesses_penalty_in_s", (int*)&(*config)->max_session_accesses_penalty_in_s)) {
		json_decref(data);
		api_instance_free(config);
		return 1;
	}

	if(api_config_get_number(data, "max_session_accesses_lookup_delta_in_s", (int*)&(*config)->max_session_accesses_lookup_delta_in_s)) {
		json_decref(data);
		api_instance_free(config);
		return 1;
	}

	if(api_cookie_config_load(data, "session_cookie", &(*config)->session_cookie)) {
		json_decref(data);
		api_instance_free(config);
		return 1;
	}

	if(api_sendgrid_instance_load(data, "sendgrid", &(*config)->sendgrid)) {
		json_decref(data);
		api_instance_free(config);
		return 1;
	}

	json_decref(data);
	return 0;
}

void api_instance_free(api_instance_t** config) {
	if(*config == NULL) return;
	string_free(&(*config)->ssl_private_key_file);
	string_free(&(*config)->ssl_certificate_file);
	string_free(&(*config)->conn_info);
	string_free(&(*config)->signature_key);
	string_free(&(*config)->default_access_control_allow_origin);
	string_free(&(*config)->default_access_control_allow_credentials);
	string_free(&(*config)->default_access_control_allow_methods);
	string_free(&(*config)->default_access_control_allow_headers);
	string_free(&(*config)->verification_url);
	string_free(&(*config)->verification_reroute_url);
	sendgrid_instance_free(&(*config)->sendgrid);
	api_cookie_config_free(&(*config)->session_cookie);
	pgdb_connection_queue_free(&(*config)->queue);
	free(*config);
	*config = NULL;
}

int api_bind(const char* addr, const unsigned int port, struct sockaddr_in* bind_to) {
	memset(bind_to, 0, sizeof(*bind_to));
	bind_to->sin_family = AF_INET;
	bind_to->sin_port = htons(port);
	if(inet_pton(AF_INET, addr, &bind_to->sin_addr.s_addr) != 1) {
		return 1;
	}
	return 0;
}

/**
 * @brief Default callback which simply returns a 404 with a fitting error message.
 */
int callback_default(const struct _u_request * request, struct _u_response * response, void * user_data) {
	int result = api_callback_endpoint_init(request, response, user_data);

	if(result != U_CALLBACK_CONTINUE)
		result = api_callback_endpoint_check_for_session(request, response, user_data);

	return api_endpoint_respond(request, response, user_data, 404, api_response_object("Sorry but the resources you are looking for does not exist, have been removed. name changed or is temporarily unavailable."), NOT_FOUND);
}

int api_setup_instance(api_instance_t* config, struct _u_instance* instance) {

	// Initialize instance with the port number
	if (ulfius_init_instance(instance, ntohs(config->socket.sin_port), &config->socket, NULL) != U_OK) {
		return 1;
	}

	instance->max_post_param_size = config->max_post_param_size;
	instance->max_post_body_size = config->max_post_body_size; 

	u_map_put(instance->default_headers, "Access-Control-Allow-Origin", config->default_access_control_allow_origin->ptr);
	u_map_put(instance->default_headers, "Access-Control-Allow-Credentials", config->default_access_control_allow_credentials->ptr);
	u_map_put(instance->default_headers, "Access-Control-Allow-Methods", config->default_access_control_allow_methods->ptr);
	u_map_put(instance->default_headers, "Access-Control-Allow-Headers", config->default_access_control_allow_headers->ptr);
	u_map_put(instance->default_headers, "Content-Type", "application/json;charset=UTF-8");

	ulfius_set_default_endpoint(instance, callback_default, config);
	return 0;
}

int await_sigterm() {

	pause();

	return 0;
}

int api_serve(struct _u_instance* instance) {
	if (ulfius_start_framework(instance) == U_OK) {
		char txt_addr[INET_ADDRSTRLEN];
		if(inet_ntop(AF_INET, &instance->bind_address->sin_addr.s_addr, txt_addr, INET_ADDRSTRLEN) == NULL) {
			DEBUG("Serving on port %d\n", instance->port);
		} else {
			DEBUG("Serving on http://%s:%d\n", txt_addr, instance->port);
		}

		return await_sigterm();
	} else { 
		return 1;
	}
}

const char* read_ssl_file(const char* path) {
	FILE* file = fopen(path, "r");
	if(file) {
		fseek(file, 0, SEEK_END);
		long length = ftell(file);
		fseek(file, 0, SEEK_SET);

		char* content = malloc(length);
		fread(content, sizeof(char), length, file);
		fclose(file);
		return content;
	} else {
		ERROR("Unable to find %s.\n", path);
	}
	return NULL;
}

int api_serve_secure(struct _u_instance* instance, const api_instance_t* config) {
	const char* ssl_private_key = read_ssl_file(config->ssl_private_key_file->ptr);
	const char* ssl_certificate = read_ssl_file(config->ssl_certificate_file->ptr);

	if(ssl_private_key == NULL || ssl_certificate == NULL) {
		return 1;
	}

	if (ulfius_start_secure_framework(instance, ssl_private_key, ssl_certificate) == U_OK) {
		char txt_addr[INET_ADDRSTRLEN];
		if(inet_ntop(AF_INET, &instance->bind_address->sin_addr.s_addr, txt_addr, INET_ADDRSTRLEN) == NULL) {
			DEBUG("Serving on port %d\n", instance->port);
		} else {
			DEBUG("Serving on https://%s:%d\n", txt_addr, instance->port);
		}

		return await_sigterm();
	} else { 
		return 1;
	}
}
