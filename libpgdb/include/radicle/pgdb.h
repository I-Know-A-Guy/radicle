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
 *
 * @todo Create pgdb_bind_timestamp which converts timestap to valid ISO timestamp string.
 *
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
 * @brief Binds int32 to query.
 * @todo change to uint32_t
 *
 * @param value Value to bind. 
 * @param index Index to use in arrays.
 * @param param \ref pgdb_params_t struct to use for binding.
 *
 * @returns Returns void.
 */
void pgdb_bind_int32(const int value, const int index, pgdb_params_t* params);

/**
 * @brief Binds timestamp to query.
 *
 * @param value Value to bind. 
 * @param index Index to use in arrays.
 * @param param \ref pgdb_params_t struct to use for binding.
 *
 * @returns Returns void.
 */
void pgdb_bind_uint64(const uint64_t value, const int index, pgdb_params_t* params);

/**
 * @brief Binds text to query.
 *
 * @param text Pointer to text. 
 * @param index Index to use in arrays.
 * @param param \ref pgdb_params_t struct to use for binding.
 *
 * @returns Returns void. 
 */
void pgdb_bind_text(const string_t* text, const int index, pgdb_params_t* params);

/**
 * @brief Binds uuid to query.
 *
 * @param uuid Pointer to uuid to bind. 
 * @param index Index to use in arrays.
 * @param param \ref pgdb_params_t struct to use for binding.
 *
 * @returns Returns void. 
 */
void pgdb_bind_uuid(const uuid_t* text, const int index, pgdb_params_t* params);

/**
 * @brief Binds boolean to query.
 *
 * @param flag Boolean value to bind. 
 * @param index Index to use in arrays.
 * @param param \ref pgdb_params_t struct to use for binding.
 *
 * @returns Returns void. 
 */
void pgdb_bind_bool(const bool flag, const int index, pgdb_params_t* params);


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
 * @brief Retrievs a int32.
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
 * @brief Checks if a column is given and field is not NULL.
 *
 * @param result \ref pgdb_params_t containing query result.
 * @param name Name of columns
 * @param row Row of value.
 *
 * @returns True if column exists.
 */
bool pgdb_exists(const pgdb_result_t* result, const char* name, const int row);

#if defined(__cplusplus)
}
#endif

#endif // RADICLE_PGDB_INCLUDE_RADICLE_PGDB_H

/** @} */
