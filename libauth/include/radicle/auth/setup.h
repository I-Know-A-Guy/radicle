/**
 * @file 
 * @brief Contains function for creating database used by this module.
 * @author Nils Egger
 * @addtogroup Auth
 * @{
 */

#ifndef RADICLE_AUTH_INCLUDE_RADICLE_AUTH_SETUP_H 
#define RADICLE_AUTH_INCLUDE_RADICLE_AUTH_SETUP_H 

#include <libpq-fe.h>

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Helper function used to setup database for testing or what not.
 *
 * @param conn Connection to database.
 *
 * @returns 0 on success.
 */
int auth_create_db_tables(PGconn* conn);

#if defined(__cplusplus)
}
#endif

#endif // RADICLE_AUTH_INCLUDE_RADICLE_AUTH_SETUP_H 

/** @} */
