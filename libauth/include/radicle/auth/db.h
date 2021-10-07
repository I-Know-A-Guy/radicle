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
 * @brief Contains all functions for database storage.
 * @author Nils Egger
 * @addtogroup Auth
 * @{
 */

#ifndef RADICLE_AUTH_INCLUDE_RADICLE_AUTH_DB_H 
#define RADICLE_AUTH_INCLUDE_RADICLE_AUTH_DB_H 

#include <stdint.h>

#include <libpq-fe.h>

#include "radicle/types/string.h"
#include "radicle/types/uuid.h"
#include "radicle/auth/types.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
* @brief Sessions expire one month after being created or manually revoked.
*/
#define SESSION_EXPIRE 2629746

/**
 * @brief Inserts account into Accounts. Sets verified to false.
 *
 * @param conn Connection to database.
 * @param account Account params which will be used for creating account. Only \ref auth_account_t.email, \ref auth_account_t.password, \ref auth_account_t.role and \ref auth_account_t.verified are regarded.
 * @param uuid Random v4 uuid created by postgres for account.
 *
 * returns Returns 0 on success.
 */
int auth_save_account(PGconn* conn, const auth_account_t* account, uuid_t** uuid);

/**
 * @brief Updates password of account. Must already be hashed.
 *
 * @param conn Connection to database.
 * @param uuid UUID of account which will be updated.
 * @param password Hashed password which will be inserted into database.
 *
 * @return Returns 0 on success
 */
int auth_update_account_password(PGconn* conn, const uuid_t* uuid, const string_t* password);

/**
 * @brief Inserts token into database
 *
 * @param conn Connection to database.
 * @param owner Owner of token
 * @param token Random token which will be sent via email.
 * @param custom TOken Specific extra data which needs to be stored
 *
 * @returns Returns 0 on success.
 */
int auth_save_token(PGconn* conn, const uuid_t* owner, const string_t* token, token_type_t type, const string_t* custom);

/**
 * @brief Saves session to database and returns its row id.
 *
 * @param conn Connection to database.
 * @param owner Owner uuid of session. Can be null.
 * @param token Generated session id which will be included in cookie.
 * @param salt Salt used to sign token. (Session id included in cookie.)
 * @param id Returned row id of session.
 *
 * @returns Returns 0 on success.
 */
int auth_save_session(PGconn* conn, const uuid_t* owner, const string_t* token, const time_t expires, const string_t* salt, uint32_t* id);

/**
 * @brief Saves session access to database.
 *
 * @param conn Connection to database.
 * @param session_id Row id of session.
 * @param requester Network requester.
 * @param status Status to remember access by. Example: invalid_login
 *
 * @returns Returns 0 on success.
 */
int auth_save_session_access(PGconn* conn, const uint32_t session_id, const auth_request_log_t* request_log);

/**
 * @brief Deletes all registration tokens linked to owner.
 *
 * @param conn Connection to database.
 * @param owner Owner of registration tokens
 * @param type Type of token to be removed
 *
 * @return Returns 0 on success.
 */
int auth_remove_token_by_owner(PGconn* conn, const uuid_t* owner, token_type_t type);

/**
 * @brief If token exists and is valid, owner param will be set. Furthermore,
 * entry in database will be removed.
 *
 * @param conn Connection to database.
 * @param token Verification token.
 * @param owner Owner which will be set.
 * @param custom token specific data field
 *
 * @todo dont return token type but instead add it to where clause in sql
 * statement
 *
 * @return Returns 0 on success. Owner only contains a value if token was valid.
 */
int auth_verify_token(PGconn* conn, const string_t* token, token_type_t expected_type, uuid_t** owner, string_t** custom);

/**
 * @brief Method to update account verification. Usually used after @ref
 * auth_verify_and_remove_registration_token.
 *
 * @param conn Connection to database.
 * @param account Account uuid to be updated.
 * @param verified Value to be set in verified row.
 *
 * @return Returns 0 on success.
 */
int auth_update_account_verification_status(PGconn* conn, const uuid_t* account, bool verified);

/**
 * @brief Retrieves account matching email, if none is found, \p account stays NULL.
 *
 * @param conn Connection to database.
 * @param email Email of account to find.
 * @param account Double pointer to account. Will have to be freed after use.
 *
 * @returns Returns 0 on success.
 */
int auth_get_account_by_email(PGconn* conn, const string_t* email, auth_account_t** account);

/**
 * @brief Looks up cookie in database, verifies expiration or revoked and returns id, salt and account data. 
 *
 * @param conn Connection to database.
 * @param cookie Token of cookie, which identifies it in database.
 * @param id ID of queried session.
 * @param salt Salt used for signature.
 * @param account Account to be set on valid cookie verification.
 * 
 * @returns Returns 0 on success.
 */
int auth_get_session_by_cookie(PGconn* conn, const string_t* cookie, uint32_t* id, string_t** salt, auth_account_t** account);

#if defined(__cplusplus)
}
#endif

#endif // RADICLE_AUTH_INCLUDE_RADICLE_AUTH_DB_H 

/** @} */
