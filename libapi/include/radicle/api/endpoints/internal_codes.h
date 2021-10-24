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
 * @brief Contains Enum of all Internal Codes
 *
 * @addtogroup libapi 
 * @addtogroup libapi_endpoints
 * @{
 */

#ifndef INCLUDE_IKAG_ENDPOINTS_INTERNAL_CODES_H
#define INCLUDE_IKAG_ENDPOINTS_INTERNAL_CODES_H

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum internal_errors {
	SUCCESS = 0,
	NOT_FOUND,
	OPTION_RESPONSE,

	VALIDATION_ERRORS = 100,
	VALIDATION_INVALID_EMAIL,
	VALIDATION_INVALID_PASSWORD,
	VALIDATION_INVALID_COOKIE,
	VALIDATION_NOT_AUTHENTICATED,
	VALIDATION_MISSING_PARAMETER,
	VALIDATION_EMAIL_NOT_VERIFIED,
	VALIDATION_EMAIL_ALREADY_VERIFIED,
	VALIDATION_MISSING_JSON_BODY,
	VALIDATION_ACCOUNT_DEACTIVATED,
	VALIDATION_UNAUTHORIZED,
	VALIDATION_MISSING_RESET_TOKEN,

	PGDB = 1000,
	PGDB_UNABLE_TO_CLAIM,
	ERROR_TRANSACTION_BEGIN,
	ERROR_TRANSACTION_COMMIT,
	ERROR_TRANSACTION_ROLLBACK,

	AUTH = 2000,
	SUCCESS_REGISTER,
	SUCCESS_CREDENTIALS_SIGN_IN,
	SUCCESS_COOKIE_SIGN_IN,
	SUCCESS_VERIFY_REGISTRATION,
	ERROR_OWNED_SESSION,
	ERROR_EMAIL_LOOKUP,
	ERROR_REGISTER,
	ERROR_CREATING_TOKEN,
	ERROR_VERIFYING_TOKEN,
	INVALID_TOKEN_TYPE,
	ERROR_REVOKE_REGISTRATIONS_TOKEN,
	ERROR_SEND_MAIL_DUPLICATE_REGISTER_NOTIFY,
	ERROR_SEND_MAIL_REGISTER_TOKEN,
	INVALID_REGISTER_TOKEN,
	DUPLICATE_EMAIL_REGISTER,
	INVALID_USER_CREDENTIALS,
	FAILED_TO_SEND_MAIL,
	PASSWORD_RESET_EMAIL_DOESNT_EXIST,
	ERROR_UPDATING_PASSWORD,
	ERROR_UPDATING_ACCOUNT_EMAIL,
	ERROR_BLACKLIST_LOOKUP,
	ERROR_SESSION_ACCESS_LOOKUP,
	FORBIDDEN_BLACKLIST_IP,
	ERROR_SAVING_BLACKLIST,
	ERROR_SAVE_BLACKLIST_ACCESS
} internal_errors_t;

const char* internal_errors_msg(internal_errors_t code);

/**
 * @todo create, then change RESPOND to use this as text and then test if gotos
 * with cleanup would improve returns!
 */
const char* public_internal_errors_msg(internal_errors_t code);

#if defined(__cplusplus)
}
#endif

#endif // INCLUDE_IKAG_ENDPOINTS_INTERNAL_CODES_H

/** @} */
