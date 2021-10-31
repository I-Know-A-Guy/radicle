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

/**
 * @file
 * @brief Contains \ref api_endpoint_t which is passed between the different callbacks.
 * @addtogroup libapi 
 * @{
 * @addtogroup libapi_endpoints
 * @{
 */

#ifndef IKAG_INCLUDE_IKAG_ENDPOINTS_ENDPOINT_H
#define IKAG_INCLUDE_IKAG_ENDPOINTS_ENDPOINT_H

#include <ulfius.h>
#include "radicle/pgdb.h"
#include "radicle/auth/types.h"
#include "radicle/api/instance.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define DEFAULT_200_MSG "Ok"
#define DEFAULT_500_MSG "Failed to process request."
#define RESPOND(status, message, internal_status) api_endpoint_respond(request, response, user_data, status, api_response_object(message), internal_status);
#define RESPOND_JSON(status, message, internal_status) api_endpoint_respond(request, response, user_data, status, message, internal_status);

/**
 * @brief Checks if key is given in map, then converts it to int and sets
 * pointer.
 *
 * @param map Map to check key
 * @param key Key to extract from map
 * @param result Result pointer
 *
 * @return Returns 0 on success
 */
int api_map_get_int64(const struct _u_map* map, const char* key, int64_t* result);

/**
 * @brief Checks if key is given and then returns c_string as string_t
 *
 * @param map Map to check key
 * @param key Key to extract from map
 * @param result Result pointer
 *
 * @return Returns 0 on success
 */
int api_map_get_string(const struct _u_map* map, const char* key, string_t** result);

/**
 * @brief This struct will be initialized by callback before upload,
 * then uuid will be set by uploading callback to be further used by
 * next callback to save or update references to other tables
 */
typedef struct api_file_upload {
	string_t* relative_path; /**< Path without root folder */
	uuid_t* uuid; /**< UUID of uploaded file. */
	file_type_t allowed_files; /**< Bitor of all allowed files */
} api_file_upload_t;

/**
 * @brief Contains a connection to the database. Received by \ref pgdb_connection_queue_t
 */
typedef struct api_endpoint {
	pgdb_connection_t* conn; /**< Connection to database claimed from queue. Must be released when no longer in use */
	json_t* json_body; /**< Loaded and parsed http body. */
	auth_request_log_t* request_log; /**< Log which helps monitoring API. */

	uint32_t session; /**< Session id used for access logging. */
	bool authenticated;
	auth_account_t* account; /**< User accessing api. */
	bool refresh_cookie; /**< If requester already has a cookie, but a new one with account linked to it has to be created, set to true. */

	api_file_upload_t* file_upload;
} api_endpoint_t;

/**
 * @brief Initializes endoint and tries to claim a connection to the database. If it fails, it ends the callback chain.
 */
int api_callback_endpoint_init(const struct _u_request* request, struct _u_response * response, void * user_data);

/**
 * @brief Loads body json, if it fails, cancels the callback chain.
 */
int api_callback_endpoint_load_json_body(const struct _u_request* request, struct _u_response * response, void * user_data);

/**
 * @brief If ip is blacklisted, terminates request. 
 */
int api_auth_callback_check_blacklist(const struct _u_request * request, struct _u_response * response, void * user_data);

/**
 * @brief Checks if ip has typical malicious behaviour
 *
 * @todo implement properly 
 */
int api_auth_callback_check_ip_for_malicious_activity(const struct _u_request * request, struct _u_response * response, void * user_data);

/**
 * @brief Checks if request contains cookie for a session. Aka authentication.
 */
int api_callback_endpoint_check_for_session(const struct _u_request* request, struct _u_response * response, void * user_data);

/**
 * @brief If not authenticated, returns not authorized.
 */
int api_callback_endpoint_check_for_authentication(const struct _u_request* request, struct _u_response * response, void * user_data);

/**
 * @brief Checks that user has verified his email.
 */
int api_callback_endpoint_check_for_verified_email(const struct _u_request* request, struct _u_response * response, void * user_data);

/**
 * @brief Logs request with given internal_status to database.
 *
 * @param request Request given by ulfius containing url and such
 * @param endpoint Endpoint calling this function
 * @param http_status HTTP status which will be returned to user, if this log
 * happens inbetween, use 0
 * @param internal_status Internal Status which should give more detail.
 *
 * @param Returns 0
 */
int api_endpoint_log(const struct _u_request* request, api_endpoint_t* endpoint, const unsigned int http_status, const int internal_status);

/**
 * @brief Finishes and clears endpoint, saves access log.
 *
 * @param request Request by client.
 * @param response Response to be sent.
 * @param instance Instance containing configuration values.
 * @param http_status Status to be set
 * @param body HTTP body which will be set.
 * @param internal_status Status which will be written to database.
 *
 * @return Returns U_CALLBACK_COMPLETE
 */
int api_endpoint_respond(const struct _u_request* request, struct _u_response * response, api_instance_t* instance, unsigned int http_status, json_t* body, const int internal_status);

/**
 * @brief Creates a simple json_t object with one member called message.
 */
json_t* api_response_object(const char* message);

/**
 * @brief Creates response object containing message ok.
 */
json_t* api_response_object_ok();


/**
 * @brief Simple options callback which returns default headers.
 */
int api_default_options_callback(const struct _u_request * request, struct _u_response * response, void * user_data);

/**
 * @brief Cleans up and endpoint and releases its connection.
 *
 * @param endpoint Pointer to endpoint data.
 *
 */
void api_endpoint_free(api_endpoint_t* endpoint);

/**
 * @brief Adds endpoint to instance.
 *
 * @param instance Ulfius Library instance
 * @param method HTTP method to bind to endpoint
 * @param url URL of endpoint
 * @param api_instance IKAG Instance containing global properties.
 * @param authenticated If true, endpoint is only allowed to be used
 * authenticated.
 * @param jsonBody if true, endpoint will load json body and return an error if
 * none is give.
 */
void api_add_endpoint(struct _u_instance* instance, const char* method, const char* url, int (* callback_function)(const struct _u_request * request, // Input parameters (set by the framework)
                                                         struct _u_response * response,     // Output parameters (set by the user)
                                                         void * user_data), api_instance_t* api_instance, bool authenticated, bool verified, bool jsonBody);

/**
 * @brief If rollback fails, resets connection so that open transaction wont be
 * transfered to next person.
 */
void api_endpoint_safe_rollback(const struct _u_request* request, struct _u_response * response);

#if defined(__cplusplus)
}
#endif

#endif //IKAG_INCLUDE_IKAG_ENDPOINTS_ENDPOINT_H

/** @} */
/** @} */
