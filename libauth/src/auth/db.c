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
#include <time.h>

#include <libpq-fe.h>

#include "radicle/pgdb.h"
#include "radicle/auth/db.h"

int auth_save_account(PGconn* conn, const auth_account_t* account, uuid_t** uuid) {
	const char* stmt = "INSERT INTO Accounts(uuid, email, password, role, verified, created, active) VALUES(gen_random_uuid(), $1::text, $2::text, $3::role, $4::boolean, $5::timestamp, TRUE) RETURNING uuid;";
	pgdb_result_t* result = NULL;
	pgdb_params_t* params = pgdb_params_new(5);

	pgdb_bind_text(account->email, 0, params);
	pgdb_bind_text(account->password, 1, params);
	pgdb_bind_text(account->role, 2, params);
	pgdb_bind_bool(account->verified, 3, params);
	pgdb_bind_timestamp(time(NULL), 4, params);

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

int auth_save_registration(PGconn* conn, const uuid_t* uuid, const string_t* token) {
	const char* stmt = "INSERT INTO Registrations(account, created, token) VALUES($1::uuid, now(), $2::text);";
	pgdb_params_t* params = pgdb_params_new(2);
	pgdb_bind_uuid(uuid, 0, params);
	pgdb_bind_text(token, 1, params);

	if(pgdb_execute_param(conn, stmt, params)) {
		pgdb_params_free(&params);
		return 1;
	}
	pgdb_params_free(&params);

	return 0;
}

int auth_save_session(PGconn* conn, const uuid_t* owner, const string_t* token, const time_t expires, const string_t* salt, uint32_t* id) {

	const char* stmt = "INSERT INTO Sessions(owner, token, created, expires, revoked, salt) VALUES($1::uuid, $2::text, $3::timestamp, $4::timestamp, FALSE, $5::text) RETURNING id;";
	pgdb_params_t* params = pgdb_params_new(5);
	if(owner != NULL) {
		pgdb_bind_uuid(owner, 0, params);
	}
	pgdb_bind_text(token, 1, params);
	pgdb_bind_timestamp(time(NULL), 2, params);
	pgdb_bind_timestamp(expires, 3, params);
	pgdb_bind_text(salt, 4, params);

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

int auth_save_session_access(PGconn* conn, const uint32_t session_id, const auth_requester_t* requester, const string_t* status) {
	const char* stmt = "INSERT INTO SessionAccesses(session_id, requester, url, status, date) VALUES($1::int4, $2::text, $3::text, $4::text, now());";
	pgdb_params_t* params = pgdb_params_new (4);

	pgdb_bind_uint32(session_id, 0, params);
	pgdb_bind_text(requester->ip, 1, params);
	pgdb_bind_text(requester->path, 2, params);
	pgdb_bind_text(status, 3, params);

	if(pgdb_execute_param(conn, stmt, params)) {
		DEBUG("Failed to insert session access.\n");
		pgdb_params_free(&params);
		return 1;
	}

	pgdb_params_free(&params);
	return 0;
}

int auth_get_account_by_email(PGconn* conn, const string_t* email, auth_account_t** account) {
	const char* stmt = "SELECT uuid, password, role, verified, active, created FROM Accounts WHERE email = $1::text";
	pgdb_params_t* params = pgdb_params_new(1);
	pgdb_bind_text(email, 0, params);

	pgdb_result_t* result = NULL;
	if(pgdb_fetch_param(conn, stmt, params, &result)) {
		pgdb_params_free(&params);
		return 1;
	}
	pgdb_params_free(&params);
	
	if(PQntuples(result->pg) == 1) {
		*account  = calloc(1, sizeof(auth_account_t));
		pgdb_get_uuid(result, 0, "uuid", &(*account)->uuid);
		pgdb_get_text(result, 0, "password", &(*account)->password);
		pgdb_get_text(result, 0, "role", &(*account)->role);
		pgdb_get_bool(result, 0, "verified", &(*account)->verified);
		pgdb_get_bool(result, 0, "active", &(*account)->active);
		pgdb_get_timestamp(result, 0, "created", &(*account)->created);
		(*account)->email = string_copy(email);
	} 

	pgdb_result_free(&result);

	return 0;
}

int auth_get_session_by_cookie(PGconn* conn, const string_t* cookie, auth_session_t** session, auth_account_t** account) {
	const char* stmt = "SELECT Sessions.id, Sessions.salt, Accounts.uuid, Accounts.email, Accounts.role, Accounts.verified, Accounts.active, Accounts.created" \
			   " FROM Sessions LEFT JOIN Accounts ON Accounts.uuid = Sessions.owner WHERE Sessions.token=$1 AND" \
			   " revoked=FALSE AND expires>$2::timestamp LIMIT 1";
	pgdb_params_t* params = pgdb_params_new(2);
	pgdb_bind_text(cookie, 0, params);
	pgdb_bind_timestamp(time(NULL), 1, params);

	pgdb_result_t* result = NULL;
	if(pgdb_fetch_param(conn, stmt, params, &result)) {
		INFO("Failed to fetch account info.\n");
		pgdb_params_free(&params);
		return 1;
	}	
	pgdb_params_free(&params);

	if(PQntuples(result->pg) == 1) {
		*session = auth_session_new();
		pgdb_get_uint32(result, 0, "id", &(*session)->id);
		pgdb_get_text(result, 0, "salt", &(*session)->salt);

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
		DEBUG("No rows for session\n");
	}

	pgdb_result_free(&result);

	return 0;
}
