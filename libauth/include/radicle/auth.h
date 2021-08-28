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
 *
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
typedef enum auth_errors {
	AUTH_OK = 0,
	AUTH_ERROR = 1,
	AUTH_EMAIL_NOT_FOUND,
	AUTH_INVALID_PASSWORD,
	AUTH_ACCOUNT_NOT_VERIFIED,
	AUTH_ACCOUNT_NOT_ACTIVE,
	AUTH_INVALID_COOKIE,
	AUTH_COOKIE_NOT_FOUND,
	AUTH_INVALID_SIGNATURE
} auth_errors_t;

/**
 * @brief Creates a owned session and saves it to the database.
 *
 * @param conn Connection to database.
 * @param uuid Uuid of owner account.
 * @param signature_key Key used for signing cookie.
 * @param cookie Pointer to cookie whihc will be created by this function.
 * @param id Identifier of session row.
 *
 * @returns Returns either \ref auth_errors_t.AUTH_OK or auth_errors_t.AUTH_ERROR.
 */
auth_errors_t auth_make_owned_session(PGconn* conn, const uuid_t* uuid, const string_t* signature_key, auth_cookie_t** cookie, uint32_t* id);

/**
 * @brief Creates a session which is not associated to any account.
 *
 * @param conn Connection to database.
 * @param signature_key Key used for signing cookie.
 * @param cookie Pointer to cookie whihc will be created by this function.
 * @param id Identifier of session row.
 *
 * @returns Returns either \ref auth_errors_t.AUTH_OK or auth_errors_t.AUTH_ERROR.
 */
auth_errors_t auth_make_free_session(PGconn* conn, const string_t* signature_key, auth_cookie_t** cookie, uint32_t* id);

/**
 * @brief Logs a session access with its corresponding status.
 *
 * @param conn Connedtion to database
 * @param session_id Identifier of owning session.
 * @param request_log Log containing all information about request.
 *
 * @returns Returns either \ref auth_errors_t.AUTH_OK or auth_errors_t.AUTH_ERROR.
 */
auth_errors_t auth_log_access(PGconn* conn, const uint32_t session_id, const auth_request_log_t* request_log);

/**
 * @brief Registers account and creates a session.
 *
 * @param conn connection to database.
 * @param account Account which will be created. 
 *
 * @returns \ref auth_account_t.uuid will be set by function.
 * @returns Returns either \ref auth_errors_t.AUTH_OK or auth_errors_t.AUTH_ERROR.
 *
 * @pre \ref account_param_t.email, \ref account_param_t.password, \ref account_param_t.role, \ref account_param_t.verified must be set.
 */
auth_errors_t auth_register(PGconn* conn, auth_account_t* account);


/**
 * @brief Creates and saves token.
 *
 * @param conn Connection to database.
 * @param owner  Owner of token.
 * @param type Type of token
 * @param token Token which will be created and is ready to be send via email.
 *
 * @return Returns either \ref auth_errors_t.AUTH_OK or auth_errors_t.AUTH_ERROR
 */
auth_errors_t auth_create_token(PGconn* conn, const uuid_t* owner, token_type_t type, string_t** token);

/**
 * @brief Tries to sign in using the given email and password combination. 
 * Returns the complete user account on success.
 *
 * @param conn Connection to database.
 * @param email Email of account to lookup.
 * @param password Unencoded password of account.
 * @param requester Network requester. Used for filtering spam and api misuse.
 * @param signature_key Key which will be used for signing the session cookie.
 * @param account Pointer to \ref auth_account_t which will be created on success. 
 * @param cookie Pointer to \ref auth_cookie_t which will be created.
 *
 * @returns Returns either \ref auth_errors_t.AUTH_OK or \ref auth_errors_t.AUTH_ERROR.
 * @returns Returns one of
 * \ref auth_errors_t.AUTH_OK, \ref auth_errors_t.AUTH_ERROR, \ref auth_errors_t.AUTH_EMAIL_NOT_FOUND,
 * \ref auth_errors_t.AUTH_ACCOUNT_NOT_VERIFIED, \ref auth_errors_t.AUTH_ACCOUNT_NOT_ACTIVE,
 * \ref auth_errors_t.AUTH_INVALID_PASSWORD
 */
auth_errors_t auth_sign_in(PGconn* conn, const string_t* email, const string_t* password, auth_account_t** account);

/**
 * @brief Checks if received cookie has a valid signature and exists in database.
 * If it exists, it also checks for expiration or if it has been manually revoked.
 *
 * @param conn Connection to database.
 * @param cookie Raw textual cookie representation.
 * @param session Session which correlates to the given token in cookie. 
 * @param account Pointer to account which will be set if cookie is valid and has a owner. Can be null if session is not owned.
 *
 * @returns Returns one of \ref auth_errors_t.AUTH_OK, \ref auth_errors_t.AUTH_ERROR,
 * \ref auth_errors_t.AUTH_INVALID_COOKIE, \ref auth_errors_t.AUTH_COOKIE_NOT_FOUND,
 * \ref auth_errors_t.AUTH_INVALID_SIGNATURE, \ref auth_errors_t.AUTH_ACCOUNT_NOT_VERIFIED, 
 * \ref auth_errors_t.AUTH_ACCOUNT_NOT_ACTIVE
 */
auth_errors_t auth_verify_cookie(PGconn* conn, const string_t* signature_key, const string_t* cookie, uint32_t* session_id, auth_account_t** account);

#if defined(__cplusplus)
}
#endif

#endif // RADICLE_AUTH_INCLUDE_RADICLE_AUTH_H 

/** @} */
