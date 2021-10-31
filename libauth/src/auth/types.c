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

int auth_account_role_from_str(const char* role) {
	if(strcmp("user", role) == 0) {
		return ROLE_USER;
	} else if(strcmp("admin", role) == 0) {
		return ROLE_ADMIN;
	}
	return ROLE_NONE;
}

const char* auth_account_role_to_str(auth_account_role_t role) {
	switch(role) {
		case ROLE_USER:
			return "user";
		case ROLE_ADMIN:
			return "admin";
		default:
			return "none";
	}
}

auth_account_t* auth_account_new(uuid_t* uuid, string_t* email, string_t* password, auth_account_role_t role, bool active, bool verified, time_t created) {

	auth_account_t* acc = calloc(1, sizeof(auth_account_t));

	acc->uuid = uuid_copy(uuid); 
	acc->email = string_copy(email);
	acc->password = string_copy(password);
	acc->role = role;
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
	free(*account);
	*account = NULL;
}

auth_request_log_t* auth_request_log_new() {
	auth_request_log_t* rq = calloc(1, sizeof(auth_request_log_t));
	clock_gettime(CLOCK_MONOTONIC, &rq->timer);
	return rq;
}

void auth_request_log_calculate_response_time(auth_request_log_t* request_log) {
	struct timespec now = {0, 0};
	clock_gettime(CLOCK_MONOTONIC, &now);
	request_log->response_time = (((double)now.tv_sec + 1.0e-9*now.tv_nsec) -
	       	((double)request_log->timer.tv_sec + 1.0e-9*request_log->timer.tv_nsec)) * 1000000.0;
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

int token_type_from_str(const char* type) {
	if(strcmp(type, "registration") == 0) {
		return REGISTRATION;
	} else if(strcmp(type, "password_reset") == 0) {
		return PASSWORD_RESET;
	} else if(strcmp(type, "change_email") == 0) {
		return CHANGE_EMAIL;
	}
	return -1;
}

const char* token_type_to_str(token_type_t type) {
	switch(type) {
		case REGISTRATION:
			return "registration";
		case PASSWORD_RESET:
			return "password_reset";
		case CHANGE_EMAIL:
			return "change_email";
		default:
			return "none";
	}
}

void auth_session_access_entry_free(void* ptr) {
	if(ptr == NULL) return;
	auth_session_access_entry_t* entry = (auth_session_access_entry_t*)ptr;
	uuid_free(&entry->owner);
	free(ptr);
}

int file_type_from_str(const char* type) {
	if(strcmp(type, "image/jpeg") == 0 || strcmp(type, "image/jpg") == 0) {
		return FILE_TYPE_IMAGE_JPEG;
	} else if(strcmp(type, "image/png") == 0) {
		return FILE_TYPE_IMAGE_PNG;
	}
	return FILE_TYPE_UNKNOWN;
}

const char* file_type_to_str(file_type_t type) {
	switch(type) {
		case FILE_TYPE_IMAGE_JPEG:
			return "image/jpeg";
		case FILE_TYPE_IMAGE_PNG:
			return "image/png";
		default:
			return NULL;
	}
}

void auth_file_free(auth_file_t** file) {
	if(*file == NULL) return;
	uuid_free(&(*file)->uuid);
	uuid_free(&(*file)->owner);
	string_free(&(*file)->path);
	string_free(&(*file)->name);
	free(*file);
	*file = NULL;
}

