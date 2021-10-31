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
 *
 * @todo add function for checking if string is same
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
 * @brief Creates a new string and copies value of first then seconds
 *
 * @param first Beginning string
 * @param second Tailing string
 *
 * @return Returns new string
 */
string_t* string_cat(const string_t* first, const string_t* second);

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
