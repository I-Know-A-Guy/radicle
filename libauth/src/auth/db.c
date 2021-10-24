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

#include "radicle/auth/types.h"
#include "radicle/pgdb.h"
#include "radicle/auth/db.h"

int auth_save_account(PGconn* conn, const auth_account_t* account, uuid_t** uuid) {
	const char* stmt = "INSERT INTO Accounts(uuid, email, password, role, verified, created, active) VALUES(gen_random_uuid(), $1::text, $2::text, $3::ACCOUNTS_ROLE, $4::boolean, $5::timestamp, TRUE) RETURNING uuid;";
	pgdb_result_t* result = NULL;
	pgdb_params_t* params = pgdb_params_new(5);

	pgdb_bind_text(account->email, params);
	pgdb_bind_text(account->password, params);
	pgdb_bind_c_str(auth_account_role_to_str(account->role), params);
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

int auth_update_account_email(PGconn* conn, const uuid_t* uuid, const string_t* email) {
	const char* stmt = "UPDATE Accounts SET email=$1::text, verified=true WHERE uuid=$2::uuid;";

	pgdb_params_t* params = pgdb_params_new(2);
	pgdb_bind_text(email, params);
	pgdb_bind_uuid(uuid, params);

	int result = pgdb_execute_param(conn, stmt, params);
	pgdb_params_free(&params);
	return result;

}

int auth_update_account_password(PGconn* conn, const uuid_t* uuid, const string_t* password) {
	const char* stmt = "UPDATE Accounts SET password=$1::text WHERE uuid=$2::uuid;";

	pgdb_params_t* params = pgdb_params_new(2);
	pgdb_bind_text(password, params);
	pgdb_bind_uuid(uuid, params);

	int result = pgdb_execute_param(conn, stmt, params);
	pgdb_params_free(&params);
	return result;
}

int auth_save_token(PGconn* conn, const uuid_t* owner, const string_t* token, token_type_t type, const string_t* custom) {
	const char* stmt = "INSERT INTO Tokens(owner, created, token, type, custom) VALUES($1::uuid, now(), $2::text, $3::TOKEN_TYPE, $4::text);";
	pgdb_params_t* params = pgdb_params_new(4);
	pgdb_bind_uuid(owner, params);
	pgdb_bind_text(token, params);
	pgdb_bind_c_str(token_type_to_str(type), params);

	if(custom != NULL)
		pgdb_bind_text(custom, params);
	else
		pgdb_bind_null(params);


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

int auth_verify_token(PGconn* conn, const string_t* token, token_type_t expected_type, uuid_t** owner, string_t** custom) {
	const char* stmt = "DELETE FROM Tokens WHERE token=$1::text AND type=$2::TOKEN_TYPE RETURNING owner, custom;";
	pgdb_params_t* params = pgdb_params_new(2);
	pgdb_bind_text(token, params);
	pgdb_bind_c_str(token_type_to_str(expected_type), params);

	pgdb_result_t* result = NULL;
	if(pgdb_fetch_param(conn, stmt, params, &result)) {
		pgdb_params_free(&params);
		*owner = NULL;
		return 1;
	}

	if(PQntuples(result->pg) == 1) {
		if(pgdb_get_uuid(result, 0, "owner", owner)) {
			pgdb_params_free(&params);
			pgdb_result_free(&result);
			*owner = NULL;
			return 1;
		}
		if(custom != NULL)
			pgdb_get_text(result, 0, "custom", custom);
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
		pgdb_get_enum(result, 0, "role", &auth_account_role_from_str, (int*)&(*account)->role);
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
		if(pgdb_get_uint32(result, 0, "id", id) || pgdb_get_text(result, 0, "salt", salt)) {
			DEBUG("Cannot find columnd id or salt\n");
			pgdb_result_free(&result);
			string_free(salt);
			*id = 0;
			return 1;
		}

		if(pgdb_exists(result, "uuid", 0)) {
			*account  = calloc(1, sizeof(auth_account_t));
			if(
			pgdb_get_uuid(result, 0, "uuid", &(*account)->uuid) ||
			pgdb_get_text(result, 0, "email", &(*account)->email) ||
			pgdb_get_enum(result, 0, "role", &auth_account_role_from_str, (int*)&(*account)->role) ||
			pgdb_get_bool(result, 0, "verified", &(*account)->verified) ||
			pgdb_get_bool(result, 0, "active", &(*account)->active) ||
			pgdb_get_timestamp(result, 0, "created", &(*account)->created)) {
				DEBUG("Failed to read in all account columns\n");
				pgdb_result_free(&result);
				*id = 0;
				string_free(salt);
				auth_account_free(account);
				return 1;
			}
		}
	} else {
		*id = 0;
		DEBUG("No rows for session\n");
	}

	pgdb_result_free(&result);

	return 0;
}

int auth_blacklist_ip(PGconn* conn, const string_t* ip, const time_t date, const time_t ban_lift, uint32_t* id) {
	const char* stmt = "INSERT INTO Blacklist(ip, added, ban_lift) VALUES ($1::text, $2::timestamp, $3::timestamp) RETURNING id;";
	pgdb_params_t* params = pgdb_params_new(3);
	pgdb_bind_text(ip, params);
	pgdb_bind_timestamp(date, params);
	if(ban_lift != 0)
		pgdb_bind_timestamp(ban_lift, params);
	else 
		pgdb_bind_null(params);

	pgdb_result_t* result = NULL;
	if(pgdb_fetch_param(conn, stmt, params, &result)) {
		pgdb_params_free(&params);
		*id = 0;
		return 1;
	}
	pgdb_params_free(&params);

	if(pgdb_get_uint32(result, 0, "id", id)) {
		pgdb_result_free(&result);
		*id = 0;
		return 1;
	}
	pgdb_result_free(&result);
	return 0;
}

int auth_save_blacklist_access(PGconn* conn, const int id, const time_t date, const string_t* url) {
	const char* stmt = "INSERT INTO BlacklistAccesses(blacklist_id, date, url) VALUES ($1::int, $2::timestamp, $3::text);";
	pgdb_params_t* params = pgdb_params_new(3);
	pgdb_bind_uint32(id, params);
	pgdb_bind_timestamp(date, params);
	pgdb_bind_text(url, params);
	int result = pgdb_execute_param(conn, stmt, params);
	pgdb_params_free(&params);
	return result;
}

int auth_blacklist_lookup_ip(PGconn* conn, const string_t* ip, uint32_t* id) {
	const char* stmt = "SELECT id FROM Blacklist WHERE ip=$1::text AND (ban_lift IS NULL OR ban_lift < $2::timestamp) LIMIT 1;";

	pgdb_params_t* params = pgdb_params_new(2);
	pgdb_bind_text(ip, params);
	pgdb_bind_timestamp(time(NULL), params);

	pgdb_result_t* result = NULL;
	if(pgdb_fetch_param(conn, stmt, params, &result)) {
		pgdb_params_free(&params);
		return 1;
	}

	if(PQntuples(result->pg) == 1) {
		if(pgdb_get_uint32(result, 0, "id", id)) {
			*id = 0;
			pgdb_params_free(&params);
			pgdb_result_free(&result);
			DEBUG("Failed to extract id\n");
			return 1;
		}
	} else {
		*id = 0;
	}

	pgdb_params_free(&params);
	pgdb_result_free(&result);

	return 0;
}

int auth_session_lookup_ip(PGconn* conn, const string_t* ip, const time_t begin, list_t** results) {
	const char* stmt = "SELECT Sessions.owner, SessionAccesses.internal_status, SessionAccesses.response_code FROM SessionAccesses "
				"JOIN Sessions ON SessionAccesses.session_id=Sessions.id "
				"WHERE SessionAccesses.requester_ip=$1::text AND SessionAccesses.date > $2::timestamp;";

	pgdb_params_t* params = pgdb_params_new(2);
	pgdb_bind_text(ip, params);
	pgdb_bind_timestamp(begin, params);

	pgdb_result_t* result = NULL;
	if(pgdb_fetch_param(conn, stmt, params, &result)) {
		pgdb_params_free(&params);
		return 1;
	}
	pgdb_params_free(&params);

	int rows = PQntuples(result->pg);

	if(rows == 0) {
		pgdb_result_free(&result);
		*results = NULL;
		return 0;
	}

	for(int i = 0; i < rows; i++) {
		auth_session_access_entry_t* entry = (auth_session_access_entry_t*)calloc(1, sizeof(auth_session_access_entry_t));
		if(pgdb_get_uint32(result, i, "internal_status", &entry->internal_status) ||
		   pgdb_get_uint32(result, i, "response_code", &entry->response_code)) {

			pgdb_result_free(&result);
			list_free(*results, auth_session_access_entry_free);
			return 1;
		}
		pgdb_get_uuid(result, i, "owner", &entry->owner);
		list_tail(results, entry);
	}
		
	pgdb_result_free(&result);
	return 0;
}
