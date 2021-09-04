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
 * @brief Setup functions for creating an ulfius instance.
 * @author Nils Egger
 *
 * @addtogroup libapi
 * @{
 */

#ifndef IKAG_INCLUDE_IKAG_SETUP_H
#define IKAG_INCLUDE_IKAG_SETUP_H

#include <stdint.h>
#include <arpa/inet.h>

#include <ulfius.h>

#include "radicle/pgdb.h"
#include "radicle/api/mail/sendgrid.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Contains cookie settings for session id.
 */
typedef struct api_cookie_config {
	bool secure; /**< If true, cookie will only be sent over https. */
	bool http_only; /**< If true, cookie will not be accessible by js. */
	int max_age; /**< Max age of session cookie. */
	int same_site; /**< One of U_COOKIE_SAME_SITE_STRICT, U_COOKIE_SAME_SITE_LAX, U_COOKIE_SAME_SITE_NONE. */ 
} api_cookie_config_t;

/**
 * @brief Copies original and returns a pointer to a new object.
 */
api_cookie_config_t* api_cookie_config_copy(const api_cookie_config_t* original);

/**
 * @brief Frees all data associated to the cooke config.
 *
 * @brief cookie_config Double pointer to config.
 */
void api_cookie_config_free(api_cookie_config_t** cookie_config);

/**
 * @brief Container for configuration which will be loaded from file on start up.
 */
typedef struct api_instance {
	struct sockaddr_in socket; /**< IP and port which will be used for socket */
	bool https; /**< If true, instance will use https */
	string_t* ssl_private_key_file; /**< Path to private key file. */
	string_t* ssl_certificate_file; /**< Path to certificate file. */
	pgdb_connection_queue_t* queue; /**< Queue which will be responsible for handing out connections to the database or refusing to do so. */
	string_t* conn_info; /**< PG auth */
	string_t* signature_key; /**< key used for signing cookies. */
	int max_post_param_size;
	int max_post_body_size;
	bool mirror_origin; /**< If true, Access Control Allow Origin will send back Origin supplied by requester. */
	string_t* default_access_control_allow_origin;
	string_t* default_access_control_allow_credentials;
	string_t* default_access_control_allow_methods;
	string_t* default_access_control_allow_headers;
	api_cookie_config_t* session_cookie;
	sendgrid_instance_t* sendgrid; /**< SendGrdi values like API key and tempalte ids; */
	string_t* verification_url; /**< URL which will be used for verifying registraiton codes. */
	string_t* verification_reroute_url; /**< URL to which users will be rerouted after completing verification. */
	string_t* password_reset_url; /**< URL which will be used for reseting password using a token. */
	string_t* no_associated_account_url; /**< Users will be routed to this url, if they tried to reset password for an email which does not exist. */
} api_instance_t;

/**
 * @brief Loads configuration from file and tries to safely set values. If not all values are set, returns 1.
 *
 * @param file Path to config file.
 * @param config Config which will be created.
 * 
 * @return Returns 0 if all values were set.
 */
int api_instance_load_from_file(const char* file, api_instance_t** config);

/**
 * @brief Frees all associated data to the config.
 */
void api_instance_free(api_instance_t** config);

/**
 * @brief Binds provided ip as address. 
 *
 * @param addr IP in text.
 * @param bind_to Pointer to struct to bind address to.
 * @param port Port to bind to
 *
 * @return Returns 0 for success.
 */
int api_bind(const char* addr, const unsigned int port, struct sockaddr_in* bind_to);

/**
 * @brief Initizalizes \ref pgdb_connection_queue_t and Ulfius instance.
 *
 * @param config Configuration to be used for instance. 
 * @param instance Pointer to instance wihich will be created.
 *
 * @returns Returns 0 for success.
 */
int api_setup_instance(api_instance_t* config, struct _u_instance* instance);

/**
 * @brief Starts instance and waits for getchar to finish.
 *
 * @param instance Ulfius instance to serve.
 *
 * @returns Returns 0 if successfull.
 */
int api_serve(struct _u_instance* instance);

/**
 * @brief Starts a secure instance and waits for getchar to finish.
 *
 * @param instance Ulfius instance to serve.
 * @param key_pem Private key data.
 * @param cert_pem Certificate data.
 *
 * @returns Returns 0 if successfull.
 */
int api_serve_secure(struct _u_instance* instance, const api_instance_t* config);

#if defined(__cplusplus)
}
#endif

#endif //IKAG_INCLUDE_IKAG_SETUP_H

/** @} */
