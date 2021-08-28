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
 * @brief Header file containing all functions related to low level database manipulation.
 * @author Nils Egger
 * @addtogroup pgdb
 * @{
 */
#ifndef RADICLE_PGDB_INCLUDE_RADICLE_PGDB_H
#define RADICLE_PGDB_INCLUDE_RADICLE_PGDB_H
#include <stdbool.h>
#include <time.h>

#include <libpq-fe.h>
#include <arpa/inet.h>

#include "radicle/types/string.h"
#include "radicle/types/uuid.h"
#include "radicle/print.h"

#if defined(__cplusplus)
extern "C" {
#endif

/* https://stackoverflow.com/a/28592202 */
#define htonll(x) ((1==htonl(1)) ? (x) : ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
#define ntohll(x) ((1==ntohl(1)) ? (x) : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))

/**
 * @brief Represents SQL parameters which will be bound to a query.
 *
 * @see pgdb_params_new()
 * @see pgdb_params_free()
 * @see pgdb_bind_int32()
 * @see pgdb_bind_text()
 */
typedef struct pgdb_params {
	int count; /**< Amount of parameters to bind to query. */
	Oid* types; /**< Type of parameters, if an index is set to NULL, postgres will try to guess the type. */
	char** values; /**< Actaul values to send. */
	int* lengths; /**< Length of values. */
	int* formats; /**< Format of values, either 0 for text or 1 for binary. */
	int next_index; /**< When binding values, this will increase by one and specify next index of next param. */
} pgdb_params_t;

/**
 * @brief Creates a new \ref pgdb_params_t object and initialzes all array to the size of \p count.
 *
 * @param count Amount of parameters.
 *
 * @returns Returns a new pointer to \ref pgdb_params_t.
 */
pgdb_params_t* pgdb_params_new(const int count);

/**
 * @brief Frees all associated data to \p ptr and sets it to NULL.
 *
 * @returns Returns void.
 */
void pgdb_params_free(pgdb_params_t** ptr);

/**
 * @brief Representation of a SQL query result. Containing the actual \ref PGresult and \ref ExecStatusType.
 *
 * @see pgdb_result_new()
 * @see pgdb_result_free()
 */
typedef struct pgdb_result {
	PGresult* pg; /**< Actual result returned by libpq. */
	ExecStatusType status; /**< Status returned by query. */
} pgdb_result_t;

/**
 * @brief Creates a new \ref pgdb_result_t and returns its pointer.
 *
 * @returns Returns pointer to \ref pgdb_result_t.
 */
pgdb_result_t* pgdb_result_new(PGresult* result);

/** 
 * Frees \ref pgdb_result_t.pg and the \p result itself. Sets the pointer to NULL.
 *
 * @param result Pointer to result pointer.
 *
 * @returns Returns void.
 *
 */
void pgdb_result_free(pgdb_result_t** result);

/**
 * @brief Connects to database using connection info string, then checks status of conneciton, \
 * if connection is bad, frees memory again.
 *
 * @param conninfo String of connection info, e.g. host=localhost port=5432 dbname=ikag user=postgres password=postgres connect_timeout=10
 * @param connection Double pointer to PGconn, which will store actual successfull data. Is set to NULL on failure. 
 *
 * @returns 0 on success
 */
int pgdb_connect(const char* conninfo, PGconn** connection);

/**
 * @brief Pings the database server to make sure it is running and returns the status as text.
 *
 * @param conninfo String of connection info. e.g. host=localhost port=5432 dbname=ikag user=postgres password=postgres connect_timeout=10
 * @param status Pointer to status variable.
 *
 * @returns Returns text for one of PQPING_OK, PQPING_REJECT, PQPING_NO_RESPONSE or PQPING_NO_ATTEMPT
 *
 * @retval PQPING_OK The server is running and appears to be accepting connections.
 * @retval PQPING_REJECT The server is running but is in a state that disallows connections (startup, shutdown, or crash recovery).
 * @retval PQPING_NO_RESPONSE The server could not be contacted. This might indicate that the server is not running, or that there is something wrong with the given connection parameters (for example, wrong port number), or that there is a network connectivity problem (for example, a firewall blocking the connection request).
 * @retval PQPING_NO_ATTEMPT No attempt was made to contact the server, because the supplied parameters were obviously incorrect or there was some client-side problem (for example, out of memory).
 */
const char* pgdb_ping_info(const char* conninfo, PGPing* status);

/**
 * @brief Sends query to database. Does not expect data to be returned, if it does, an error will be returned.
 *
 * @param conn Connection to database.
 * @param stmt SQL Query statement.
 *
 * @returns Returns 0 for success.
 */
int pgdb_execute(PGconn* conn, const char* stmt);

/**
 * @brief Sends query to database. Does not expect data to be returned, if it does, an error will be returned.
 *
 * @param conn Connection to database.
 * @param stmt SQL Query statement. Use $1, $2, .., $n for parameters.
 * @param params Parameters to be bound to stmt.
 *
 * @returns Returns 0 for success.
 */
int pgdb_execute_param(PGconn* conn, const char* stmt, const pgdb_params_t* params);

/**
 * Sends query to database. Expects data to be returned. 
 *
 * @param conn Connection to database.
 * @param stmt SQL Query statement. Use $1, $2, .., $n for parameters.
 * @param params Parameters to be bound to query.
 * @param result Pointer to result buffer. Will be created but not freed by this function.
 *
 * @returns Returns 0 for success.
 */
int pgdb_fetch_param(PGconn* conn, const char* stmt, const pgdb_params_t* params, pgdb_result_t** result);

/**
 * @brief Sends BEGIN command to database.
 *
 * @param conn Connection to database.
 *
 * @returns Returns 0 for success.
 */
int pgdb_transaction_begin(PGconn* conn);

/**
 * @brief Sends COMMIT command to database.
 *
 * @param conn Connection to database.
 *
 * @returns Returns 0 for success.
 */
int pgdb_transaction_commit(PGconn* conn);

/**
 * @brief Sends ROLLBACK command to database.
 *
 * @param conn Connection to database.
 *
 * @returns Returns 0 for success.
 */
int pgdb_transaction_rollback(PGconn* conn);

/**
 * @binds NULL to query and increases next param index.
 */
void pgdb_bind_null(pgdb_params_t* params);
/**
 * @brief Binds int32 to query.
 *
 * @param value Value to bind. 
 * @param param \ref pgdb_params_t struct to use for binding.
 *
 * @returns Returns void.
 */
void pgdb_bind_uint32(const int value, pgdb_params_t* params);

/**
 * @brief Binds timestamp to query.
 *
 * @param value Value to bind. 
 * @param param \ref pgdb_params_t struct to use for binding.
 *
 * @returns Returns void.
 */
void pgdb_bind_uint64(const uint64_t value, pgdb_params_t* params);

/**
 * @brief Binds text to query.
 *
 * @param text Pointer to text. 
 * @param param \ref pgdb_params_t struct to use for binding.
 *
 * @returns Returns void. 
 */
void pgdb_bind_text(const string_t* text, pgdb_params_t* params);

/**
 * @brief Binds text to query.
 *
 * @param text C string. 
 * @param param \ref pgdb_params_t struct to use for binding.
 *
 * @returns Returns void. 
 */
void pgdb_bind_c_str(const char* text, pgdb_params_t* params);

/**
 * @brief Binds uuid to query.
 *
 * @param uuid Pointer to uuid to bind. 
 * @param param \ref pgdb_params_t struct to use for binding.
 *
 * @returns Returns void. 
 */
void pgdb_bind_uuid(const uuid_t* text, pgdb_params_t* params);

/**
 * @brief Binds boolean to query.
 *
 * @param flag Boolean value to bind. 
 * @param index Index to use in arrays.
 * @param param \ref pgdb_params_t struct to use for binding.
 *
 * @returns Returns void. 
 */
void pgdb_bind_bool(const bool flag, pgdb_params_t* params);

/**
 * @brief Binds timestamp to query.
 *
 * @param timestamp Value of timestamp. 
 * @param index Index to use in arrays.
 * @param param \ref pgdb_params_t struct to use for binding.
 *
 * @returns Returns void. 
 */
void pgdb_bind_timestamp(const time_t timestamp, pgdb_params_t* params);

/**
 * @brief Retrieves a text value and writes it to buffer.
 *
 * @param result \ref pgdb_params_t containing query result. 
 * @param row Row index of data.
 * @param field Name of the column.
 * @param buffer String buffer to write to.
 *
 * @returns Returns 1 if value is NULL, otherwise 0 for success.
 */
int pgdb_get_text(const pgdb_result_t* result, const int row, const char* field, string_t** buffer);

/**
 * @brief Retrievs a uint32.
 *
 * @param result \ref pgdb_params_t containing query result. 
 * @param row Row index of data.
 * @param field Name of the column.
 * @param buffer Buffer to write to.
 *
 * @returns Returns 1 if value is NULL, otherwise 0 for success.
 */
int pgdb_get_uint32(const pgdb_result_t* result, const int row, const char* field, uint32_t* buffer);

/**
 * @brief Retrievs a uint64.
 *
 * @param result \ref pgdb_params_t containing query result. 
 * @param row Row index of data.
 * @param field Name of the column.
 * @param buffer Buffer to write to.
 *
 * @returns Returns 1 if value is NULL, otherwise 0 for success.
 */
int pgdb_get_uint64(const pgdb_result_t* result, const int row, const char* field, uint64_t* buffer);


/**
 * @brief Retrievs a boolean value.
 *
 * @param result \ref pgdb_params_t containing query result. 
 * @param row Row index of data.
 * @param field Name of the column.
 * @param buffer Buffer to write to.
 *
 * @returns Returns 1 if value is NULL, otherwise 0 for success.
 */
int pgdb_get_bool(const pgdb_result_t* result, const int row, const char* field, bool* buffer);

/**
 * @brief a timestamp value.
 *
 * @param result \ref pgdb_params_t containing query result. 
 * @param row Row index of data.
 * @param field Name of the column.
 * @param buffer Buffer to write to.
 *
 * @returns Returns 1 if value is NULL, otherwise 0 for success.
 */
int pgdb_get_timestamp(const pgdb_result_t* result, const int row, const char* field, time_t* buffer);

/**
 * @brief Retrievs a uuid value.
 *
 * @param result \ref pgdb_params_t containing query result. 
 * @param row Row index of data.
 * @param field Name of the column.
 * @param buffer Buffer to write to.
 *
 * @returns Returns 1 if value is NULL, otherwise 0 for success.
 */
int pgdb_get_uuid(const pgdb_result_t* result, const int row, const char* field, uuid_t** uuid);

/**
 * @brief Retrieves textual representation of enum and converts it to int using
 * the given function.
 *
 * @param result \ref pgdb_params_t containing query result. 
 * @param row Row index of data.
 * @param field Name of the column.
 * @param func Function which converts c str to int
 * @param buffer Buffer to write to.
 *
 * @returns Returns 1 if value is NULL, otherwise 0 for success. Sets buffer to
 * -1 if it wasnt found
 */
int pgdb_get_enum(const pgdb_result_t* result, const int row, const char* field, int(*conv)(const char*), int* buffer);

/**
 * @brief Checks if a column is given and field is not NULL.
 *
 * @param result \ref pgdb_params_t containing query result.
 * @param name Name of columns
 * @param row Row of value.
 *
 * @returns True if column exists.
 */
bool pgdb_exists(const pgdb_result_t* result, const char* name, const int row);

/**
 * @brief Converts from unix epoch to 01-01-2000 epoch in microseconds.
 *
 * @param timestamp UNIX timestamp.
 *
 * @returns Returns time in microseconds with epoch at 01-01-2000.
 */
int64_t pgdb_convert_to_pg_timestamp(const time_t timestamp);

/**
 * @brief Converts postgres timestamp to UNIX timestamp in seconds since 1970.
 *
 * @param timestamp Postgres timestamp.
 *
 * @returns Returns time in seconds since 1970.
 */
time_t pgdb_convert_to_unix_timestamp(const int64_t timestamp);

/**
 * @brief Struct which contains info about a claimed or free connection.
 */
typedef struct pgdb_connection {
	PGconn* connection; /**< Actual connection to database. */
	bool active; /**< If active, connection has been made to database. This does not guarantee a successfull conneciton, simply that the connection once has been made. */
	bool claimed; /**< If locked, connection is being used by thread. */
	time_t created; /**< Timestamp when connection was made. Used for keeping track of age. */
} pgdb_connection_t;

/**
 * @brief Struct which contains all connections and is capable of creating new ones or release old ones.
 * @todo Create function which checks in intervalls if there are connections, which must be freed.
 */
typedef struct pgdb_connection_queue {
	char* conn_info; /**< Connection info required to connect to database. */
	int max_connections; /**< Max amount of simultanious connections. */
	int64_t max_age; /**< Max age allowed of non active connection. */
	pgdb_connection_t* connections; /**< Array of connections. */
} pgdb_connection_queue_t;

/**
 * @brief Creates a new connection queue.
 *
 * @param max_connection Max simultanious connections.
 * @param max_age Max age of a connection which is not being used.
 *
 * @returns Pointer to new connection queue.
 */
pgdb_connection_queue_t* pgdb_connection_queue_new(const char* connection_info, int max_connections, int64_t max_age);

/**
 * @brief Frees a connection queue.
 *
 * @param queue Pointer to connection queue.
 */
void pgdb_connection_queue_free(pgdb_connection_queue_t** queue);

/**
 * @brief Checks if there is a free connection in queue. If not returns NULL.
 *
 * @param queue Queue to take connection from.
 * @param conn Connection which will be claimed. NULL if none could be found.
 *
 * @returns Returns 0 on success. If non is found, 0 is also returned. For failures like, connection could not be made to database, 1 is returned.
 *
 * @see pgdb_release_connection()
 */
int pgdb_claim_connection(pgdb_connection_queue_t* queue, pgdb_connection_t** conn);

/**
 * @brief Releases a connection and makes it available by pgdb_claim_connection()
 *
 * @see pgdb_claim_connection()
 */
void pgdb_release_connection(pgdb_connection_t** connection);

#if defined(__cplusplus)
}
#endif

#endif // RADICLE_PGDB_INCLUDE_RADICLE_PGDB_H

/** @} */
