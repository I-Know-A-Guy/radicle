/**
 * @file
 */
#include <string.h>

#include <jansson.h>
#include <ulfius.h>
#include <errno.h>

#include "radicle/api/mail/sendgrid.h"
#include "radicle/auth/db.h"
#include "radicle/auth/types.h"
#include "radicle/auth/crypto.h"
#include "radicle/auth.h"
#include "radicle/pgdb.h"

#include "radicle/api/instance.h"
#include "radicle/api/json_validate.h"
#include "radicle/api/endpoints/endpoint.h"
#include "radicle/api/endpoints/auth.h"
#include "radicle/api/endpoints/internal_codes.h"
#include "radicle/types/uuid.h"

int api_auth_callback_register(const struct _u_request * request, struct _u_response * response, void * user_data) {

	api_instance_t* instance = user_data;
	api_endpoint_t* endpoint = response->shared_data;

	endpoint->account = auth_account_new(NULL, NULL, NULL, ROLE_USER, true, false, 0);
	if(api_json_validate_email(endpoint->json_body, "email", &endpoint->account->email))
		return RESPOND(422, "Supplied email is invalid", VALIDATION_INVALID_EMAIL);

	if(api_json_validate_password(endpoint->json_body, "password", &endpoint->account->password))
		return RESPOND(422, "Supplied password is either too short, long or not complex enough.", VALIDATION_INVALID_PASSWORD);

	if(pgdb_transaction_begin(endpoint->conn->connection))
		return RESPOND(500, DEFAULT_500_MSG, ERROR_TRANSACTION_BEGIN);

	auth_account_t* duplicate_account = NULL;
	if(auth_get_account_by_email(endpoint->conn->connection, endpoint->account->email, &duplicate_account)) {
		api_endpoint_safe_rollback(request, response, instance);
		return RESPOND(500, DEFAULT_500_MSG, ERROR_EMAIL_LOOKUP);
	}

	if(duplicate_account != NULL) {
		auth_account_free(&duplicate_account);
		// fake hash to minimize difference of response times
		string_t* fake_pw_buffer = NULL;
		if(auth_hash_password(endpoint->account->password, &fake_pw_buffer))
			DEBUG("Failed to fake hash password.\n");
		string_free(&fake_pw_buffer);

		if(send_duplicate_mail_notify(instance->sendgrid, endpoint->account->email))
			api_endpoint_log(request, instance, endpoint, 0, ERROR_SEND_MAIL_DUPLICATE_REGISTER_NOTIFY);

		api_endpoint_safe_rollback(request, response, instance);
		return RESPOND(200, "A verification email has been sent to your email.", DUPLICATE_EMAIL_REGISTER);
	}

	if(auth_register(endpoint->conn->connection, endpoint->account)) {
		api_endpoint_safe_rollback(request, response, instance);
		return RESPOND(500, DEFAULT_500_MSG, ERROR_REGISTER);
	}

	string_t* registration_token = NULL;
	if(auth_create_token(endpoint->conn->connection, endpoint->account->uuid, REGISTRATION, NULL, &registration_token)) {
		api_endpoint_safe_rollback(request, response, instance);
		return RESPOND(500, DEFAULT_500_MSG, ERROR_CREATING_TOKEN);
	}

	if(pgdb_transaction_commit(endpoint->conn->connection)) {
		string_free(&registration_token);
		api_endpoint_safe_rollback(request, response, instance);
		return RESPOND(500, DEFAULT_500_MSG, ERROR_TRANSACTION_COMMIT);
	}

	if(send_verification_mail(instance->sendgrid, endpoint->account->email, instance->verification_url, registration_token))
		api_endpoint_log(request, instance, endpoint, 0, ERROR_SEND_MAIL_REGISTER_TOKEN);

	string_free(&registration_token);

	return RESPOND(200, "A verification email has been sent to your email.", SUCCESS_REGISTER);
}

