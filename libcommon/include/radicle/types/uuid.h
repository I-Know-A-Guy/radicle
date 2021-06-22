/**
 * @file 
 * @brief Header file for \ref uuid_t.
 * @addtogroup Common 
 * @{
 * @addtogroup Types
 * @{
 */

#ifndef RADICLE_COMMON_INCLUDE_RADICLE_TYPES_UUID_H
#define RADICLE_COMMON_INCLUDE_RADICLE_TYPES_UUID_H

#include "string.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Struct which contains binary representation of a uuid.
 *
 * @see uuid_to_str()
 */
typedef struct uuid {
	unsigned char bin[16];/**< Binary representation. */
} uuid_t;

/**
 * @brief Creates a new uuid from binary.
 *
 * @param bin UUID binary.
 *
 * @returns Returns a new pointer to a \ref uuid_t object containing the copied binary uuid.
 */
uuid_t* uuid_new(const unsigned char* bin);

/**
 * @brief Frees the uuid and sets its pointer to NULL
 *
 * @brief uuid Pointer to \ref uuid_t.
 *
 * @returns Returns void.
 */
void uuid_free(uuid_t** uuid);

/**
 * @brief Converts a uuid to textual representation.
 *
 * @param uuid Binary uuid to convert.
 *
 * @returns Returns new pointer to \ref string_t containing the textual representation of the \p uuid.
 */
string_t* uuid_to_str(const uuid_t* uuid);

/**
 * @brief Creates a new copy of an existing \ref uuid_t.
 *
 * @param uuid UUID to copy.
 *
 * @returns Returns a new pointer to \ref uuid_t.
 */
uuid_t* uuid_copy(uuid_t* uuid);

#if defined(__cplusplus)
}
#endif

#endif // RADICLE_COMMON_INCLUDE_RADICLE_TYPES_UUID_H

/** @} */
/** @} */

