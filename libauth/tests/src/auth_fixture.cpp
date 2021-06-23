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
 * @brief Sets values of a fake PGresult. Should only be used for testing.
 * @bug Doesnt work for multiple values. Instead pass around PGresAttDesc pointer.
 *
 * @param result Result to be filled with values.
 * @param row Row of result to fill.
 * @param column Column of value to be inserted.
 * @param name Column name.
 * @param value Column value.
 * @param length Column value length.
 *
 * @returns Returns 0 on succes.
 */
int set_fake_value(PGresult* result, const int row, const int column, const char* name, char* value, int length) {
	PGresAttDesc attr = {(char*)name, 0, 0, 1, 0, length, 0};
	// TODO for multiple list is needed ,
	if(PQsetResultAttrs(result, 1, &attr) == 0) {
		return 1;
	}
	if(PQsetvalue(result, row, column, value, length) == 0) {
		return 1;
	}
	return 0;
}

int set_fake_uuid(PGresult* result, const int row, const int column, const char* name) {
	char uuid[16] = {0x00};
	return set_fake_value(result, row, column, name, uuid, 16);
}

int set_fake_int(PGresult* result, const int row, const int column, const char* name, int value) {
	uint32_t* buffer = (uint32_t*)malloc(sizeof(uint32_t));
	*buffer = htonl((uint32_t)value);
	if(set_fake_value(result, row, column, name, (char*)buffer, sizeof(uint32_t)))  {
		free(buffer);
		return 1;
	}
	free(buffer);
	return 0;
}

int set_fake_text(PGresult* result, const int row, const int column, const char* name) {
	const char* txt = "I am a fake database string.!";
	return set_fake_value(result, row, column, name, (char*) txt, strlen(txt));
}

int set_fake_bool(PGresult* result, const int row, const int column, const char* name, bool value) {
	return set_fake_value(result, row, column, name, (char*)&value, 1);
}

int pgdb_execute_param_fake(PGconn* conn, const char* stmt, const pgdb_params_t* params) {
	return 0;
}

int pgdb_fetch_param_fake_uuid(PGconn* conn, const char* stmt, const pgdb_params_t* params, pgdb_result_t** result) {
	*result = pgdb_result_new(PQmakeEmptyPGresult(conn, PGRES_TUPLES_OK));
	return set_fake_uuid((*result)->pg, 0, 0, "uuid");	
}

int pgdb_fetch_param_fake_id(PGconn* conn, const char* stmt, const pgdb_params_t* params, pgdb_result_t** result) {
	*result = pgdb_result_new(PQmakeEmptyPGresult(conn, PGRES_TUPLES_OK));
	return set_fake_int((*result)->pg, 0, 0, "id", 1);	
}

int pgdb_fetch_param_fake_complete(PGconn* conn, const char* stmt, const pgdb_params_t* params, pgdb_result_t** result) {
	*result = pgdb_result_new(PQmakeEmptyPGresult(conn, PGRES_TUPLES_OK));
	set_fake_uuid((*result)->pg, 0, 0, "uuid");
	set_fake_text((*result)->pg, 0, 1, "password");
	set_fake_text((*result)->pg, 0, 2, "role");
	set_fake_bool((*result)->pg, 0, 3, "verified", true);
	set_fake_bool((*result)->pg, 0, 4, "active", true);
	set_fake_int((*result)->pg, 0, 5, "created", 1624020176);
	set_fake_int((*result)->pg, 0, 6, "id", 1);
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
