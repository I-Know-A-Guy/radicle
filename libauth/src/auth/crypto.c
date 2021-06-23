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

#include <stdbool.h>
#include <string.h> 

#include <openssl/aes.h>
#include <openssl/hmac.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/rand.h>

#include <argon2.h>

#include "radicle/print.h"
#include "radicle/auth/crypto.h"

int base64_encode(const unsigned char* input, size_t length, string_t** buffer) {
	BIO* b64 = BIO_new(BIO_f_base64());
	if(b64 == NULL) {
		ERROR("Failed to use BIO_new for b64.\n");
		return 1;
	}
	BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
	BIO* bmem = BIO_new(BIO_s_mem());
	if(bmem == NULL) {
		BIO_free(b64);
		ERROR("Failed to use BIO_new for bmem.\n");
		return 1;
	}
	b64 = BIO_push(b64, bmem);
	size_t bytes_written = BIO_write(b64, input, length);
	if(bytes_written < 1) {
		/* no data was successfully read or written if the result is 0 or -1 */
		BIO_free_all(b64);
		ERROR("BIO_write failed to write to b64. Returned %ld\n", bytes_written);
		return 1;
	}
	if(BIO_flush(b64) < 1) {
		BIO_free_all(b64);
		ERROR("Failed to use BIO_flush for b64.\n");
		return 1;
	}
	BUF_MEM* bptr = NULL;
	BIO_get_mem_ptr(b64, &bptr);
	*buffer = string_new(bptr->data, bptr->length);
	BIO_free_all(b64);
	return 0;
}

int base64_decode(const string_t* base64, string_t** buffer) {
	BIO* b64 = BIO_new(BIO_f_base64());
	if(b64 == NULL) {
		ERROR("Failed to use BIO_new for b64.\n");
		return 1;
	}
	BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
	BIO* bio = BIO_new_mem_buf(base64->ptr, base64->length);
	bio = BIO_push(b64, bio);
	*buffer = calloc(1, sizeof(string_t));
	(*buffer)->ptr  = calloc(base64->length + 1, sizeof(char));
	(*buffer)->length = BIO_read(bio, (*buffer)->ptr, base64->length);
	if((*buffer)->length < 1) {
		/* no data was successfully read or written if the result is 0 or -1 */
		BIO_free_all(bio);
		ERROR("BIO_read did not read successfully. Returned %ld.\n", (*buffer)->length);
		string_free(buffer);
		return 1;
	}
	BIO_free_all(bio);
	return 0;
}

int hmac_sign(const unsigned char* input, const size_t input_length, const string_t* key, string_t** buffer) {
	unsigned char hmac_buffer[EVP_MAX_MD_SIZE];
	unsigned int hmac_buffer_length;
	if(HMAC(EVP_sha512(), key->ptr, key->length, input, input_length, hmac_buffer, &hmac_buffer_length) != hmac_buffer) {
		ERROR("Failed to sign using hmac.\n");
		return 1;
	}
	
	*buffer = string_new_empty(hmac_buffer_length * 2);
	char* iter = (*buffer)->ptr;
	for(int i = 0; i < hmac_buffer_length; i++) {
		iter += sprintf(iter, "%02x", hmac_buffer[i]);
	}
	return 0;
}

int hmac_verify(const string_t* key, const string_t* signature, const string_t* input) {
	string_t* buffer = NULL;
	if(hmac_sign((unsigned char*)input->ptr, input->length, key, &buffer)) {
		return 1;
	}

	if(signature->length != buffer->length || strcmp(buffer->ptr, signature->ptr) != 0) {
		string_free(&buffer);
		return 1;
	}
	string_free(&buffer);
	return 0;
}

int hmac_verify_salted(const string_t* key, const string_t* salt, const string_t* signature, const string_t* input) {
	string_t* final_key = string_new_empty(key->length + salt->length);
	strncpy(final_key->ptr, key->ptr, key->length);
	strncat(final_key->ptr, salt->ptr, salt->length);
	int res = hmac_verify(final_key, signature, input);
	string_free(&final_key);
	return res;
}

