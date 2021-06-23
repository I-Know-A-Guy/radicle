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
 * @brief Contains functions for register, sigining in and logging out a user. 
 * @author Nils Egger
 * @addtogroup Auth
 * @{
 */

#ifndef RADICLE_AUTH_INCLUDE_RADICLE_AUTH_H 
#define RADICLE_AUTH_INCLUDE_RADICLE_AUTH_H 

#include <libpq-fe.h>

#include "radicle/auth/types.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Error types used by these high level functions.
 */
enum {
	AUTH_OK = 0,
	AUTH_ERROR = 1,
	AUTH_EMAIL_NOT_FOUND,
	AUTH_ACCOUNT_NOT_VERIFIED,
	AUTH_ACCOUNT_NOT_ACTIVE,
	AUTH_FAILED_VERIFY_PASSWORD,
	AUTH_FAILED_TO_GENERATE_COOKIE,
	AUTH_FAILED_TO_SAVE_SESSION,
	AUTH_FAILED_TO_SAVE_SESSION_ACCESS,
};

/**
 * @brief Registers account and creates a session.
 * @todo Must send a verification email.
 *
 * @param conn connection to database.
 * @param account Account which will be created. 
 * @param requester Network requester. Used for filtering spam and api misuse.
 * @param signature_key Key which will be used for signing the session cookie.
 * @param cookie Pointer to \ref auth_cookie_t which will be created.
 *
 * @returns \ref auth_account_t.uuid will be set by function.
 * @returns Returns 0 on success.
 *
 * @pre \ref account_param_t.email, \ref account_param_t.password, \ref account_param_t.role, \ref account_param_t.verified must be set.
 */
int auth_register(PGconn* conn, auth_account_t* account, auth_requester_t* requester, const string_t* signature_key, auth_cookie_t** cookie);

/**
 * @brief Tries to sign in using the given email and password combination. 
 * Returns the complete user account on success.
 * @todo Does requester really have to be here? wont it be in a seperate place?
 *
 * @param conn Connection to database.
 * @param email Email of account to lookup.
 * @param password Unencoded password of account.
 * @param requester Network requester. Used for filtering spam and api misuse.
 * @param signature_key Key which will be used for signing the session cookie.
 * @param account Pointer to \ref auth_account_t which will be created on success. 
 * @param cookie Pointer to \ref auth_cookie_t which will be created.
 *
 * @returns Returns one of
 * AUTH_OK, AUTH_ERROR, AUTH_EMAIL_NOT_FOUND,
 * AUTH_ACCOUNT_NOT_VERIFIED, AUTH_ACCOUNT_NOT_ACTIVE,
 * AUTH_FAILED_VERIFY_PASSWORD, AUTH_FAILED_TO_GENERATE_COOKIE,
 * AUTH_FAILED_TO_SAVE_SESSION, AUTH_FAILED_TO_SAVE_SESSION_ACCESS
 */
int auth_sign_in(PGconn* conn, const string_t* email, const string_t* password,
	       	const auth_requester_t* requester, const string_t* signature_key,
		auth_account_t** account, auth_cookie_t** cookie);


/**
 * @brief Checks if received cookie has a valid signature and exists in database.
 * If it exists, it also checks for expiration or if it has been manually revoked.
 * @todo write test
 *
 * @param conn Connection to database.
 * @param cookie Raw textual cookie representation.
 * @param requester Auth network requester. Used for anti spam measures.
 * @param account Pointer to account which will be set if cookie is valid.
 *
 * @returns Returns 0 on success.
 */
int auth_verify_cookie(PGconn* conn, const string_t* signature_key, const string_t* cookie, const auth_requester_t* requester, auth_account_t** account);

#if defined(__cplusplus)
}
#endif

#endif // RADICLE_AUTH_INCLUDE_RADICLE_AUTH_H 

/** @} */