int api_auth_callback_resend_verification_mail(const struct _u_request * request, struct _u_response * response, void * user_data) {
	api_instance_t* instance = user_data;
	api_endpoint_t* endpoint = response->shared_data;

	if(endpoint->account->verified)
		return RESPOND(400, "Your mail has already been verified!", VALIDATION_EMAIL_ALREADY_VERIFIED);

	if(pgdb_transaction_begin(endpoint->conn->connection))	
		return RESPOND(500, DEFAULT_500_MSG, ERROR_TRANSACTION_BEGIN);

	if(auth_remove_token_by_owner(endpoint->conn->connection, endpoint->account->uuid, REGISTRATION)) {
		api_endpoint_safe_rollback(request, response, instance);
		return RESPOND(500, DEFAULT_500_MSG, ERROR_REVOKE_REGISTRATIONS_TOKEN);
	}

	string_t* registration_token = NULL;
	if(auth_create_token(endpoint->conn->connection, endpoint->account->uuid, REGISTRATION, NULL, &registration_token)) {
		api_endpoint_safe_rollback(request, response, instance);
		return RESPOND(500, DEFAULT_500_MSG, ERROR_CREATING_TOKEN);
	}

	if(pgdb_transaction_commit(endpoint->conn->connection)) {
		string_free(&registration_token);
		api_endpoint_safe_rollback(request, response, instance);
		return RESPOND(500, DEFAULT_500_MSG, ERROR_TRANSACTION_COMMIT);
	}

	if(send_verification_mail(instance->sendgrid, endpoint->account->email, instance->verification_url, registration_token))
		api_endpoint_log(request, instance, endpoint, 0, ERROR_SEND_MAIL_REGISTER_TOKEN);

	string_free(&registration_token);

	return RESPOND(200, DEFAULT_200_MSG, SUCCESS);
}

int api_auth_callback_register_verify(const struct _u_request * request, struct _u_response * response, void * user_data) {

	api_endpoint_t* endpoint = response->shared_data;
	api_instance_t* instance = user_data;

	/**
	 * somehow there is a case where url?t= yields a length of 1 instead of
	 * 0
	 */
	if(!u_map_has_key(request->map_url, "t") || u_map_get_length(request->map_url, "t") <= 1) 
		return RESPOND(400, "Missing parameter for token.", VALIDATION_MISSING_PARAMETER);

	if(pgdb_transaction_begin(endpoint->conn->connection))
		return RESPOND(500, DEFAULT_500_MSG, ERROR_TRANSACTION_BEGIN);

	string_t* token = string_from_literal(u_map_get(request->map_url, "t"));
	uuid_t* owner = NULL;
	if(auth_verify_token(endpoint->conn->connection, token, REGISTRATION, &owner, NULL)) {
		string_free(&token);
		api_endpoint_safe_rollback(request, response, instance);
		return RESPOND(500, DEFAULT_500_MSG, ERROR_VERIFYING_TOKEN);
	}

	string_free(&token);

	if(owner == NULL)  {
		api_endpoint_safe_rollback(request, response, instance);
		char location_url[instance->verification_reroute_url->length + 16];
		sprintf(location_url, "%s?verified=false", instance->verification_reroute_url->ptr);
		ulfius_add_header_to_response(response, "Location", location_url);
		return RESPOND(307, "Your token does not exist or has already been used. You will be rerouted.", INVALID_REGISTER_TOKEN);
	}

	if(auth_update_account_verification_status(endpoint->conn->connection, owner, true)) {
		uuid_free(&owner);
		api_endpoint_safe_rollback(request, response, instance);
		return RESPOND(500, DEFAULT_500_MSG, ERROR_VERIFYING_TOKEN);
	}

	if(pgdb_transaction_commit(endpoint->conn->connection)) {
		api_endpoint_safe_rollback(request, response, instance);
		return RESPOND(500, DEFAULT_500_MSG, ERROR_TRANSACTION_COMMIT);
	}


	char location_url[instance->verification_reroute_url->length + 15];
	sprintf(location_url, "%s?verified=true", instance->verification_reroute_url->ptr);
	ulfius_add_header_to_response(response, "Location", location_url);

	return RESPOND(303, "Your email has been verified. You will be rerouted.", SUCCESS_VERIFY_REGISTRATION);
}

int api_auth_callback_sign_in(const struct _u_request * request, struct _u_response * response, void * user_data) {
	api_instance_t* instance = user_data;
	api_endpoint_t* endpoint = response->shared_data;

	string_t* email, *password;

	if(api_json_validate_text(endpoint->json_body, "email", 5, 25, &email))
		return RESPOND(422, "Please supply an email.", VALIDATION_INVALID_EMAIL);

	if(api_json_validate_text(endpoint->json_body, "password", 8, 64, &password)) {
		string_free(&email);
		return RESPOND(422, "Please supply a password.", VALIDATION_INVALID_PASSWORD);
	}

	if(auth_sign_in(endpoint->conn->connection, email, password, &endpoint->account)) {
		string_free(&email);
		string_free(&password);
		return RESPOND(401, "There is no account matching your username and password combination.", INVALID_USER_CREDENTIALS);
	}

	string_free(&email);
	string_free(&password);

	// Force new cookie to be added to response
	endpoint->refresh_cookie = true;
	endpoint->authenticated = true;

	return RESPOND(200, DEFAULT_200_MSG, SUCCESS_CREDENTIALS_SIGN_IN);
}

