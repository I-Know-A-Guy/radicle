#include <jansson.h>

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

#include "radicle/print.h"
#include "radicle/types/string.h"

#include "radicle/api/json_validate.h"

int validate_regex(const char* exp, const int capture_groups, const char* input) {
	PCRE2_SIZE error_offset;
	int ec;

	pcre2_code* re = pcre2_compile((PCRE2_SPTR8)exp, -1, 0, &ec, &error_offset, NULL);
	if(re == NULL) {
		PCRE2_UCHAR8 buffer[120];
		pcre2_get_error_message(ec, buffer, 120);
		ERROR("Failed to compile %s with error message %s.\n", exp, buffer);
		return 1;
	}
	pcre2_match_data* match_data = pcre2_match_data_create(capture_groups, NULL);
	ec = pcre2_match(re, (PCRE2_SPTR8)input, -1, 0, 0, match_data, NULL);

	if(ec == 0) {
		ERROR("Vector of offsets is too small. (Increase pcre2_match_data_create\n");
	} else if (ec < 0 && ec != PCRE2_ERROR_NOMATCH) {
		PCRE2_UCHAR8 buffer[120];
		pcre2_get_error_message(ec, buffer, 120);
		ERROR("Failed to match expression %s with error %s.\n", exp, buffer);
	}

	pcre2_match_data_free(match_data);
	pcre2_code_free(re);

	return !(ec >= 1);
}

/**
 * @brief Checks if key is in object, verifies typei s string and then runs
 * regex on it.
 *
 * @param object JSON object
 * @param key Key of value to extract from object
 * @param regex Regex to check on value
 * @param result If regex was valid, value will be stored in result
 * 
 * @result Returns 0 on success
 */
int api_json_validate_string(const json_t* object, const char* key, const char* regex, const int capture_groups, string_t** result) {
	json_t* child = json_object_get(object, key);
	if(child == NULL || child->type != JSON_STRING) {
		*result = NULL;
		return 1;
	}
	const char* value = json_string_value(child);
	
	if(validate_regex(regex, capture_groups, value)) {
		*result = NULL;
		return 1;
	}

	*result = string_new(value, json_string_length(child));
	return 0;

}

int api_json_validate_text(const json_t* object, const char* key, const int minLength, const int maxLength, string_t** result) {
	char buffer[50];
	sprintf(buffer, "^.{%d,%d}$", minLength, maxLength);
	return api_json_validate_string(object, key, buffer, 0, result);
}

int api_json_validate_email(const json_t* object, const char* key, string_t** result) {
	return api_json_validate_string(object, key, "^[^\\s@]+@([^\\s@.,]+\\.)+[^\\s@.,]{2,}$", 2, result);
}

int api_json_validate_password(const json_t* object, const char* key, string_t** result) {
	return api_json_validate_string(object, key, "^(?=.*?[A-Z])(?=.*?[a-z])(?=.*?[0-9])(?=.*?[#?!@$ %^&*-]).{8,}$", 4, result);
}
