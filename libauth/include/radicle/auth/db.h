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
 * @brief Inserts registartion with token which will be needed to verify.
 *
 * @param conn Connection to database.
 * @param uuid UUID of account to be verified.
 * @param token Random token which will be sent via email.
 *
 * @returns Returns 0 on success.
 */
int auth_save_registration(PGconn* conn, const uuid_t* uuid, const string_t* token);

/**
 * @brief Saves session to database and returns its row id.
 *
 * @param conn Connection to database.
 * @param owner Owner uuid of session.
 * @param token Generated session id which will be included in cookie.
 * @param salt Salt used to sign token. (Session id included in cookie.)
 * @param id Returned row id of session.
 *
 * @returns Returns 0 on success.
 */
int auth_save_session(PGconn* conn, const uuid_t* owner, const string_t* token, const string_t* salt, uint32_t* id);

/**
 * @brief Saves session access to database.
 *
 * @param conn Connection to database.
 * @param session_id Row id of session.
 * @param requester Network requester.
 *
 * @returns Returns 0 on success.
 */
int auth_save_session_access(PGconn* conn, const uint32_t session_id, const auth_requester_t* requester);

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

#if defined(__cplusplus)
}
#endif

#endif // RADICLE_AUTH_INCLUDE_RADICLE_AUTH_DB_H 

/** @} */
