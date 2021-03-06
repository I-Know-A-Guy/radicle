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

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "radicle/print.h"
#include "radicle/pgdb.h"
#include "radicle/auth/crypto.h"
#include "radicle/auth/db.h"
#include "radicle/auth.h"

auth_errors_t auth_make_owned_session(PGconn* conn, const uuid_t* uuid, const string_t* signature_key, auth_cookie_t** cookie, uint32_t* id) {
	if(auth_generate_session_cookie(signature_key, cookie)) {
		DEBUG("Failed to  generate cookie for session.\n");
		return AUTH_ERROR;
	}

	if(auth_save_session(conn, uuid, (*cookie)->token, time(NULL) + SESSION_EXPIRE, (*cookie)->salt, id)) {
		DEBUG("Failed to save session.\n");
		auth_cookie_free(cookie);
		return AUTH_ERROR;
	}
	return AUTH_OK;
}

auth_errors_t auth_make_free_session(PGconn* conn, const string_t* signature_key, auth_cookie_t** cookie, uint32_t* id) {
	return auth_make_owned_session(conn, NULL, signature_key, cookie, id);
}

auth_errors_t auth_log_access(PGconn* conn, const uint32_t session_id, const auth_request_log_t* request_log) {
	if(auth_save_session_access(conn, session_id, request_log)) {
		return AUTH_ERROR;
	}
	return AUTH_OK;
}

auth_errors_t auth_register(PGconn* conn, auth_account_t* account) {

	string_t* password_hash = NULL;

	if(auth_hash_password(account->password, &password_hash)) {
		DEBUG("Failed to hash password.\n");
		return AUTH_ERROR;
	}

	string_free(&account->password);
	account->password = password_hash;

	if(auth_save_account(conn, account, &account->uuid)) {
		DEBUG("Failed to save account.\n");
		return AUTH_ERROR;
	}

	return AUTH_OK;
}

auth_errors_t auth_update_password(PGconn* conn, const uuid_t* uuid, const string_t* password) {
	string_t* password_hash = NULL;
	if(auth_hash_password(password, &password_hash)) {
		DEBUG("Failed to hash password.\n");
		return AUTH_ERROR;
	}
	
	if(auth_update_account_password(conn, uuid, password)) {
		DEBUG("Failed to update password of account.");
		return AUTH_ERROR;
	}

	string_free(&password_hash);

	return AUTH_OK;
}

auth_errors_t auth_create_token(PGconn* conn, const uuid_t* owner, const token_type_t type, const string_t* custom, string_t** token) {
	if(auth_generate_random_base64_url_safe(256, token)) 
		return AUTH_ERROR;

	if(auth_save_token(conn, owner, *token, type, custom)) {
		string_free(token);
		return AUTH_ERROR;
	}

	return AUTH_OK;
}

auth_errors_t auth_sign_in(PGconn* conn, const string_t* email, const string_t* password, auth_account_t** account) {
		
	if(auth_get_account_by_email(conn, email, account)) {
		DEBUG("Failed to retrieve account.\n");
		return AUTH_ERROR;
	}

	// There is no account linked to the email
	// Do a fake hash to mislead email finder? what are they called?
	if(*account == NULL) {
		return AUTH_EMAIL_NOT_FOUND;
	}

	if(auth_verify_password((*account)->password, password)) {
		auth_account_free(account);
		return AUTH_INVALID_PASSWORD;
	}

	if(!(*account)->active) {
		auth_account_free(account);
		return AUTH_ACCOUNT_NOT_ACTIVE;
	}

	return AUTH_OK;
}

auth_errors_t auth_verify_cookie(PGconn* conn, const string_t* signature_key, const string_t* cookie, uint32_t* session_id, auth_account_t** account) {
	
	auth_cookie_t* cookie_result;
	if(auth_split_cookie(cookie, &cookie_result)) {
		return AUTH_INVALID_COOKIE;
	}

	string_t* session_salt = NULL;
	if(auth_get_session_by_cookie(conn, cookie_result->token, session_id, &session_salt, account)) {
		auth_cookie_free(&cookie_result);
		return AUTH_ERROR;
	}

	if(*session_id == 0) {
		auth_cookie_free(&cookie_result);
		return AUTH_COOKIE_NOT_FOUND;
	}

	if(hmac_verify_salted(signature_key, session_salt, cookie_result->signature, cookie_result->token)) {
		DEBUG("%s %s %s %s\n", signature_key->ptr, session_salt->ptr, cookie_result->signature->ptr, cookie_result->token->ptr);
		*session_id = 0;
		string_free(&session_salt);
		auth_cookie_free(&cookie_result);
		auth_account_free(account);
		return AUTH_INVALID_SIGNATURE;
	}
	string_free(&session_salt);
	auth_cookie_free(&cookie_result);

	if(*account == NULL) {
		return AUTH_OK;
	}

	if((*account)->active == false) {
		auth_account_free(account);
		return AUTH_ACCOUNT_NOT_ACTIVE;
	}

	return AUTH_OK;
}
