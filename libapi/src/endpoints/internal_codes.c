#include "radicle/api/endpoints/internal_codes.h"

const char* internal_errors_msg(internal_errors_t code) {
	switch(code) {
		case SUCCESS:
			return "Ok";
		case NOT_FOUND:
			return "Not Found";
		case OPTION_RESPONSE:
			return "Options request.";
		case VALIDATION_ERRORS:
			return "Validation errors.";
		case VALIDATION_INVALID_EMAIL:
			return "Validation Invalid email";
		case VALIDATION_INVALID_PASSWORD:
			return "Validation invalid password.";
		case VALIDATION_INVALID_COOKIE:
			return "Validation invalid cookie.";
		case VALIDATION_NOT_AUTHENTICATED:
			return "Validation not authenticated.";
		case VALIDATION_MISSING_PARAMETER:
			return "Validation missing parameter";
		case VALIDATION_EMAIL_ALREADY_VERIFIED:
			return "Email has already been verified.";
		case PGDB:
			return "PGDB codes";
		case PGDB_UNABLE_TO_CLAIM:
			return "Unable to claim pgdb connection.";
		case ERROR_TRANSACTION_BEGIN:
			return "Unable to begin transaction";
		case ERROR_TRANSACTION_COMMIT:
			return "Unable to commit transaction.";
		case ERROR_TRANSACTION_ROLLBACK:
			return "Unable to rollback transaction.";
		case AUTH:
			return "Auth internal codes.";
		case SUCCESS_REGISTER:
			return "Successfully registered account.";
		case SUCCESS_CREDENTIALS_SIGN_IN:
			return "Successfully signed in using credentials.";
		case SUCCESS_COOKIE_SIGN_IN:
			return "Successfully signed in using cookie.";
		case SUCCESS_VERIFY_REGISTRATION:
			return "Successfully verified email.";
		case ERROR_OWNED_SESSION:
			return "Unable to create owned session.";
		case ERROR_EMAIL_LOOKUP:
			return "Failed to lookup email.";
		case ERROR_REGISTER:
			return "Failed to register.";
		case ERROR_CREATING_TOKEN:
			return "Error creating token.";
		case ERROR_REVOKE_REGISTRATIONS_TOKEN:
			return "Error revoking registration token.";
		case ERROR_SEND_MAIL_DUPLICATE_REGISTER_NOTIFY:
			return "Error sending duplicate register mail";
		case ERROR_SEND_MAIL_REGISTER_TOKEN:
			return "Error sending token";
		case INVALID_REGISTER_TOKEN:
			return "Invalid register token";
		case ERROR_VERIFYING_TOKEN:
			return "Error verifying register token";
		case DUPLICATE_EMAIL_REGISTER:
			return "Duplicate mail registering";
		case INVALID_USER_CREDENTIALS:
			return "Invalid user credentials";
		case FAILED_TO_SEND_MAIL:
			return "Failed to send mail";
		case INVALID_TOKEN_TYPE:
			return "Token is of wrong type.";
		case ERROR_UPDATING_PASSWORD:
			return "Failed to update password.";
		case VALIDATION_EMAIL_NOT_VERIFIED:
			return "Email not yet verified.";
		case ERROR_UPDATING_ACCOUNT_EMAIL:
			return "Failed to update mail.";
		case ERROR_BLACKLIST_LOOKUP:
			return "Failed to lookup blacklist.";
		case FORBIDDEN_BLACKLIST_IP:
			return "Request aborted since requesters ip has been blacklsited.";
		case ERROR_SESSION_ACCESS_LOOKUP:
			return "Failed to lookup session access.";
		case ERROR_SAVING_BLACKLIST:
			return "Failed to save new ip to blacklsit";
		default:
			return "missing error message.";
	}
}

const char* public_internal_errors_msg(internal_errors_t code) {
}
