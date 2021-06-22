/**
 * @file
 * @brief Contains all auth functions related to crypto non database calls.
 * @author Nils Egger
 * @addtogroup Auth
 * @{
 */

#ifndef RADICLE_AUTH_INCLUDE_RADICLE_AUTH_CRYPTO_H 
#define RADICLE_AUTH_INCLUDE_RADICLE_AUTH_CRYPTO_H 

#include <stddef.h>

#include "radicle/types/string.h"
#include "radicle/auth/types.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Length which all session tokens must atleast have.
 */
#define SESSION_ID_LENGTH 128

/**
 * @brief Length of resulting HMAC-sha512 hash.
 */
#define HMAC_LENGTH 128 // sha512 results in a hex string of size 128

/**
 * @brief Generates a \ref auth_cookie_t with the given key.
 *
 * @param key Key which will be used for the hmac-sha512 hash.
 * @param cookie Pointer to resulting \ref auth_cookie_t.
 *
 * @return Returns 0 on success.
 */
int auth_generate_session_cookie(const string_t* key, auth_cookie_t** cookie);

/**
 * @brief Splits a cookie string into its token and signature part and creates a new \ref auth_cookie_t.
 *
 * @param cookie Raw cookie string to split.
 * @param result Pointer to unpacked \ref auth_cookie_t.
 *
 * @returns Returns 0 on success.
 */
int auth_split_cookie(const string_t* cookie,  auth_cookie_t** result);

/**
 * Generates a secure random session id and converts it to base64.
 *
 * @param rand_bytes Length to use for generating random bytes. buffer will not be of this length, due to the convertion to base64.
 * @param buffer Double pointer to char buffer, resulting base64 char array will be stored here.
 * @param buffer_length Length of buffer base64 string. 
 *
 * @returns 0 on success
 */
int auth_generate_random_base64(const int rand_bytes, string_t** buffer);

/**
 * @brief Creates a hash using Argon2i.
 *
 * @param password Raw password string.
 * @param buffer Pointer to resulting hashed password, encoded alongside argon2 parameters. 
 *
 * @returns Returns 0 on success.
 */
int auth_hash_password(const string_t* password, string_t** buffer);

/**
 * @brief Verifies if password matches prehashed password.
 *
 * @param encoded Encoded argon2 password string.
 * @param password Raw password to validate.
 *
 * @returns Returns 0 on success.
 */
int auth_verify_password(const string_t* encoded, const string_t* password);

/**
 * @brief Takes a string as input and converts it to base64.
 *
 * @param input Input string or bytes.
 * @param length Length of input.
 * @param buffer Double pointer to buffer to write to.
 *
 * @returns Returns 0 on success.
 */
int base64_encode(const unsigned char* input, size_t length, string_t** buffer);

/**
 * @brief Takes a base64 as input and converts it back to its original bytes.
 *
 * @param input Input base64 string.
 * @param buffer Double pointer to buffer to write to.
 * 
 * @returns Returns 0 on success.
 */
int base64_decode(const string_t* base64, string_t** buffer);

/**
 * @brief Signs an input bytes array with the given key using hmac-sha512.
 *
 * @param input Bytes to sign
 * @param input_length Length of input.
 * @param key Key to use for signing.
 * @param buffer Double pointer to buffer to write to.
 *
 * @returns Returns 0 on success.
 */
int hmac_sign(const unsigned char* input, const size_t input_length, const string_t* key, string_t** buffer);

#if defined(__cplusplus)
}
#endif

#endif // RADICLE_AUTH_INCLUDE_RADICLE_AUTH_CRYPTO_H 

/** @} */