int api_auth_callback_send_password_reset(const struct _u_request * request, struct _u_response * response, void * user_data) {
	api_instance_t* instance = user_data;
	api_endpoint_t* endpoint = response->shared_data;

	string_t* email;

	if(api_json_validate_email(endpoint->json_body, "email", &email))
		return RESPOND(422, "Please supply a correct email.", VALIDATION_INVALID_EMAIL);

	/* Checks if authenticated user email is same as requested one */
	if(endpoint->authenticated) {
		if(strcmp(endpoint->account->email->ptr, email->ptr) != 0) {
			string_free(&email);
			return RESPOND(403, "Not allowed to request password reset for this account.", VALIDATION_UNAUTHORIZED);
		}
	}

	if(pgdb_transaction_begin(endpoint->conn->connection)) {
		string_free(&email);
		return RESPOND(500, DEFAULT_500_MSG, ERROR_TRANSACTION_BEGIN);
	}
	
	auth_account_t* account = NULL;
	if(auth_get_account_by_email(endpoint->conn->connection, email, &account)) {
		string_free(&email);
		api_endpoint_safe_rollback(request, response, instance);
		return RESPOND(500, DEFAULT_500_MSG, ERROR_EMAIL_LOOKUP);
	}


	if(account == NULL) {
		if(send_mail_not_associated(instance->sendgrid, email, instance->no_associated_account_url))
			api_endpoint_log(request, instance, endpoint, 0, FAILED_TO_SEND_MAIL);
		api_endpoint_safe_rollback(request, response, instance);
		string_free(&email);
		return RESPOND(200, "Email has been sent.", PASSWORD_RESET_EMAIL_DOESNT_EXIST);
	} 
	string_free(&email);

	if(!account->verified) {
		api_endpoint_safe_rollback(request, response, instance);
		return RESPOND(403, "Please verify your email before requesting a password reset.", VALIDATION_EMAIL_NOT_VERIFIED);
	}

	string_t* token = NULL;
	if(auth_create_token(endpoint->conn->connection, account->uuid, PASSWORD_RESET, NULL, &token)) {
		auth_account_free(&account);
		api_endpoint_safe_rollback(request, response, instance);
		return RESPOND(500, DEFAULT_500_MSG, ERROR_CREATING_TOKEN);
	}

	if(pgdb_transaction_commit(endpoint->conn->connection)) {
		string_free(&token);
		auth_account_free(&account);
		api_endpoint_safe_rollback(request, response, instance);
		return RESPOND(500, DEFAULT_500_MSG, ERROR_TRANSACTION_COMMIT);
	}

	if(send_reset_password_mail(instance->sendgrid, account->email, instance->password_reset_url, token))
		api_endpoint_log(request, instance, endpoint, 0, FAILED_TO_SEND_MAIL);

	string_free(&token);
	auth_account_free(&account);

	return RESPOND(200, "Email has been sent.", SUCCESS);
}

