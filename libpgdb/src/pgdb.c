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

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "radicle/pgdb.h"
#include "radicle/types/string.h"
#include "radicle/types/uuid.h"

pgdb_params_t* pgdb_params_new(const int count) {
	pgdb_params_t* buf = calloc(1, sizeof(pgdb_params_t));
	buf->count = count;
	buf->types = calloc(count, sizeof(Oid));
	buf->values = calloc(count, sizeof(char*));
	buf->lengths = calloc(count, sizeof(int));
	buf->formats = calloc(count, sizeof(int));
	return buf;
}

void pgdb_params_free(pgdb_params_t** ptr) {
	if(*ptr == NULL) return;
	free((*ptr)->types);
	for(int i = 0; i < (*ptr)->count; i++) {
		free((*ptr)->values[i]);
	}
	free((*ptr)->values);
	free((*ptr)->lengths);
	free((*ptr)->formats);
	free(*ptr);
	*ptr = NULL;
}

pgdb_result_t* pgdb_result_new(PGresult* result) {
	pgdb_result_t* buf = calloc(1, sizeof(pgdb_result_t));
	buf->pg= result;
	return buf;
}

void pgdb_result_free(pgdb_result_t** result) {
	if(*result == NULL) return;
	PQclear((*result)->pg);
	free(*result);
	*result = NULL;
}

int pgdb_connect(const char* conninfo, PGconn** connection) {
	*connection = PQconnectdb(conninfo);
	ConnStatusType status = PQstatus(*connection);

	if(status == CONNECTION_BAD) {
		PQfinish(*connection);
		*connection = NULL;
		return EXIT_FAILURE;
	}
	return 0;
}

const char* pgdb_ping_info(const char* conninfo, PGPing* status) {
	*status = PQping(conninfo);	
	switch(*status) {
		case PQPING_OK:
			return "The server is running and appears to be accepting connections.";
		case PQPING_REJECT:
			return "The server is running but is in a state that disallows connections (startup, shutdown, or crash recovery).";
		case PQPING_NO_RESPONSE:
			return "The server could not be contacted. This might indicate that the server is not running, or that there is something wrong with the given connection parameters (for example, wrong port number), or that there is a network connectivity problem (for example, a firewall blocking the connection request).";
		case PQPING_NO_ATTEMPT:
			return "No attempt was made to contact the server, because the supplied parameters were obviously incorrect or there was some client-side problem (for example, out of memory).";
	}
}

/**
 * Copies the error_message from libpq and frees the associated PGresult.
 *
 * @param result PGresult of previous query.
 * @param error_msg Double pointer to char array which will be used as buffer.
 *
 * @returns Returns void.
 */
void pgdb_copy_error_message(PGresult* result, char** error_msg) {
	char* tmp = PQresultErrorMessage(result);
	size_t len = strlen(tmp);
	*error_msg = malloc(sizeof(char)*len);
	memcpy(*error_msg, tmp, len);
	PQclear(result);
}

/**
 * Checks if PGresult status is OK and if not, fills error_msg with the sql error supplied by libpq. 
 * Frees all associated data.
 *
 * @param result pgdb_result_t used for query.
 * @param expected_result_code Expected result code of libpq. PGRES_COMMAND_OK, PGRES_TUPLES_OK
 *
 * @returns 0 for success.
 */
int pgdb_manage_query(pgdb_result_t** result, int expected_result_code) {
	(*result)->status = PQresultStatus((*result)->pg);
	if((*result)->status != expected_result_code) {
		DEBUG("%s: %s\n", PQresStatus((*result)->status), PQresultErrorMessage((*result)->pg));
		pgdb_result_free(result);
		return 1;
	}
	return 0;	
}

int pgdb_execute(PGconn* conn, const char* stmt) {
	pgdb_result_t* result = pgdb_result_new(PQexec(conn, stmt));
	int r = pgdb_manage_query(&result, PGRES_COMMAND_OK);	
	pgdb_result_free(&result);
	return r;
}

int pgdb_execute_param(PGconn* conn, const char* stmt, const pgdb_params_t* params) {
	pgdb_result_t* result = pgdb_result_new(
			PQexecParams(conn, stmt, params->count, params->types, (const char* const*)params->values, params->lengths, params->formats, 1));
	int r = pgdb_manage_query(&result, PGRES_COMMAND_OK);	
	pgdb_result_free(&result);
	return r;
}

/**
 * Checks if PGresult status is PGRES_TUPLES_OK and if not, fills error_msg with the sql error supplied by libpq. 
 * Only frees result if an error occured.
 *
 * @param result Double pointer to PGresult recently used to send a query.
 * @param status Returned Query status returned by postgres.
 * @param error_msg Error message received by postgres. Needs to be freed on error. 
 *
 * @returns 0 for success.
 */

int pgdb_fetch_param(PGconn* conn, const char* stmt, const pgdb_params_t* params, pgdb_result_t** result) {
	*result = pgdb_result_new(PQexecParams(conn, stmt, params->count, params->types, (const char* const*)params->values, params->lengths, params->formats, 1));
	return pgdb_manage_query(result, PGRES_TUPLES_OK);	
}

