/**
 * @file
 * @brief Starting point of program. 
 * @todo set up tests using gtest and fake request objects
 */
#include <stdio.h>
#include <string.h>

#include <ulfius.h>

#include "radicle/pgdb.h"
#include "radicle/print.h"
#include "radicle/api/endpoints/endpoint.h"
#include "radicle/api/instance.h"
#include "radicle/api/endpoints/auth.h"
#include "radicle/api/mail/sendgrid.h"

// @todo program crashes if config file path is invalid
int main(int argc, char* argv[]) {
	
	if(argc != 2) {
		ERROR("Please supply path to config file.\n");
		return 1;
	}

	api_instance_t* ikag_instance = NULL;

	if(api_instance_load_from_file(argv[1], &ikag_instance)) {
		ERROR("Failed to load %s.\n", argv[1]);
		return 1;
	}

	// @todo  add parameters to config
	ikag_instance->queue = pgdb_connection_queue_new(ikag_instance->conn_info->ptr, 10, 60);

	struct sockaddr_in bind_to;
	struct _u_instance instance;

	ulfius_global_init();

	if(api_setup_instance(ikag_instance, &instance)) {
		ERROR("Failed to setup instance.\n");
		api_instance_free(&ikag_instance);
		return 1;
	}

	api_add_endpoint(&instance, "POST", "/accounts/register", &api_auth_callback_register, ikag_instance, false, false, true);
	api_add_endpoint(&instance, "POST", "/accounts/send-reset", &api_auth_callback_send_password_reset, ikag_instance, false, false, true);
	api_add_endpoint(&instance, "POST", "/accounts/reset", &api_auth_callback_reset_password, ikag_instance, false, false, true);
	api_add_endpoint(&instance, "POST", "/accounts/resend", &api_auth_callback_resend_verification_mail, ikag_instance, true, false, false);
	api_add_endpoint(&instance, "GET", "/accounts/verify", &api_auth_callback_register_verify, ikag_instance, false, false, false);
	api_add_endpoint(&instance, "POST", "/accounts/login", &api_auth_callback_sign_in, ikag_instance, false, false, true);
	api_add_endpoint(&instance, "GET", "/accounts/cookie-info", &api_auth_callback_cookie_info, ikag_instance, true, false, false);
	
	if(ikag_instance->https) {
		if(api_serve_secure(&instance, ikag_instance)) {
			ERROR("Failed to start secure instance.\n");
		}
	} else {
		INFO("!!! Not using ssl. !!!\n");
		if(api_serve(&instance)) {
			ERROR("Failed to start instance.\n");
		}
	}

	api_instance_free(&ikag_instance);

	ulfius_stop_framework(&instance);
	ulfius_clean_instance(&instance);
	ulfius_global_close();

	return 0;
}
