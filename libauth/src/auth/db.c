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
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <libpq-fe.h>

#include "radicle/pgdb.h"
#include "radicle/auth/db.h"

int auth_save_account(PGconn* conn, const auth_account_t* account, uuid_t** uuid) {
	const char* stmt = "INSERT INTO Accounts(uuid, email, password, role, verified, created, active) VALUES(gen_random_uuid(), $1::text, $2::text, $3::ACCOUNTS_ROLE, $4::boolean, $5::timestamp, TRUE) RETURNING uuid;";
	pgdb_result_t* result = NULL;
	pgdb_params_t* params = pgdb_params_new(5);

	pgdb_bind_text(account->email, params);
	pgdb_bind_text(account->password, params);
	pgdb_bind_text(account->role, params);
	pgdb_bind_bool(account->verified, params);
	pgdb_bind_timestamp(time(NULL), params);

	if(pgdb_fetch_param(conn, stmt, params, &result)) {
		pgdb_params_free(&params);
		return 1;
	}
	pgdb_params_free(&params);

	if(pgdb_get_uuid(result, 0, "uuid", uuid)) {
		ERROR("pgdb_get_text returned NULL for uuid. This should not be possible.\n");
		pgdb_result_free(&result);
		return 1;
	}
	pgdb_result_free(&result);
	return 0;
}

int auth_save_token(PGconn* conn, const uuid_t* owner, const string_t* token, token_type_t type) {
	const char* stmt = "INSERT INTO Tokens(owner, created, token, type) VALUES($1::uuid, now(), $2::text, $3::TOKEN_TYPE);";
	pgdb_params_t* params = pgdb_params_new(3);
	pgdb_bind_uuid(owner, params);
	pgdb_bind_text(token, params);
	pgdb_bind_c_str(token_type_to_str(type), params);

	int result = pgdb_execute_param(conn, stmt, params);
	pgdb_params_free(&params);
	return result;
}

int auth_save_session(PGconn* conn, const uuid_t* owner, const string_t* token, const time_t expires, const string_t* salt, uint32_t* id) {

	const char* stmt = "INSERT INTO Sessions(owner, token, created, expires, revoked, salt) VALUES($1::uuid, $2::text, $3::timestamp, $4::timestamp, FALSE, $5::text) RETURNING id;";
	pgdb_params_t* params = pgdb_params_new(5);
	if(owner != NULL)
		pgdb_bind_uuid(owner, params);
	else
		pgdb_bind_null(params);
	pgdb_bind_text(token, params);
	pgdb_bind_timestamp(time(NULL), params);
	pgdb_bind_timestamp(expires, params);
	pgdb_bind_text(salt, params);

	pgdb_result_t* result = NULL;

	if(pgdb_fetch_param(conn, stmt, params, &result)) {
		DEBUG("Failed to insert session data.\n");
		pgdb_params_free(&params);
		return 1;
	}
	pgdb_params_free(&params);

	if(pgdb_get_uint32(result, 0, "id", id)) {
		ERROR("pgdb_get_int returned NULL for id. This should not be possible.\n");
		pgdb_result_free(&result);
		return 1;
	}

	pgdb_result_free(&result);
	return 0;
}

int auth_save_session_access(PGconn* conn, const uint32_t session_id, const auth_request_log_t* request_log) {
	const char* stmt = "INSERT INTO SessionAccesses(session_id, requester_ip, requester_port, date, url, response_time, response_code, internal_status) "
			   "VALUES($1::int4, $2::text, $3::int4, $4::timestamp, $5::text, $6::int4, $7::int4, $8::int4);";
	pgdb_params_t* params = pgdb_params_new(8);

	pgdb_bind_uint32(session_id, params);
	pgdb_bind_text(request_log->ip, params);
	pgdb_bind_uint32(request_log->port, params);
	pgdb_bind_timestamp(request_log->date, params);
	pgdb_bind_text(request_log->url, params);
	pgdb_bind_uint32(request_log->response_time, params);
	pgdb_bind_uint32(request_log->response_code, params);
	pgdb_bind_uint32(request_log->internal_status, params);

	int result = pgdb_execute_param(conn, stmt, params);
	pgdb_params_free(&params);
	return result;
}

int auth_remove_token_by_owner(PGconn* conn, const uuid_t* owner, token_type_t type) {
	const char* stmt = "DELETE FROM Tokens WHERE owner=$1::uuid and type=$2::TOKEN_TYPE;";
	pgdb_params_t* params = pgdb_params_new(2);
	pgdb_bind_uuid(owner, params);
	pgdb_bind_c_str(token_type_to_str(type), params);
	int result = pgdb_execute_param(conn, stmt, params);
	pgdb_params_free(&params);
	return result;
}

