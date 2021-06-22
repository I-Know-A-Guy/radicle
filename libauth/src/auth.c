#include <stdlib.h>
#include <string.h>

#include "radicle/print.h"
#include "radicle/pgdb.h"
#include "radicle/auth/crypto.h"
#include "radicle/auth/db.h"
#include "radicle/auth.h"

int auth_register(PGconn* conn, auth_account_t* account, auth_requester_t* requester, const string_t* signature_key, auth_cookie_t** cookie) {

	string_t* password_hash = NULL;
	uint32_t session_id = 0;

	if(auth_hash_password(account->password, &password_hash)) {
		DEBUG("Failed to hash password.\n");
		return 1;
	}
	string_free(&account->password);
	account->password = password_hash;

	if(auth_save_account(conn, account, &account->uuid)) {
		DEBUG("Failed to save account.\n");
		return 1;
	}

	if(!account->verified) {
		string_t* registration_token = NULL;

		if(auth_generate_random_base64(256, &registration_token)) {
			ERROR("Failed to generate registration token.\n");
			uuid_free(&account->uuid);
			return 1;
		}

		if(auth_save_registration(conn, account->uuid, registration_token)) {
			ERROR("Failed to save registration.\n");
			string_free(&registration_token);
			uuid_free(&account->uuid);
			return 1;
		}
		string_free(&registration_token);
	}

	if(auth_generate_session_cookie(signature_key, cookie)) {
		ERROR("Failed to  generate cookie for session.\n");
		uuid_free(&account->uuid);
		return 1;
	}

	if(auth_save_session(conn, account->uuid, (*cookie)->token, (*cookie)->salt, &session_id)) {
		ERROR("Failed to save session.\n");
		uuid_free(&account->uuid);
		auth_cookie_free(cookie);
		return 1;
	}

	if(auth_save_session_access(conn, session_id, requester)) {
		uuid_free(&account->uuid);
		auth_cookie_free(cookie);
		return 1;
	}

	return 0;
}

int auth_sign_in(PGconn* conn, const string_t* email, const string_t* password,
	       	const auth_requester_t* requester, const string_t* signature_key,
		auth_account_t** account, auth_cookie_t** cookie) {

	// TODO check blacklist

	if(auth_get_account_by_email(conn, email, account)) {
		DEBUG("Failed to retrieve account.\n");
		return AUTH_ERROR;
	}

	// There is no account linked to the email
	// Do a fake hash to mislead email finder? what are they called?
	if(account == NULL) {
		// TODO do a fake verify hash.
		return AUTH_EMAIL_NOT_FOUND;
	}

	if(!(*account)->verified) {
		auth_account_free(account);
		return AUTH_ACCOUNT_NOT_VERIFIED;
	}

	if(!(*account)->active) {
		auth_account_free(account);
		return AUTH_ACCOUNT_NOT_ACTIVE;
	}
	
	if(auth_verify_password((*account)->password, password)) {
		auth_account_free(account);
		return AUTH_FAILED_VERIFY_PASSWORD;
	}

	if(auth_generate_session_cookie(signature_key, cookie)) {
		auth_account_free(account);
		return AUTH_FAILED_TO_GENERATE_COOKIE;
	}
	
	uint32_t session_row_id;
	if(auth_save_session(conn, (*account)->uuid, (*cookie)->token, (*cookie)->salt, &session_row_id)) {
		auth_account_free(account);
		auth_cookie_free(cookie);
		return AUTH_FAILED_TO_SAVE_SESSION;
	}

	if(auth_save_session_access(conn, session_row_id, requester)) {
		auth_account_free(account);
		auth_cookie_free(cookie);
		return AUTH_FAILED_TO_SAVE_SESSION_ACCESS;
	}

	return AUTH_OK;
}