int pgdb_transaction_begin(PGconn* conn) {
	return pgdb_execute(conn, "BEGIN;");
}

int pgdb_transaction_commit(PGconn* conn) {
	return pgdb_execute(conn, "COMMIT;");
}

int pgdb_transaction_rollback(PGconn* conn) {
	return pgdb_execute(conn, "ROLLBACK;");
}

void pgdb_bind_uint32(int value, int index, pgdb_params_t* params) {
	uint32_t* buffer = malloc(sizeof(uint32_t));
	*buffer = htonl((uint32_t)value);

	params->values[index] = (char*)buffer;
	params->lengths[index] = sizeof(uint32_t);
	params->formats[index] = 1;
}

void pgdb_bind_uint64(const uint64_t value, const int index, pgdb_params_t* params) {
	uint32_t* buffer = malloc(sizeof(uint64_t));
	*buffer = htonll((uint64_t)(value));

	params->values[index] = (char*)buffer;
	params->lengths[index] = sizeof(uint64_t);
	params->formats[index] = 1;
}

void pgdb_bind_text(const string_t* text, int index, pgdb_params_t* params) {
	params->lengths[index] = text->length;
	params->values[index] = calloc(text->length, sizeof(char));
	params->formats[index] = 1;

	memcpy(params->values[index], text->ptr, text->length);
}

void pgdb_bind_uuid(const uuid_t* uuid, int index, pgdb_params_t* params) {
	params->lengths[index] = 16;
	params->values[index] = calloc(16, sizeof(char));
	params->formats[index] = 1;

	memcpy(params->values[index], (char*)uuid->bin, 16);
}

void pgdb_bind_bool(const bool flag, const int index, pgdb_params_t* params) {
	params->lengths[index] = 1;
	params->values[index] = calloc(1, sizeof(char));
	params->formats[index] = 1;

	memcpy(params->values[index], &flag, 1);
}

void pgdb_bind_timestamp(const time_t timestamp, const int index, pgdb_params_t* params) {
	time_t * buffer = malloc(sizeof(time_t));
	*buffer = htonll(pgdb_convert_to_pg_timestamp(timestamp));
	params->values[index] = (char*)buffer;
	params->lengths[index] = sizeof(time_t);
	params->formats[index] = 1;
}

int pgdb_get_text(const pgdb_result_t* result, const int row, const char* field, string_t** buffer) {
	int column = PQfnumber(result->pg, field);
	if(column == -1 || PQgetisnull(result->pg, row, column)) {
		*buffer = NULL;
		return 1;
	}
	*buffer = string_new(PQgetvalue(result->pg, row, column), PQgetlength(result->pg, row, column));
	return 0;
}

int pgdb_get_uint32(const pgdb_result_t* result, const int row, const char* field, uint32_t* buffer) {
	int column = PQfnumber(result->pg, field);
	if(column == -1 || PQgetisnull(result->pg, row, column)) {
		return 1;
	}
  	*buffer = ntohl(*((uint32_t *)PQgetvalue(result->pg, row, column)));
	return 0;
}

int pgdb_get_uint64(const pgdb_result_t* result, const int row, const char* field, uint64_t* buffer) {
	int column = PQfnumber(result->pg, field);
	if(column == -1 || PQgetisnull(result->pg, row, column)) {
		return 1;
	}
  	*buffer = ntohll(*(uint64_t*)PQgetvalue(result->pg, row, column));
	return 0;

}

int pgdb_get_bool(const pgdb_result_t* result, const int row, const char* field, bool* buffer) {
	int column = PQfnumber(result->pg, field);
	if(column == -1 || PQgetisnull(result->pg, row, column)) {
		return 1;
	}
	if(*PQgetvalue(result->pg, row, column) == 0x00) {
		*buffer = false;
	} else {
		*buffer = true;
	}
	return 0;
}

int pgdb_get_timestamp(const pgdb_result_t* result, const int row, const char* field, time_t* buffer) {
	int column = PQfnumber(result->pg, field);
	if(column == -1 || PQgetisnull(result->pg, row, column)) {
		return 1;
	}
	int64_t psql_timestamp = (int64_t)ntohll(*(uint64_t*)PQgetvalue(result->pg, row, column));
  	*buffer = pgdb_convert_to_unix_timestamp(psql_timestamp);
	return 0;
}

int pgdb_get_uuid(const pgdb_result_t* result, const int row, const char* field, uuid_t** buf) {
	int column = PQfnumber(result->pg, field);
	if(column == -1 || PQgetisnull(result->pg, row, column)) {
		*buf = NULL;
		return 1;
	}
	*buf = uuid_new((unsigned char*)PQgetvalue(result->pg, row, column));
	return 0;
}

bool pgdb_exists(const pgdb_result_t* result, const char* name, const int row) {
	int column = PQfnumber(result->pg, name);
	return column != -1 && !PQgetisnull(result->pg, row, column);
}

int64_t pgdb_convert_to_pg_timestamp(const time_t timestamp) {
	return ((int64_t)(timestamp - 946684800)) * 1000000;
}

time_t pgdb_convert_to_unix_timestamp(const int64_t timestamp) {
	return (time_t)(timestamp / 1000000) + 946684800;
}