int auth_hash_password(const string_t* password, string_t** buffer) {
	string_t* salt;
	if(auth_generate_random_base64(16, &salt)) {
		ERROR("Failed to generate salt for password hashing.\n");
		return 1;
	}
	uint32_t t_cost = 2;            // 2-pass computation
    	uint32_t m_cost = (1<<16);      // 64 mebibytes memory usage
    	uint32_t parallelism = 2;       // number of threads and lanes
	uint32_t hash_length = 32;
	*buffer = string_new_empty(256);
	int error = argon2i_hash_encoded(t_cost, m_cost, parallelism, password->ptr, password->length, salt->ptr, salt->length, hash_length, (*buffer)->ptr, (*buffer)->length);
	if(error != ARGON2_OK) {
		ERROR("Failed to hash password. %s\n", argon2_error_message(error));
		string_free(&salt);
		string_free(buffer);
		return 1;
	}
	// because string_new_empty sets size to 256, it has to be corrected
	(*buffer)->length = strnlen((*buffer)->ptr, (*buffer)->length);

	string_free(&salt);
	return 0;
}

int auth_verify_password(const string_t* encoded, const string_t* password) {
	int error = argon2i_verify(encoded->ptr, password->ptr, password->length);
	if(error != ARGON2_OK) {
		ERROR("Failed to verify password. %s\n", argon2_error_message(error));
		return 1;
	}
	return 0;
}

int auth_generate_random_base64(const int rand_bytes, string_t** buffer) {
	unsigned char rand_buffer[rand_bytes];
	if(RAND_bytes(rand_buffer, rand_bytes) != 1) {
		unsigned int error_code = ERR_get_error();
		if(error_code != 0) {
			char error_buffer[1024];
			ERR_error_string_n(error_code, error_buffer, 1024);
			ERROR("%s\n", error_buffer);
		} else {
			ERROR("RAND_bytes reported an error but ERR_get_error does not return any error information.\n");	
		}
		return 1;
	}
	if(base64_encode(rand_buffer, rand_bytes, buffer)) {
		ERROR("Failed to encode rand_buffer to base64.\n");
		return 1;
	}
	return 0;
}

int auth_generate_session_cookie(const string_t* key, auth_cookie_t** cookie) {

	*cookie = auth_cookie_new_empty();

	if(auth_generate_random_base64(SESSION_ID_LENGTH, &(*cookie)->token)) {
		auth_cookie_free(cookie);
		ERROR("Failed to generate session token.\n");
		return 1;
	}

	if(auth_generate_random_base64(SESSION_ID_LENGTH, &(*cookie)->salt)) {
		auth_cookie_free(cookie);
		ERROR("Failed to generate salt for session id.\n");
		return 1;
	}

	string_t* final_key = string_new_empty(key->length + (*cookie)->salt->length);
	strncpy(final_key->ptr, key->ptr, key->length);
	strncat(final_key->ptr, (*cookie)->salt->ptr, (*cookie)->salt->length);

	if(hmac_sign((unsigned char*)(*cookie)->token->ptr, (*cookie)->token->length, key, &(*cookie)->signature)) {
		string_free(&final_key);
		auth_cookie_free(cookie);
		ERROR("Failed to sign session id.");
		return 1;
	}

	string_free(&final_key);

	(*cookie)->cookie = string_new_empty((*cookie)->token->length + (*cookie)->signature->length + 1);
	sprintf((*cookie)->cookie->ptr, "%s-%s", (*cookie)->token->ptr, (*cookie)->signature->ptr);
	
	return 0;
}

int auth_split_cookie(const string_t* cookie,  auth_cookie_t** result) {
	if(cookie->length < SESSION_ID_LENGTH + HMAC_LENGTH + 1) {
		ERROR("Cookie size is invalid. Minimum length must be size of random session bytes + HMAC 512 length (128 bytes) + delimitier char.\n");
		return 1;
	}
	
	char* delimiter = strchr(cookie->ptr, '-');
	if(delimiter == NULL) {
		ERROR("Cookie does not contain a delimiter.\n");
		return 1;
	}

	*result = auth_cookie_new_empty();
	(*result)->token = string_new_empty(delimiter - cookie->ptr);
	if((*result)->token->length < SESSION_ID_LENGTH) {
		ERROR("Session id length is too short. Must be atleast of %d length. Is %ld.\n", SESSION_ID_LENGTH, (*result)->token->length);
		auth_cookie_free(result);
		return 1;
	}

	(*result)->signature = string_new_empty(cookie->length - (*result)->token->length - 1);
	if((*result)->signature->length != HMAC_LENGTH) {
		ERROR("Signature must be of size %d not %ld.\n", HMAC_LENGTH, (*result)->signature->length);
		auth_cookie_free(result);
		return 1;
	}

	strncpy((*result)->token->ptr, cookie->ptr, (*result)->token->length);
	strncpy((*result)->signature->ptr, delimiter + 1, (*result)->signature->length);

	return 0;
}
