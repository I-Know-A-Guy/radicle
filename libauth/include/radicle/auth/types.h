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

typedef enum auth_account_role {
	ROLE_NONE = 0,
	ROLE_ADMIN,
	ROLE_USER
} auth_account_role_t;

int auth_account_role_from_str(const char* role);
const char* auth_account_role_to_str(auth_account_role_t role);

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
	auth_account_role_t role;
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
auth_account_t* auth_account_new(uuid_t* uuid, string_t* email, string_t* password, auth_account_role_t role, bool active, bool verified, time_t created);

/**
 * @brief Frees all associated data of account_t and sets pointer to NULL.
 *
 * @param account Pointer to account to be freed.
 *
 * @returns Returns void
 */
void auth_account_free(auth_account_t** account);

/**
 * @brief Log of a request. Usable for monitoring api.
 *
 * @see auth_request_log_new()
 * @see auth_request_log_free()
 */
typedef struct auth_request_log {
	string_t* ip; /**< Ip of requester. */
	unsigned int port; /**< Port of requester. */
	time_t date; /**< Time of request. */
       	string_t* url; /**< URL which was requested. */
	struct timespec timer; /**< Timer used for measuring response time. */
	int response_time; /**< Time in milliseconds */
	int response_code; /**< Code returned to requester. */
	int internal_status; /**< Internal status code of request. */
} auth_request_log_t;

/**
 * @brief Creates a new auth_requester_t struct and copies string values.
 *
 * @returns Returns pointer to new \ref auth_requester_t.
 */
auth_request_log_t* auth_request_log_new();

/**
 * @brief Calculates time since \ref auth_reqeust_log_new was called. Time is given in microseconds.
 *
 * @param request_log Log to calculate time for.
 */
void auth_request_log_calculate_response_time(auth_request_log_t* request_log);

/**
 * @brief Frees struct and points pointer to NULL.
 *
 * @param requester Requester object to be freed.
 *
 * @returns Returns void.
 */
void auth_request_log_free(auth_request_log_t** requester);

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

typedef enum token_type {
	NONE=0,
	REGISTRATION, /**< Token is used for verifying registration. */
	PASSWORD_RESET, /**< Token is used for granting password reset rights. */
	CHANGE_EMAIL
} token_type_t;

int token_type_from_str(const char* type);
const char* token_type_to_str(token_type_t type);

typedef struct auth_session_access_entry {
	uuid_t* owner;
	uint32_t internal_status;
	uint32_t response_code;
} auth_session_access_entry_t;

void auth_session_access_entry_free(void* ptr);

typedef enum file_type {
	UNKNOWN=0,
	IMAGE_JPEG,
	IMAGE_PNG
} file_type_t;

int file_type_from_str(const char* type);
const char* file_type_to_str(file_type_t type);

typedef struct auth_file {
	uuid_t* uuid;
	uuid_t* owner;
	file_type_t type;
	string_t* path;
	string_t* name;
	uint64_t size;
	time_t uploaded;
} auth_file_t;

/**
 * @brief Frees a auth_file_t and all its associated data.
 */
void auth_file_free(auth_file_t** file);

#if defined(__cplusplus)
}
#endif

#endif // RADICLE_AUTH_INCLUDE_RADICLE_AUTH_TYPES_H 

/** @} */
