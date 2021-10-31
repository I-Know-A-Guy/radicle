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
 * @brief All endpoints associated with authentication.
 * @addtogroup libapi 
 * @{
 * @addtogroup libapi_endpoints Endpoints 
 * @{
 */

#ifndef RADICLE_LIBAPI_INCLUDE_RADICLE_API_ENDPOINTS_AUTH_H
#define RADICLE_LIBAPI_INCLUDE_RADICLE_API_ENDPOINTS_AUTH_H

#include <ulfius.h>

#include "radicle/pgdb.h"

#if defined(__cplusplus)
extern "C" {
#endif


/**
 * @brief Registers user to database.
 */
int api_auth_callback_register(const struct _u_request * request, struct _u_response * response, void * user_data);

/**
 * @brief Resend mail with email verification code.
 */
int api_auth_callback_resend_verification_mail(const struct _u_request * request, struct _u_response * response, void * user_data);

/**
 * @brief Verifies email code
 */
int api_auth_callback_register_verify(const struct _u_request * request, struct _u_response * response, void * user_data);

/**
 * @brief Signs in user using credentials and responds with a cookie
 *
 * @todo Check if cookie needs to be forever or only session 
 */
int api_auth_callback_sign_in(const struct _u_request * request, struct _u_response * response, void * user_data);

/**
 * @brief Sends password reset code.
 */
int api_auth_callback_send_password_reset(const struct _u_request * request, struct _u_response * response, void * user_data);

/**
 * @brief Resets password with code.
 */
int api_auth_callback_reset_password(const struct _u_request * request, struct _u_response * response, void * user_data);

/**
 * @brief Returns object containing email and verified status.
 */
int api_auth_callback_cookie_info(const struct _u_request * request, struct _u_response * response, void * user_data);

/**
 * @brief Sends email to new mail for verification
 */
int api_auth_callback_send_new_email_verification(const struct _u_request * request, struct _u_response * response, void * user_data);

/**
 * @brief Verifies email change token
 */
int api_auth_callback_verify_new_email(const struct _u_request * request, struct _u_response * response, void * user_data);

/**
 * @brief Middle callback for file upload, endpont->file_upload must have
 * allready been init
 */
int api_auth_callback_upload_file(const struct _u_request * request, struct _u_response * response, void * user_data);

#if defined(__cplusplus)
}
#endif

#endif // RADICLE_LIBAPI_INCLUDE_RADICLE_API_ENDPOINTS_AUTH_H

/** @} */
/** @} */
