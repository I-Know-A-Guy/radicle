/**
 * @file
 * @brief All endpoints associated with authentication.
 * @author Nils Egger
 * @addtogroup endpoints 
 * @{
 */

#ifndef IKAG_INCLUDE_IKAG_ENDPOINTS_AUTH_H
#define IKAG_INCLUDE_IKAG_ENDPOINTS_AUTH_H

#include <ulfius.h>

#include "radicle/pgdb.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define EMAIL_ALREADY_VERIFIED "Your email has already been verified."

int api_auth_callback_register(const struct _u_request * request, struct _u_response * response, void * user_data);

int api_auth_callback_resend_verification_mail(const struct _u_request * request, struct _u_response * response, void * user_data);

int api_auth_callback_register_verify(const struct _u_request * request, struct _u_response * response, void * user_data);

int api_auth_callback_sign_in(const struct _u_request * request, struct _u_response * response, void * user_data);

int api_auth_callback_send_password_reset(const struct _u_request * request, struct _u_response * response, void * user_data);

int api_auth_callback_reset_password(const struct _u_request * request, struct _u_response * response, void * user_data);

int api_auth_callback_cookie_info(const struct _u_request * request, struct _u_response * response, void * user_data);

#if defined(__cplusplus)
}
#endif

#endif // IKAG_INCLUDE_IKAG_ENDPOINTS_AUTH_H

/** @} */
