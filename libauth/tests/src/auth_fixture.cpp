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
 * @brief Contains functions for faking pgdb account queries.
 * @author Nils Egger
 * @addtogroup testing 
 * @{
 * @addtogroup auth_testing
 * @{
 */

#include "radicle/tests/auth/auth_fixture.hpp"
#include "radicle/pgdb.h"

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
int set_fake_columns_attributes(PGresult* result, PGresAttDesc* descs, const int columns, const char** names) {
	for(int i = 0; i < columns; i++) {
		descs[i] = {(char*)names[i], 0, 0, 1, 0, 0, 0};
	}
	if(PQsetResultAttrs(result, columns, descs) == 0)
		return 1;
	return 0;
}

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
int set_fake_value(PGresult* result, const int row, const int column, char* value, const int length) {
	if(PQsetvalue(result, row, column, value, length) == 0) {
		return 1;
	}
	return 0;
}

int set_fake_uuid(PGresult* result, const int row, const int column) {
	char uuid[16] = {0x00};
	return set_fake_value(result, row, column, uuid, 16);
}

int set_fake_int(PGresult* result, const int row, const int column, int value) {
	uint32_t* buffer = (uint32_t*)malloc(sizeof(uint32_t));
	*buffer = htonl((uint32_t)value);
	if(set_fake_value(result, row, column, (char*)buffer, sizeof(uint32_t)))  {
		free(buffer);
		return 1;
	}
	free(buffer);
	return 0;
}

int set_fake_text(PGresult* result, const int row, const int column) {
	const char* txt = "I am a fake database string.!";
	return set_fake_value(result, row, column, (char*) txt, strlen(txt));
}

int set_fake_bool(PGresult* result, const int row, const int column, bool value) {
	return set_fake_value(result, row, column, (char*)&value, 1);
}

int pgdb_execute_param_fake(PGconn* conn, const char* stmt, const pgdb_params_t* params) {
	return 0;
}

int pgdb_fetch_param_fake_uuid(PGconn* conn, const char* stmt, const pgdb_params_t* params, pgdb_result_t** result) {
	*result = pgdb_result_new(PQmakeEmptyPGresult(conn, PGRES_TUPLES_OK));
	PGresAttDesc descs;
	const char* name = "uuid";
	if(set_fake_columns_attributes((*result)->pg, &descs, 1, &name)) {
		PQclear((*result)->pg);
		return 1;
	}
	if(set_fake_uuid((*result)->pg, 0, 0)) {
		PQclear((*result)->pg);
		return 1;
	}
	return 0;
}

int pgdb_fetch_param_fake_id(PGconn* conn, const char* stmt, const pgdb_params_t* params, pgdb_result_t** result) {
	*result = pgdb_result_new(PQmakeEmptyPGresult(conn, PGRES_TUPLES_OK));
	PGresAttDesc descs;
	const char* name = "id";
	if(set_fake_columns_attributes((*result)->pg, &descs, 1, &name)) {
		PQclear((*result)->pg);
		return 1;
	}
	if(set_fake_int((*result)->pg, 0, 0, 1)) {
		PQclear((*result)->pg);
		return 1;
	}
	return 0;
}

int pgdb_fetch_param_fake_account(PGconn* conn, const char* stmt, const pgdb_params_t* params, pgdb_result_t** result) {
	*result = pgdb_result_new(PQmakeEmptyPGresult(conn, PGRES_TUPLES_OK));
	PGresAttDesc descs[7];
	const char* columns[] = {"uuid", "email", "password", "role", "verified", "active", "created"};
	if(set_fake_columns_attributes((*result)->pg, descs, 7, columns)) {
		PQclear((*result)->pg);
		return 1;
	}
	if(set_fake_uuid((*result)->pg, 0, 0)) {
		PQclear((*result)->pg);
		return 1;
	}
	if(set_fake_text((*result)->pg, 0, 1)) {
		PQclear((*result)->pg);
		return 1;
	}
	if(set_fake_text((*result)->pg, 0, 2)) {
		PQclear((*result)->pg);
		return 1;
	}
	if(set_fake_text((*result)->pg, 0, 3)) {
		PQclear((*result)->pg);
		return 1;
	}
	if(set_fake_bool((*result)->pg, 0, 4, true)) {
		PQclear((*result)->pg);
		return 1;
	}
	if(set_fake_bool((*result)->pg, 0, 5, true)) {
		PQclear((*result)->pg);
		return 1;
	}
	if(set_fake_int((*result)->pg, 0, 6, 1624020176)) {
		PQclear((*result)->pg);
		return 1;
	}
	return 0;
}

int pgdb_fetch_param_fake_owned_session(PGconn* conn, const char* stmt, const pgdb_params_t* params, pgdb_result_t** result) {
	*result = pgdb_result_new(PQmakeEmptyPGresult(conn, PGRES_TUPLES_OK));
	PGresAttDesc descs[9];
	const char* columns[] = {"id", "salt", "uuid", "email", "password", "role", "verified", "active", "created"};
	if(set_fake_columns_attributes((*result)->pg, descs, 9, columns)) {
		PQclear((*result)->pg);
		return 1;
	}
	if(set_fake_int((*result)->pg, 0, 0, 1)) {
		PQclear((*result)->pg);
		return 1;
	}
	if(set_fake_text((*result)->pg, 0, 1)) {
		PQclear((*result)->pg);
		return 1;
	}
	if(set_fake_uuid((*result)->pg, 0, 2)) {
		PQclear((*result)->pg);
		return 1;
	}
	if(set_fake_text((*result)->pg, 0, 3)) {
		PQclear((*result)->pg);
		return 1;
	}
	if(set_fake_text((*result)->pg, 0, 4)) {
		PQclear((*result)->pg);
		return 1;
	}
	if(set_fake_text((*result)->pg, 0, 5)) {
		PQclear((*result)->pg);
		return 1;
	}
	if(set_fake_bool((*result)->pg, 0, 6, true)) {
		PQclear((*result)->pg);
		return 1;
	}
	if(set_fake_bool((*result)->pg, 0, 7, true)) {
		PQclear((*result)->pg);
		return 1;
	}
	if(set_fake_int((*result)->pg, 0, 8, 1624020176)) {
		PQclear((*result)->pg);
		return 1;
	}
	return 0;
}

int auth_generate_random_base64_fake(const int rand_bytes, string_t** buffer) {
	*buffer = string_from_literal("fPtjd+zvvzMafpvYuJC10Q==");	
	return 0;
}

int auth_hash_password_fake(const string_t* password, string_t** buffer) {
	*buffer = string_from_literal("$argon2i$v=19$m=16,t=2,p=1$VXpBNkVOT2h0U3BKV0NHYQ$5lylwjo684I2q5cj2NWhwA");
	return 0;
}

int auth_verify_password_fake(const string_t* encoded, const string_t* password) {
	return 0;
}

int hmac_sign_fake(const unsigned char* input, const size_t input_length, const string_t* key, string_t** buffer) {
	*buffer = string_from_literal("e65ca5c23a3af38236ab9723d8d5a1c0");
	return 0;
}

/** @} */
/** @} */
