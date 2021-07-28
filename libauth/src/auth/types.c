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

auth_request_log_t* auth_request_log_new() {
	auth_request_log_t* rq = calloc(1, sizeof(auth_request_log_t));
	return rq;
}

auth_request_log_t* auth_request_log_newl(const char* ip, const char* url) {
	auth_request_log_t* rq = calloc(1, sizeof(auth_request_log_t));

	rq->ip = string_new(ip, strlen(ip));
	rq->url= string_new(url, strlen(url));

	return rq;
}

void auth_request_log_free(auth_request_log_t** requester) {
	if(*requester == NULL) return;
	string_free(&(*requester)->ip);
	string_free(&(*requester)->url);
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

auth_session_t* auth_session_new() {
	auth_session_t* buf = calloc(1, sizeof(auth_session_t));
	return buf;
}

void auth_session_free(auth_session_t** session) {
	if(*session == NULL) return;
	string_free(&(*session)->salt);
	free(*session);
	*session = NULL;
}
