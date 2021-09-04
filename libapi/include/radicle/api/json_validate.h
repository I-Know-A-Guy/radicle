/**
 * @file
 * @brief Functions used for validating json input from requests.
 * @author Nils Egger
 * @addtogroup tools 
 * @{
 */

#ifndef IKAG_INCLUDE_IKAG_JSON_VALIDATE_H
#define IKAG_INCLUDE_IKAG_JSON_VALIDATE_H

#include <jansson.h>
#include "radicle/types/string.h"

#if defined(__cplusplus)
extern "C" {
#endif


/**
 * @brief Validates that key is given in object and checks that minLength and maxLength are correct.
 *
 * @param object JSON object.
 * @param key Key of value to check.
 * @param minLength Minimum length string must have.
 * @param maxLength Maximumg length string is allowed to have.
 * @param result String value will be copies to this parameter if valid.
 *
 * @return Return 0 for success.
 */
int api_json_validate_text(const json_t* object, const char* key, const int minLength, const int maxLength, string_t** result);

/**
 * @brief Validates that key is given in object and checks that email is valid.
 *
 * @param object JSON object.
 * @param key Key of value to check.
 * @param result String value will be copies to this parameter if valid.
 *
 * @return Return 0 for success.
 */
int api_json_validate_email(const json_t* object, const char* key, string_t** result);

/**
 * @brief Validates that key is given in object and checks that password meets
 * required complexety.
 *
 * @param object JSON object.
 * @param key Key of value to check.
 * @param result String value will be copies to this parameter if valid.
 *
 * @return Return 0 for success.
 */
int api_json_validate_password(const json_t* object, const char* key, string_t** result);


#if defined(__cplusplus)
}
#endif

#endif // IKAG_INCLUDE_IKAG_JSON_VALIDATE_H

/** @} */

