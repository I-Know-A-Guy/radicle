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

#include "radicle/auth/types.h"
#include "radicle/print.h"

auth_account_t* auth_account_new(uuid_t* uuid, string_t* email, string_t* password, string_t* role, bool active, bool verified, time_t created) {

	auth_account_t* acc = calloc(1, sizeof(auth_account_t));

	acc->uuid = uuid_copy(uuid); 
	acc->email = string_copy(email);
	acc->password = string_copy(password);
	acc->role = string_copy(role);
	acc->active = active;
	acc->verified = verified;
	acc->created = created;

	return acc;
}


void auth_account_free(auth_account_t** account) {
	if(*account == NULL) return;
	free((*account)->uuid);
	string_free(&(*account)->email);
	string_free(&(*account)->password);
	string_free(&(*account)->role);
	free(*account);
	*account = NULL;
}

auth_requester_t* auth_requester_new(const string_t* ip, const string_t* path) {
	auth_requester_t* rq = calloc(1, sizeof(auth_requester_t));

	rq->ip = string_copy(ip);
	rq->path = string_copy(path);
	
	return rq;
}

auth_requester_t* auth_requester_newl(const char* ip, const char* path) {
	auth_requester_t* rq = calloc(1, sizeof(auth_requester_t));

	rq->ip = string_new(ip, strlen(ip));
	rq->path= string_new(path, strlen(path));

	return rq;
}

void auth_requester_free(auth_requester_t** requester) {
	if(*requester == NULL) return;
	string_free(&(*requester)->ip);
	string_free(&(*requester)->path);
	free(*requester);
	*requester = NULL;
}

auth_cookie_t* auth_cookie_new_empty() {
	return calloc(1, sizeof(auth_cookie_t));
}

void auth_cookie_free(auth_cookie_t** cookie) {
	if(*cookie == NULL) return;
	string_free(&(*cookie)->cookie);
	string_free(&(*cookie)->token);
	string_free(&(*cookie)->signature);
	string_free(&(*cookie)->salt);
	free(*cookie);
	*cookie = NULL;
}
