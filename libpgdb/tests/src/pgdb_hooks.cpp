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

#include "radicle/tests/pgdb_hooks.hpp"

int set_fake_columns_attributes(PGresult* result, PGresAttDesc* descs, const int columns, const char** names) {
	for(int i = 0; i < columns; i++) {
		descs[i] = {(char*)names[i], 0, 0, 1, 0, 0, 0};
	}
	if(PQsetResultAttrs(result, columns, descs) == 0)
		return 1;
	return 0;
}

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

