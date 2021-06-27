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

#include "radicle/pgdb.h"
#include "radicle/tests/subhook.h"
#include "radicle/tests/radicle_fixture.hpp"

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
 *
 * @returns Returns 0 on succes.
 */
int set_fake_uuid(PGresult* result, const int row, const int column);

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
 * @returns Returns 0 on succes.
 */
int set_fake_text(PGresult* result, const int row, const int column);
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


class RadiclePGDBHooks: public RadicleTests {

	protected:
		subhook_t install_fetch_uuid_hook() {
			subhook_t buf = subhook_new((void*)pgdb_fetch_param, (void*)pgdb_fetch_param_fake_uuid, SUBHOOK_64BIT_OFFSET);
			install_hook(buf);
			return buf;
		}

		subhook_t install_fetch_id_hook() {
			subhook_t buf = subhook_new((void*)pgdb_fetch_param, (void*)pgdb_fetch_param_fake_id, SUBHOOK_64BIT_OFFSET);
			install_hook(buf);
			return buf;
		}

		subhook_t install_execute_param_always_success() {
			subhook_t buf = subhook_new((void*)pgdb_execute_param, (void*)pgdb_execute_param_fake, SUBHOOK_64BIT_OFFSET);
			install_hook(buf);
			return buf;
		}
};
