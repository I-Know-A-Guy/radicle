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
 * @brief Contains useful db fake fetching, querying for unit tests.
 * @author Nils Egger
 *
 * @addtogroup testing 
 * @{
 * @addtogroup pgdb_testing PGDB hooks
 * @{ 
 */

#include <gtest/gtest.h>
#include <libpq-fe.h>

#include "subhook.h"

#include "radicle/pgdb.h"
#include "radicle/tests/radicle_fixture.hpp"

#define PGDB_CREATE_FETCH_HOOK(name) subhook_new((void*)PQexecParams, (void*)name, SUBHOOK_64BIT_OFFSET)

#define PGDB_FETCH_FAKE(name)\
	PGresult* name(PGconn *conn, const char *command, int nParams, const Oid *paramTypes, const char *const *paramValues, const int *paramLengths, const int *paramFormats, int resultFormat)

#define PGDB_FAKE_RESULTS(status, n)\
	PGresult* result = PQmakeEmptyPGresult(NULL, status);\
	int column = -1;\
	PGresAttDesc descs[n];\
	if(set_fake_columns_attributes(result, descs, n, columns)) {\
		PQclear(result);\
		ERROR("Failed to set columns.\n");\
		return NULL;\
	}\

#define PGDB_FAKE_EMPTY_RESULT(status) return PQmakeEmptyPGresult(NULL, status);
	

#define PGDB_FAKE_RESULT_1(status, a)\
	const char* columns[] = {a};\
	PGDB_FAKE_RESULTS(status, 1);

#define PGDB_FAKE_RESULT_2(a, b)\
	const char* columns[] = {a, b};\
	PGDB_FAKE_RESULTS(status, 2);
	
       	

#define PGDB_FAKE_FINISH() return result;

#define PGDB_FAKE_VALUE(func, val)\
	if(func(result, 0, ++column, val)) {\
		ERROR("Failed to set fake value.\n");\
	}

#define PGDB_FAKE_UUID(val) PGDB_FAKE_VALUE(set_fake_uuid, val)
#define PGDB_FAKE_INT(val) PGDB_FAKE_VALUE(set_fake_int, val)

/**
 * @brief Fake which always returns PGRES_COMMAND_OK as status
 */
ExecStatusType pgdb_pq_result_status_fake_ok(PGconn* conn); 

/**
 * @brief Fake which always returns PGRES_FATAL_ERROR as status
 */
ExecStatusType pgdb_pq_result_status_fake_fatal(PGconn* conn); 

/**
 * @brief Fake which always returns PGRES_TUPLES_OK as status
 */
ExecStatusType pgdb_pq_result_status_fake_tuple(PGconn* conn); 

/**
 * @brief Simply returns NULL
 */
PGresult* pgdb_pq_exec_fake(PGconn* conn, const char* stmt);

/**
 * @brief Simply returns NULL
 */
PGresult* pgdb_pq_exec_param_fake(PGconn* conn, const char* stmt, int nParams, const Oid *paramTypes, const char *const *paramValues, const int *paramLengths, const int *paramFormats, int resultFormat);

/**
 * @brief Creates column attributes from name and length array.
 *
 * @param result Result which will contain columns.
 * @param descs Pointer to array to be filled.
 * @param columns Size of \p descs
 * @param names List of names of columns.
 *
 * @returns Returns 0 on success.
 */
int set_fake_columns_attributes(PGresult* result, PGresAttDesc* descs, const int columns, const char** names);

/**
 * @brief Sets values of a fake PGresult. Should only be used for testing.
 *
 * @param result Result to be filled with values.
 * @param row Row of result to fill.
 * @param column Column of value to be inserted.
 * @param value Column value.
 * @param length Column value length.
 *
 * @returns Returns 0 on succes.
 */
int set_fake_value(PGresult* result, const int row, const int column, char* value, const int length);

/**
 * @brief Sets a fake uuid.
 *
 * @param result Result to be filled with values.
 * @param row Row of result to fill.
 * @param column Column of value to be inserted.
 * @param bytes Array of size 16
 *
 * @returns Returns 0 on succes.
 */
int set_fake_uuid(PGresult* result, const int row, const int column, char* bytes);

/**
 * @brief Sets a fake int.
 *
 * @param result Result to be filled with values.
 * @param row Row of result to fill.
 * @param column Column of value to be inserted.
 * @param value Int to set.
 *
 * @returns Returns 0 on succes.
 */
int set_fake_int(PGresult* result, const int row, const int column, int value);

/**
 * @brief Sets a fake text.
 *
 * @param result Result to be filled with values.
 * @param row Row of result to fill.
 * @param column Column of value to be inserted.
 *
 * @returns Returns 0 on success.
 */
int set_fake_text(PGresult* result, const int row, const int column);

/**
 * @brief Sets a fake c str.
 *
 * @param result Result to be filled with values.
 * @param row Row of result to fill.
 * @param column Column of value to be inserted.
 * @param txt C Str whihc will be set
 *
 * @returns Returns 0 on success.
 */
int set_fake_c_str(PGresult* result, const int row, const int column, const char* txt);

/**
 * @brief Sets a fake bool.
 *
 * @param result Result to be filled with values.
 * @param row Row of result to fill.
 * @param column Column of value to be inserted.
 * @param value Value to set.
 *
 * @returns Returns 0 on succes.
 */
int set_fake_bool(PGresult* result, const int row, const int column, bool value);

/**
 * @brief Fake function for pgdb_execute_param which simply returns success without doing anyhting.
 *
 * @returns Returns 0.
 */
int pgdb_execute_fake(PGconn* conn, const char* stmt);

/**
 * @brief Fake function for pgdb_execute_param which simply returns success without doing anyhting.
 *
 * @returns Returns 0.
 */
int pgdb_execute_param_fake(PGconn* conn, const char* stmt, const pgdb_params_t* params);

/**
 * @brief Fake function for pgdb_fetch_param which should return an uuid.
 *
 * @returns Returns 0 and sets fills result with a single row containing a NILL uuid.
 */
int pgdb_fetch_param_fake_uuid(PGconn* conn, const char* stmt, const pgdb_params_t* params, pgdb_result_t** result);

/**
 * @brief Fake function for pgdb_fetch_param which should return an id.
 *
 * @returns Returns 0 and sets fills result with a single row containing a column called id with value set to 1.
 */
int pgdb_fetch_param_fake_id(PGconn* conn, const char* stmt, const pgdb_params_t* params, pgdb_result_t** result);

/**
 * @brief Fake function for \ref pgdb_connect which always returns success. Connection will stay null.
 *
 * @returns 0
 */
int pgdb_connect_fake(const char* conninfo, PGconn** connection);

/**
 * @brief Provides different hooks for faking pgdb_* methods
 */
class RadiclePGDBHooks: public RadicleTests {

	protected:
		subhook_t install_status_command_ok() {
			return install_hook(subhook_new((void*)PQresultStatus, (void*)pgdb_pq_result_status_fake_ok, SUBHOOK_64BIT_OFFSET));
		}

		subhook_t install_status_fatal_error() {
			return install_hook(subhook_new((void*)PQresultStatus, (void*)pgdb_pq_result_status_fake_fatal, SUBHOOK_64BIT_OFFSET));
		}

		subhook_t install_status_tuples_ok() {
			return install_hook(subhook_new((void*)PQresultStatus, (void*)pgdb_pq_result_status_fake_tuple, SUBHOOK_64BIT_OFFSET));
		}

		subhook_t install_pg_exec_hook() {
			return install_hook(subhook_new((void*)PQexec, (void*)pgdb_pq_exec_fake, SUBHOOK_64BIT_OFFSET));
		}

		subhook_t install_pg_exec_param_hook() {
			return install_hook(subhook_new((void*)PQexecParams, (void*)pgdb_pq_exec_param_fake, SUBHOOK_64BIT_OFFSET));
		}
		
		/**
		 * @brief Replaces \ref pgdb_fetch_param with \ref pgdb_fetch_param_fake_uuid
		 */
		subhook_t install_fetch_uuid_hook() {
			subhook_t buf = subhook_new((void*)pgdb_fetch_param, (void*)pgdb_fetch_param_fake_uuid, SUBHOOK_64BIT_OFFSET);
			install_hook(buf);
			return buf;
		}

		/**
		 * @brief Replaces \ref pgdb_fetch_param with \ref pgdb_fetch_param_fake_id
		 */
		subhook_t install_fetch_id_hook() {
			subhook_t buf = subhook_new((void*)pgdb_fetch_param, (void*)pgdb_fetch_param_fake_id, SUBHOOK_64BIT_OFFSET);
			install_hook(buf);
			return buf;
		}

		/**
		 * @brief Replaces \ref pgdb_execute with \ref pgdb_execute_fake 
		 */
		subhook_t install_execute_always_success() {
			subhook_t buf = subhook_new((void*)pgdb_execute, (void*)pgdb_execute_fake, SUBHOOK_64BIT_OFFSET);
			install_hook(buf);
			return buf;
		}

		/**
		 * @brief Replaces \ref pgdb_execute_param with \ref pgdb_execute_param_fake 
		 */
		subhook_t install_execute_param_always_success() {
			subhook_t buf = subhook_new((void*)pgdb_execute_param, (void*)pgdb_execute_param_fake, SUBHOOK_64BIT_OFFSET);
			install_hook(buf);
			return buf;
		}

		/**
		 * @brief Replaces \ref pgdb_connect with pgdb_connect_fake
		 */
		subhook_t install_pgdb_connect_fake() {
			subhook_t buf = subhook_new((void*)pgdb_connect, (void*)pgdb_connect_fake, SUBHOOK_64BIT_OFFSET);
			install_hook(buf);
			return buf;
		}
};