int api_auth_callback_reset_password(const struct _u_request * request, struct _u_response * response, void * user_data) {

	api_instance_t* instance = user_data;
	api_endpoint_t* endpoint = response->shared_data;

	/* Read in from request */
	string_t* password;
	string_t* token;

	/* Fetched from database */ 
	uuid_t* uuid = NULL;

	/* Created after token was validated */
	string_t* password_hashed = NULL;

	if(api_json_validate_password(endpoint->json_body, "password", &password))
		return RESPOND(422, "Password either missing or not complex enough.", VALIDATION_INVALID_PASSWORD);

	if(api_json_validate_text(endpoint->json_body, "token", 5, 1000, &token)) {
		string_free(&password);
		return RESPOND(422, "Please supply the token.", VALIDATION_MISSING_RESET_TOKEN);
	}

	if(pgdb_transaction_begin(endpoint->conn->connection)) {
		string_free(&password);
		string_free(&token);
		return RESPOND(500, DEFAULT_500_MSG, ERROR_TRANSACTION_BEGIN);
	}

	if(auth_verify_token(endpoint->conn->connection, token, PASSWORD_RESET, &uuid, NULL)) {
		string_free(&password);
		string_free(&token);
		api_endpoint_safe_rollback(request, response, instance);
		return RESPOND(500, DEFAULT_500_MSG, ERROR_VERIFYING_TOKEN);
	}
	string_free(&token);

	if(uuid == NULL) {
		string_free(&password);
		uuid_free(&uuid);
		api_endpoint_safe_rollback(request, response, instance);
		return RESPOND(400, "Invalid token.", INVALID_TOKEN_TYPE);
	}

	if(auth_hash_password(password, &password_hashed)) {
		string_free(&password);	
		uuid_free(&uuid);
		api_endpoint_safe_rollback(request, response, instance);
		return RESPOND(500, DEFAULT_500_MSG, ERROR_UPDATING_PASSWORD);
	}

	string_free(&password);

	if(auth_update_password(endpoint->conn->connection, uuid, password_hashed)) {
		string_free(&password);
		uuid_free(&uuid);
		api_endpoint_safe_rollback(request, response, instance);
		return RESPOND(500, DEFAULT_500_MSG, ERROR_UPDATING_PASSWORD);
	}

	string_free(&password_hashed);
	uuid_free(&uuid);

	if(pgdb_transaction_commit(endpoint->conn->connection)) {
		api_endpoint_safe_rollback(request, response, instance);
		return RESPOND(500, DEFAULT_500_MSG, ERROR_TRANSACTION_COMMIT);
	}

	return RESPOND(200, DEFAULT_200_MSG, SUCCESS);
}

int api_auth_callback_cookie_info(const struct _u_request * request, struct _u_response * response, void * user_data) {
	api_instance_t* instance = user_data;
	api_endpoint_t* endpoint = response->shared_data;

	json_t* body = api_response_object_ok();
	json_object_set(body, "email", json_stringn(endpoint->account->email->ptr, endpoint->account->email->length));
	json_object_set(body, "verified", json_boolean(endpoint->account->verified));
	return RESPOND_JSON(200, body, SUCCESS);
}


int api_auth_callback_send_new_email_verification(const struct _u_request * request, struct _u_response * response, void * user_data) {
	api_instance_t* instance = user_data;
	api_endpoint_t* endpoint = response->shared_data;

	string_t* email = NULL;
	if(api_json_validate_email(endpoint->json_body, "email", &email))
		return RESPOND(422, "Missing valid email.", VALIDATION_INVALID_EMAIL);

	if(pgdb_transaction_begin(endpoint->conn->connection)) {
		string_free(&email);
		return RESPOND(500, DEFAULT_500_MSG, ERROR_TRANSACTION_BEGIN);
	}

	string_t* token = NULL;
	if(auth_create_token(endpoint->conn->connection, endpoint->account->uuid, CHANGE_EMAIL, email, &token)) {
		string_free(&email);
		api_endpoint_safe_rollback(request, response, instance);
		return RESPOND(500, DEFAULT_500_MSG, ERROR_CREATING_TOKEN);
	}


	if(send_change_email_verification(instance->sendgrid, email, instance->change_email_url, token)) {
		string_free(&email);
		string_free(&token);

		api_endpoint_safe_rollback(request, response, instance);
		return RESPOND(500, DEFAULT_500_MSG, ERROR_TRANSACTION_COMMIT);

	}

	string_free(&email);
	string_free(&token);
	if(pgdb_transaction_commit(endpoint->conn->connection)) {
		api_endpoint_safe_rollback(request, response, instance);
		return RESPOND(500, DEFAULT_500_MSG, ERROR_TRANSACTION_COMMIT);
	}

	return RESPOND(200, DEFAULT_200_MSG, SUCCESS);
}

