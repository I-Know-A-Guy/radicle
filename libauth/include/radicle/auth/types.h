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
 * @brief Contains \ref auth_account_t, \ref auth_requester_t and \ref auth_cookie_t 
 * @author Nils Egger
 * @addtogroup Auth
 * @{
 */

#ifndef RADICLE_AUTH_INCLUDE_RADICLE_AUTH_TYPES_H 
#define RADICLE_AUTH_INCLUDE_RADICLE_AUTH_TYPES_H 

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "radicle/types/string.h"
#include "radicle/types/uuid.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Struct which represents a row in the Account table.
 *
 * @see auth_account_new()
 * @see auth_account_free()
 */
typedef struct auth_account {
	uuid_t* uuid; /**< Identifier of account. */
	string_t* email;
	string_t* password;
	string_t* role;
	bool verified, active;
	time_t created;
} auth_account_t;

/**
 * @brief Creates a new auth_account_t object and copies string values if they are not NULL.
 *
 * @param uuid UUID of new account, pass NULL if not yet set.
 * @param email Email of account.
 * @param password Password of account.
 * @param role Role of account.
 * @param active Whetever account is allowed to be used.
 * @param verified If true, email can be seen as verified.
 * @param created Datetime account was created.
 *
 * @returns Returns a new pointer to \ref auth_account_t
 */
auth_account_t* auth_account_new(uuid_t* uuid, string_t* email, string_t* password, string_t* role, bool active, bool verified, time_t created);

/**
 * @brief Frees all associated data of account_t and sets pointer to NULL.
 *
 * @param account Pointer to account to be freed.
 *
 * @returns Returns void
 */
void auth_account_free(auth_account_t** account);

/**
 * @brief Network Requester representation. Used for spam and misuse filtering.
 * @todo Create type for ip 'INET' which is compatible with postgres.
 *
 * @see auth_requester_new()
 * @see auth_requester_free()
 */
typedef struct auth_requester {
	string_t* ip; /**< Ip of requester. */
       	string_t* path; /**< URL which was requested. */
} auth_requester_t;

/**
 * @brief Creates a new auth_requester_t struct and copies string values.
 *
 * @param ip IP which was used by requester.
 * @param path URL which was requested.
 *
 * @returns Returns pointer to new \ref auth_requester_t.
 */
auth_requester_t* auth_requester_new(const string_t* ip, const string_t* path);

/**
 * @brief Creates a new auth_requester_t struct and copies literal string values.
 *
 * @param ip IP which was used by requester.
 * @param path URL which was requested.
 *
 * @returns Returns pointer to new \ref auth_requester_t.
 */

auth_requester_t* auth_requester_newl(const char* ip, const char* path);

/**
 * @brief Frees struct and points pointer to NULL.
 *
 * @param requester Requester object to be freed.
 *
 * @returns Returns void.
 */
void auth_requester_free(auth_requester_t** requester);

/**
 * Representation of a session cookie.
 *
 * @see auth_cookie_new_empty()
 * @see auth_cookie_free()
 */
typedef struct auth_cookie {
	string_t* cookie; /**< Combined token and signature seperated by '-'. */
	string_t* token; /**< Session identifier. */
	string_t* signature; /**< Signature hash of token. */
	string_t* salt; /**< Salt used for signature. */
} auth_cookie_t;


/**
 * @brief Creates a new empty auth cookie.
 *
 * @returns Returns pointer to new \ref auth_cookie_t.
 */
auth_cookie_t* auth_cookie_new_empty();

/**
 * @brief Frees all associated data and sets pointer to NULL.
 *
 * @param cookie Pointer to cookie to be freed.
 *
 * @returns Returns void.
 */
void auth_cookie_free(auth_cookie_t** cookie);

typedef struct auth_session {
	uint32_t id;
	string_t* salt;
} auth_session_t;

/**
 * @brief Creates new auth_session and initializes to NULL.
 *
 * @returns Returns new allocated \ref auth_session_t object.
 */
auth_session_t* auth_session_new();

/**
 * @brief Frees an allocated \ref auth_session_t and sets its pointer to NULL.
 *
 * @param session Pointer to session.
 *
 * @returns Returns Void.
 */
void auth_session_free(auth_session_t** session);

#if defined(__cplusplus)
}
#endif

#endif // RADICLE_AUTH_INCLUDE_RADICLE_AUTH_TYPES_H 

/** @} */