int auth_verify_token(PGconn* conn, const string_t* token, uuid_t** owner, token_type_t* type) {
	const char* stmt = "DELETE FROM Tokens WHERE token=$1::text RETURNING owner, type;";
	pgdb_params_t* params = pgdb_params_new(1);
	pgdb_bind_text(token, params);

	pgdb_result_t* result = NULL;
	if(pgdb_fetch_param(conn, stmt, params, &result)) {
		pgdb_params_free(&params);
		*owner = NULL;
		return 1;
	}

	if(PQntuples(result->pg) == 1) {
		if(pgdb_get_uuid(result, 0, "owner", owner) || pgdb_get_enum(result, 0, "type", &token_type_from_str, (int*)type)) {
			pgdb_params_free(&params);
			pgdb_result_free(&result);
			*owner = NULL;
			*type = -1;
			return 1;
		}
	}

	pgdb_params_free(&params);
	pgdb_result_free(&result);

	return 0;
}

int auth_update_account_verification_status(PGconn* conn, const uuid_t* account, bool verified) {
	const char* stmt = "UPDATE Accounts SET verified=$1::boolean WHERE uuid=$2::uuid;";
	pgdb_params_t* params = pgdb_params_new(2);
	pgdb_bind_bool(verified, params);
	pgdb_bind_uuid(account, params);

	int result = pgdb_execute_param(conn, stmt, params);
	pgdb_params_free(&params);
	return result;
}

int auth_get_account_by_email(PGconn* conn, const string_t* email, auth_account_t** account) {
	const char* stmt = "SELECT uuid, password, role, verified, active, created FROM Accounts WHERE email = $1::text";
	pgdb_params_t* params = pgdb_params_new(1);
	pgdb_bind_text(email, params);

	pgdb_result_t* result = NULL;
	if(pgdb_fetch_param(conn, stmt, params, &result)) {
		*account = NULL;
		pgdb_params_free(&params);
		return 1;
	}
	pgdb_params_free(&params);
	
	// @todo create helper function for this
	if(PQntuples(result->pg) == 1) {
		*account  = calloc(1, sizeof(auth_account_t));
		pgdb_get_uuid(result, 0, "uuid", &(*account)->uuid);
		pgdb_get_text(result, 0, "password", &(*account)->password);
		pgdb_get_text(result, 0, "role", &(*account)->role);
		pgdb_get_bool(result, 0, "verified", &(*account)->verified);
		pgdb_get_bool(result, 0, "active", &(*account)->active);
		pgdb_get_timestamp(result, 0, "created", &(*account)->created);
		(*account)->email = string_copy(email);
	} else {
	       *account = NULL;
	}	       

	pgdb_result_free(&result);

	return 0;
}

int auth_get_session_by_cookie(PGconn* conn, const string_t* cookie, uint32_t* id, string_t** salt, auth_account_t** account) {
	const char* stmt = "SELECT Sessions.id, Sessions.salt, Accounts.uuid, Accounts.email, Accounts.role, Accounts.verified, Accounts.active, Accounts.created" \
			   " FROM Sessions LEFT JOIN Accounts ON Accounts.uuid = Sessions.owner WHERE Sessions.token=$1 AND" \
			   " revoked=FALSE AND expires>$2::timestamp LIMIT 1";
	pgdb_params_t* params = pgdb_params_new(2);
	pgdb_bind_text(cookie, params);
	pgdb_bind_timestamp(time(NULL), params);

	pgdb_result_t* result = NULL;
	if(pgdb_fetch_param(conn, stmt, params, &result)) {
		INFO("Failed to fetch account info.\n");
		pgdb_params_free(&params);
		return 1;
	}	
	pgdb_params_free(&params);

	if(PQntuples(result->pg) == 1) {
		pgdb_get_uint32(result, 0, "id", id);
		pgdb_get_text(result, 0, "salt", salt);

		if(pgdb_exists(result, "uuid", 0)) {
			*account  = calloc(1, sizeof(auth_account_t));
			pgdb_get_uuid(result, 0, "uuid", &(*account)->uuid);
			pgdb_get_text(result, 0, "email", &(*account)->email);
			pgdb_get_text(result, 0, "password", &(*account)->password);
			pgdb_get_text(result, 0, "role", &(*account)->role);
			pgdb_get_bool(result, 0, "verified", &(*account)->verified);
			pgdb_get_bool(result, 0, "active", &(*account)->active);
			pgdb_get_timestamp(result, 0, "created", &(*account)->created);
		}
	} else {
		*id = 0;
		DEBUG("No rows for session\n");
	}

	pgdb_result_free(&result);

	return 0;
}
