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

#include "radicle/types/string.h"
#include "radicle/print.h"

string_t* string_new(const char* str, const size_t length) {
	string_t* buf = calloc(1, sizeof(string_t));
	if(length == 0 || str == NULL) {
		buf->length = 0;
		return buf;
	}
	buf->length = length;
	buf->ptr = calloc(length + 1, sizeof(char));
	memcpy(buf->ptr, str, length);
	buf->ptr[length] = 0; // this one shouldnt be necessary
	return buf;
}

string_t* string_from_literal(const char* literal) {
	string_t* buf = string_new_empty(strlen(literal));
	strcpy(buf->ptr, literal);
	return buf;
}

string_t* string_copy(const string_t* string) {
	if(string == NULL) return NULL;
	string_t* buf = string_new_empty(string->length);
	strncpy(buf->ptr, string->ptr, string->length);
	return buf;
}

string_t* string_new_empty(const size_t length) {
	string_t* buf = calloc(1, sizeof(string_t));
	if(length == 0) {
		buf->length = 0;
		return buf;
	}
	buf->length = length;
	buf->ptr = calloc(length + 1, sizeof(char));
	return buf;
}

string_t* string_cat(const string_t* first, const string_t* second) {
	string_t* res = string_new_empty(first->length + second->length);
	memcpy(res->ptr, first->ptr, first->length);
	memcpy(res->ptr + first->length, second->ptr, second->length);
	return res;
}

void string_free(string_t** str) {
	if(*str == NULL)
	       	return;
	free((*str)->ptr);
	free(*str);
	*str = NULL;
}
