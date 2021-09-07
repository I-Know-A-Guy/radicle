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

#include "radicle/tests/auth/auth_fixture.hpp"
#include "radicle/auth/types.h"
#include "radicle/pgdb.h"

int pgdb_fetch_param_fake_account(PGconn* conn, const char* stmt, const pgdb_params_t* params, pgdb_result_t** result) {
	*result = pgdb_result_new(PQmakeEmptyPGresult(conn, PGRES_TUPLES_OK));
	PGresAttDesc descs[7];
	const char* columns[] = {"uuid", "email", "password", "role", "verified", "active", "created"};
	if(set_fake_columns_attributes((*result)->pg, descs, 7, columns)) {
		PQclear((*result)->pg);
		return 1;
	}
	char uuid[16] = {0x00};
	if(set_fake_uuid((*result)->pg, 0, 0, uuid)) {
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
	char uuid[16] = {0x00};
	if(set_fake_uuid((*result)->pg, 0, 2, uuid)) {
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

int pgdb_fetch_param_fake_token(PGconn* conn, const char* stmt, const pgdb_params_t* params, pgdb_result_t** result) {
	*result = pgdb_result_new(PQmakeEmptyPGresult(conn, PGRES_TUPLES_OK));
	PGresAttDesc descs[2];
	const char* names[] = {"owner", "type"};
	if(set_fake_columns_attributes((*result)->pg, descs, 2, names)) {
		PQclear((*result)->pg);
		return 1;
	}
	char uuid[16] = {0x00};
	if(set_fake_uuid((*result)->pg, 0, 0, uuid)) {
		PQclear((*result)->pg);
		return 1;
	}
	if(set_fake_c_str((*result)->pg, 0, 1, token_type_to_str(REGISTRATION))) {
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
