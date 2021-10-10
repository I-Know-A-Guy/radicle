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
#include <string.h>

#include "radicle/types/uuid.h"

uuid_t* uuid_new(const unsigned char* bin) {
	uuid_t* buf = calloc(1, sizeof(uuid_t));
	memcpy(buf->bin, bin, 16);
	return buf;
}

void uuid_free(uuid_t** uuid) {
	if(*uuid == NULL) return;
	free(*uuid);
	*uuid = NULL;
}

string_t* uuid_to_str(const uuid_t* uuid) {
	string_t* buf = string_new_empty(36);
	char* iter = buf->ptr;
	int pos = 0, i = 0;
	while(pos < 36) {
		switch(pos) {
			case 8:
			case 13:
			case 18:
			case 23: {
				*iter = '-';
				iter++;
				pos++;
				break;
			}
			default: {
				iter += sprintf(iter, "%02x", uuid->bin[i]);
				pos += 2;
				i++;
				break;
			}
		}
	}
	return buf;
}

uuid_t* uuid_copy(uuid_t* uuid) {
	if(uuid == NULL) return NULL;
	uuid_t* buf = calloc(1, sizeof(uuid_t));
	memcpy(buf->bin, uuid->bin, 16);
	return buf;
}
