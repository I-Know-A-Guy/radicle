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

void string_free(string_t** str) {
	if(*str == NULL)
	       	return;
	free((*str)->ptr);
	free(*str);
	*str = NULL;
}