int api_auth_callback_verify_new_email(const struct _u_request * request, struct _u_response * response, void * user_data) {

	api_instance_t* instance = user_data;
	api_endpoint_t* endpoint = response->shared_data;

	if(!u_map_has_key(request->map_url, "t") || u_map_get_length(request->map_url, "t") <= 1) 
		return RESPOND(400, "Missing parameter for token.", VALIDATION_MISSING_PARAMETER);
	
	string_t* token = string_from_literal(u_map_get(request->map_url, "t"));

	if(pgdb_transaction_begin(endpoint->conn->connection)) {
		string_free(&token);
		return RESPOND(500, DEFAULT_500_MSG, ERROR_TRANSACTION_BEGIN);
	}

	uuid_t* owner = NULL;
	string_t* new_email = NULL;
	if(auth_verify_token(endpoint->conn->connection, token, CHANGE_EMAIL, &owner, &new_email)) {
		string_free(&token);
		api_endpoint_safe_rollback(request, response, instance);
		return RESPOND(500, DEFAULT_500_MSG, ERROR_VERIFYING_TOKEN);
	}

	string_free(&token);

	if(owner == NULL) {
		api_endpoint_safe_rollback(request, response, instance);
		return RESPOND(400, "Invalid token.", INVALID_TOKEN_TYPE);
	}

	if(auth_update_account_email(endpoint->conn->connection, owner, new_email)) {
		uuid_free(&owner);
		string_free(&new_email);
		api_endpoint_safe_rollback(request, response, instance);
		return RESPOND(500, DEFAULT_500_MSG, ERROR_UPDATING_ACCOUNT_EMAIL);
	}

	uuid_free(&owner);
	string_free(&new_email);
	if(pgdb_transaction_commit(endpoint->conn->connection)) {
		api_endpoint_safe_rollback(request, response, instance);
		return RESPOND(500, DEFAULT_500_MSG, ERROR_VERIFYING_TOKEN);
	}

	return RESPOND(200, DEFAULT_200_MSG, SUCCESS);
}

int api_auth_callback_upload_file(const struct _u_request * request, struct _u_response * response, void * user_data) {
	api_instance_t* instance = user_data;
	api_endpoint_t* endpoint = response->shared_data;

	// This should only be required for debugging
	if(endpoint->file_upload == NULL) {
		ERROR("Before calling api_auth_callback_upload_file make sure endpoint->file_upload has been initialised with relative path\n");
		api_endpoint_safe_rollback(request, response, instance);
		return RESPOND(500, DEFAULT_500_MSG, FILE_UPLOAD_IS_NULL);
	}

	int64_t content_length;
	string_t* content_type_str = NULL;
	if(api_map_get_int64(request->map_header, "content-length", &content_length) ||
	   api_map_get_string(request->map_header, "content-type", &content_type_str)) {
		// content_type must not be freed here because it wont have been
		// set
		api_endpoint_safe_rollback(request, response, instance);
		return RESPOND(400, "Missing header", VALIDATION_MISSING_PARAMETER); 
	}

	file_type_t file_type = file_type_from_str(content_type_str->ptr);
	string_free(&content_type_str);

	if((file_type & endpoint->file_upload->allowed_files) == 0) {
		api_endpoint_safe_rollback(request, response, instance);
		return RESPOND(400, "File type is not allowed.", VALIDATION_FILE_UPLOAD_FILE_TYPE_NOT_ALLOWED);
	}

	/** If this is true, the image file is probably true because
	 * binary_body_length has been reduced to max size allowed */
	if(request->binary_body_length != content_length || content_length == 0) {
		api_endpoint_safe_rollback(request, response, instance);
		return RESPOND(400, "Your file is either corupt, too large or not given.", VALIDATION_FILE_UPLOAD_INVALID_CONTENT_LENGTH); 
	}

	auth_file_t* file = calloc(1, sizeof(auth_file_t));
	file->path = string_cat(instance->root_files_folder, endpoint->file_upload->relative_path);
	file->name = string_from_literal("filename");
	file->size = request->binary_body_length;
	file->owner = uuid_copy(endpoint->account->uuid);
	file->type = file_type;
	file->uploaded = time(NULL);

	/** Open file and write to it, if it fails, transaction will be
	 * rollbacked */
	FILE* fout = fopen(file->path->ptr, "w");
	errno = 0;
	if(fout == NULL || fwrite(request->binary_body, sizeof(unsigned char), file->size, fout) != file->size) {
		ERROR("Failed to write file. %s\n", strerror(errno));
		fclose(fout);
		auth_file_free(&file);
		api_endpoint_safe_rollback(request, response, instance);
		return RESPOND(500, DEFAULT_500_MSG, ERROR_SAVING_FILE);
	}	
	fclose(fout);

	/** If fails, it is important to remove file which was uploaded before
	 * */
	if(auth_save_file(endpoint->conn->connection, file)) {
		remove(file->path->ptr);	
		auth_file_free(&file);
		api_endpoint_safe_rollback(request, response, instance);
		return RESPOND(500, DEFAULT_500_MSG, ERROR_SAVING_FILE);
	}

	return U_CALLBACK_CONTINUE;
}
