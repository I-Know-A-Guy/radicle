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
#include <stdio.h>

#include "radicle/pgdb.h"
#include "radicle/auth/setup.h"

int auth_create_db_tables(PGconn* conn) {
	const char* drop_accounts_role_enum = "DROP TYPE IF EXISTS ACCOUNTS_ROLE CASCADE;";
	const char* drop_accounts_table = "DROP TABLE IF EXISTS Accounts CASCADE;";
	const char* drop_token_type_enum = "DROP TYPE IF EXISTS TOKEN_TYPE CASCADE;";
	const char* drop_tokens_table = "DROP TABLE IF EXISTS Tokens CASCADE;";
	const char* drop_sessions_table = "DROP TABLE IF EXISTS Sessions CASCADE;";
	const char* drop_sessions_accesses_table = "DROP TABLE IF EXISTS SessionAccesses CASCADE;";
	const char* drop_blacklist_table = "DROP TABLE IF EXISTS Blacklist CASCADE;";
	const char* drop_blacklist_accesses_table = "DROP TABLE IF EXISTS BlacklistAccesses CASCADE;";

	const char* accounts_role_enum = "CREATE TYPE ACCOUNTS_ROLE AS ENUM ('admin', 'user');";

	const char* accounts_table = 
"CREATE TABLE Accounts(\
uuid UUID PRIMARY KEY,\
email TEXT NOT NULL UNIQUE,\
password TEXT NOT NULL,\
role ACCOUNTS_ROLE NOT NULL,\
verified BOOLEAN DEFAULT FALSE,\
created TIMESTAMP NOT NULL,\
active BOOLEAN DEFAULT TRUE\
);";

	const char* tokens_type_enum = "CREATE TYPE TOKEN_TYPE AS ENUM ('registration', 'password_reset');";

	const char* tokens_table = 
"CREATE TABLE Tokens(\
id SERIAL PRIMARY KEY,\
owner UUID NOT NULL,\
token TEXT NOT NULL,\
created TIMESTAMP NOT NULL,\
type TOKEN_TYPE NOT NULL,\
FOREIGN KEY (owner) REFERENCES Accounts(uuid)\
);";

	const char* sessions_table = 
"CREATE TABLE Sessions(\
id SERIAL PRIMARY KEY,\
owner UUID,\
token TEXT NOT NULL,\
created TIMESTAMP NOT NULL,\
expires TIMESTAMP NOT NULL,\
revoked BOOLEAN DEFAULT TRUE,\
salt TEXT NOT NULL,\
FOREIGN KEY (owner) REFERENCES Accounts(uuid)\
);";

	const char* sessions_accesses_table =
"CREATE TABLE SessionAccesses(\
id SERIAL PRIMARY KEY,\
session_id INT NOT NULL,\
requester_ip TEXT NOT NULL,\
requester_port INT NOT NULL,\
date TIMESTAMP NOT NULL,\
url TEXT NOT NULL,\
response_time INT NOT NULL,\
response_code INT NOT NULL,\
internal_status INT NOT NULL,\
FOREIGN KEY (session_id) REFERENCES Sessions(id)\
);";

	const char* blacklist_table =
"CREATE TABLE Blacklist(\
id SERIAL PRIMARY KEY,\
ip TEXT NOT NULL,\
added TIMESTAMP NOT NULL\
);";

	const char* blacklist_accesses_table =
"CREATE TABLE BlacklistAccesses(\
id SERIAL PRIMARY KEY,\
blacklist_id INT NOT NULL,\
account UUID,\
date TIMESTAMP NOT NULL,\
FOREIGN KEY(blacklist_id) REFERENCES Blacklist(id),\
FOREIGN KEY(account) REFERENCES Accounts(uuid)\
);";


	ExecStatusType status;
	char* error_msg;

	if(pgdb_transaction_begin(conn)) {
		DEBUG("Failed to begin transaciton.\n");
		return 1;
	}

	if(pgdb_execute(conn, drop_blacklist_accesses_table)) {
		DEBUG("Failed to drop blacklist access table.\n");
		pgdb_transaction_rollback(conn);
		return 1;
	}

	if(pgdb_execute(conn, drop_blacklist_table)) {
		DEBUG("Failed to drop blacklist table.\n");
		pgdb_transaction_rollback(conn);
		return 1;
	}

	if(pgdb_execute(conn, drop_sessions_accesses_table)) {
		DEBUG("Failed to drop session access table.\n");
		pgdb_transaction_rollback(conn);
		return 1;
	}

	if(pgdb_execute(conn, drop_sessions_table)) {
		DEBUG("Failed to drop session table.\n");
		pgdb_transaction_rollback(conn);
		return 1;
	}

	if(pgdb_execute(conn, drop_tokens_table)) {
		DEBUG("Failed to drop tokens table.\n");
		pgdb_transaction_rollback(conn);
		return 1;
	}

	if(pgdb_execute(conn, drop_token_type_enum)) {
		DEBUG("Failed to drop tokens type enum.\n");
		pgdb_transaction_rollback(conn);
		return 1;
	}

	if(pgdb_execute(conn, drop_accounts_table)) {
		DEBUG("Failed to drop accounts table.\n");
		pgdb_transaction_rollback(conn);
		return 1;
	}

	if(pgdb_execute(conn, drop_accounts_role_enum)) {
		DEBUG("Failed to drop accounts role enum.\n");
		pgdb_transaction_rollback(conn);
		return 1;
	}

	if(pgdb_execute(conn, accounts_role_enum)) {
		DEBUG("Failed create account role enum.\n");
		pgdb_transaction_rollback(conn);
		return 1;
	}
		
	if(pgdb_execute(conn, accounts_table)) {
		DEBUG("Failed create accounts table.\n");
		pgdb_transaction_rollback(conn);
		return 1;
	}

	if(pgdb_execute(conn, tokens_type_enum)) {
		DEBUG("Failed create tokens type enum.\n");
		pgdb_transaction_rollback(conn);
		return 1;
	}

	if(pgdb_execute(conn, tokens_table)) {
		DEBUG("Failed create tokens table.\n");
		pgdb_transaction_rollback(conn);
		return 1;
	}

	if(pgdb_execute(conn, sessions_table)) {
		DEBUG("Failed create session table.\n");
		pgdb_transaction_rollback(conn);
		return 1;
	}

	if(pgdb_execute(conn, sessions_accesses_table)) {
		DEBUG("Failed create session access table.\n");
		pgdb_transaction_rollback(conn);
		return 1;
	}

	if(pgdb_execute(conn, blacklist_table)) {
		DEBUG("Failed create blacklist table.\n");
		pgdb_transaction_rollback(conn);
		return 1;
	}

	if(pgdb_execute(conn, blacklist_accesses_table)) {
		DEBUG("Failed create blacklist access table.\n");
		pgdb_transaction_rollback(conn);
		return 1;
	}

	if(pgdb_transaction_commit(conn)) {
		DEBUG("Failed to commit transaciton.\n");
		pgdb_transaction_rollback(conn);
		return 1;
	}
	
	return 0;
}
