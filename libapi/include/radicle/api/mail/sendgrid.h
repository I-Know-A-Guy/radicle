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
 * @brief Methods for SendGrid integration.
 *
 * @addtogroup libapi
 * @{
 * @addtogroup libapi_mail Mail
 * @{
 */

#ifndef RADICLE_LIBAPI_INCLUDE_RADICLE_API_MAIL_SENDGRID_H 
#define RADICLE_LIBAPI_INCLUDE_RADICLE_API_MAIL_SENDGRID_H

#include <ulfius.h>

#include "radicle/types/string.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define MAIL_MAX_BODY_LEN 1024

/**
 * @brief SendGrid variables
 */
typedef struct sendgrid_instance {
	string_t* apiKey; /**< API Key for Sendgrid */
	string_t* sender; /**< Sender mail which has been verified in Sendgrid */
	string_t* verification_template; /**< Mail template which is used for sending mail verification token. */
	string_t* duplicate_mail_tempate; /**< Template which is used, if someone is trying to register with an already registered email. */
	string_t* password_reset_template; /**< Template which contains a password reset link. */
	string_t* no_associated_account_template; /**< Mail which is only sent, if someone tries to reset their password for an account, which doesnt exist. */
	string_t* email_change_template; /**< Email which will need to be confirmed for email change */
} sendgrid_instance_t;

/**
 * @brief Frees all assoicated data from the instance.
 */
void sendgrid_instance_free(sendgrid_instance_t** instance);

/**
 * @brief Delievers mail to sendgrid via api integration.
 *
 * @param sg Sendgrid instance. 
 * @param templateId Template id which will be used
 * @param values JSON values which will be inserted into template.
 * @param receiver Mail of receiver.
 *
 * @return Returns 0 on success.
 */
int send_mail(const sendgrid_instance_t* sg, const string_t* templateId, const json_t* values, const string_t* receiver);

/**
 * @brief Sends a mail containing the verification token.
 *
 * @param sg SendGrid instance containing apiKey and sender mail.
 * @param receiver Mail which will receive text
 * @param url URL which shall be used for registration confimation. ?t=token
 * will be appended to it.
 * @param token Actual verification token.
 *
 * @return returns 0 on success.
 */
int send_verification_mail(const sendgrid_instance_t* sg, const string_t* receiver, const string_t* url, const string_t* token);

/**
 * @brief Sends a mail notifying the user about the duploiate mail.
 *
 * @param sg SendGrid instance containing apiKey and sender mail.
 * @param receiver Mail which will receive text
 *
 * @return returns 0 on success.
 */
int send_duplicate_mail_notify(const sendgrid_instance_t* sg, const string_t* receiver);

/**
 * @brief Sends a mail containing the password reset token.
 *
 * @param sg SendGrid instance containing apiKey and sender mail.
 * @param receiver Mail which will receive text
 * @param url URL to which users shall be routed after receiving token via mail
 * @param token Actual reset token.
 *
 * @return returns 0 on success.
 */
int send_reset_password_mail(const sendgrid_instance_t* sg, const string_t* receiver, const string_t* url, const string_t* token);

/**
 * @brief Email stating that email is not associated to any account.
 *
 * @param sg SendGrid instance containing apiKey and sender mail.
 * @param receiver Mail which will receive text
 * @param url URL to which users are linked if they want to create an account.
 *
 * @return returns 0 on success.
 */
int send_mail_not_associated(const sendgrid_instance_t* sg, const string_t* receiver, const string_t* url);

/**
 * @brief Email which contains a token which will need to be verified if someone
 * tries to change their email
 *
 * @param sg SendGrid instance containing apiKey and sender mail.
 * @param receiver Mail which will receive text
 * @param url  URL which needs to be confirmed if someone tries to change their
 * email.
 *
 * @return returns 0 on success.
 */
int send_change_email_verification(const sendgrid_instance_t* sg, const string_t* receiver, const string_t* url, const string_t* token);


#if defined(__cplusplus)
}
#endif

#endif //RADICLE_LIBAPI_INCLUDE_RADICLE_API_MAIL_SENDGRID_H

/** @} */
/** @} */
