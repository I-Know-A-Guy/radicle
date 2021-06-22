/**
 * @file
 * @brief Header file containing all signatures for methods associated with the string_t struct.
 * @author Nils Egger
 *
 * @addtogroup Common 
 * @{
 * @addtogroup Types 
 * @{
 * @addtogroup String 
 * @{
 */

#ifndef RADICLE_COMMON_INCLUDE_RADICLE_TYPES_STRING_H 
#define RADICLE_COMMON_INCLUDE_RADICLE_TYPES_STRING_H 

#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief String representation with fixed size. 
 *
 * @see string_new()
 * @see string_new_empty()
 * @see string_from_literal()
 * @see string_copy()
 * @see string_free()
 */
typedef struct string {
	char* ptr; /**< Pointer to char array. */
	size_t length;/**< Length of ptr. */
} string_t;

/**
 * @brief Creates a new string object and copies \p length from \p str to \ref string_t.ptr.
 *
 * @param str Char array to copy.
 * @param length Length of char array to copy. 
 *
 * @returns Returns pointer to \ref string_t.
 */
string_t* string_new(const char* str, const size_t length);

/**
 * @brief Creates a new empty \ref string_t of \p length.
 *
 * @param length Length of new string.
 *
 * @returns Returns pointer to \ref string_t.
 */
string_t* string_new_empty(const size_t length);

/**
 * @brief Creates a new \ref string_t from a C literal.
 *
 * @param literal Literal to copy to \ref string_t.ptr.
 *
 * @returns Returns pointer to \ref string_t.
 */
string_t* string_from_literal(const char* literal);

/**
 * @brief Copies data from a \ref string_t to a new one and returns the pointer to the copy.
 *
 * @param string String to copy.
 *
 * @returns Returns pointer to \ref string_t.
 */
string_t* string_copy(const string_t* string);

/**
 * @brief Frees \t str and sets the pointer to NULL.
 * 
 * @param str Pointer \ref string_t object to free. 
 *
 * @returns Returns void.
 */
void string_free(string_t** str);

#if defined(__cplusplus)
}
#endif

#endif // RADICLE_COMMON_INCLUDE_RADICLE_TYPES_STRING_H 

/** @} */
/** @} */
/** @} */
