/**
 * @file
 */
#include <string.h>
#include <ulfius.h>

#include "radicle/print.h"

#include "radicle/api/mail/sendgrid.h"
#include "radicle/types/string.h"

void sendgrid_instance_free(sendgrid_instance_t** instance) {
	if(*instance== NULL) return;
	string_free(&(*instance)->apiKey);
	string_free(&(*instance)->sender);
	free(*instance);
	*instance = NULL;
}

int send_mail(const sendgrid_instance_t* sg, const string_t* templateId, const json_t* values, const string_t* receiver) {

	char authorization[8 + sg->apiKey->length];
	sprintf(authorization, "Bearer %s", sg->apiKey->ptr); 

	json_t* request_body = json_object();
	json_t* personalizations = json_pack("[{s: [{s:s}], s:O?}]", "to", "email", receiver->ptr, "dynamic_template_data", values);
	json_t* sender = json_pack("{s:s}", "email", sg->sender->ptr);

	json_object_set(request_body, "personalizations", personalizations);
	json_object_set(request_body, "from", sender);
	json_object_set(request_body, "template_id", json_stringn(templateId->ptr, templateId->length));

	char* json_to_text = json_dumps(request_body, JSON_COMPACT);
	json_decref(request_body);

	struct _u_request request;
	ulfius_init_request(&request);

	/**
	 * @todo U_OPT_TIMEOUT value shouldnt be hardcoded
	 */
	ulfius_set_request_properties(&request,
		       	U_OPT_HTTP_VERB, "POST",
		       	U_OPT_HTTP_URL, "https://api.sendgrid.com/v3/mail/send",
		       	U_OPT_CHECK_SERVER_CERTIFICATE, 1,
			U_OPT_NETWORK_TYPE, U_USE_IPV4,
			U_OPT_FOLLOW_REDIRECT, 0,
			U_OPT_TIMEOUT, 10,
		       	U_OPT_STRING_BODY, json_to_text,
		       	U_OPT_HEADER_PARAMETER, "Content-Type", "application/json",
		       	U_OPT_HEADER_PARAMETER, "Authorization", authorization,
		       	U_OPT_NONE);

	struct _u_response response;
	ulfius_init_response(&response);

	if(ulfius_send_http_request(&request, &response) != U_OK) {
		ERROR("Failed to send mail.\n");
		free(json_to_text);
		ulfius_clean_request(&request);
		ulfius_clean_response(&response);
		return 1;
	}
	free(json_to_text);
	ulfius_clean_request(&request);

	if(response.status != 200 && response.status != 202) {
		char body[response.binary_body_length + 1];
		memcpy(body, response.binary_body, response.binary_body_length);
		body[response.binary_body_length] = 0;
		ulfius_clean_response(&response);
		ERROR("SendGrid responded with %ld: %s\n", response.status, body); 
		return 1;
	}

	ulfius_clean_response(&response);

	return 0;
}

int send_verification_mail(const sendgrid_instance_t* sg, const string_t* receiver, const string_t* url, const string_t* token) {
	const int max_verification_url_length = url->length + token->length + 4;
	char verification_url[max_verification_url_length];
	snprintf(verification_url,max_verification_url_length, "%s?t=%s", url->ptr, token->ptr);
	json_t* values = json_object();
	json_object_set(values, "verification_url", json_string(verification_url));
	json_object_set(values, "name", json_stringn(receiver->ptr, receiver->length));
	int result = send_mail(sg, sg->verification_template, values, receiver);
	return result;
}

int send_duplicate_mail_notify(const sendgrid_instance_t* sg, const string_t* receiver) {
	int result = send_mail(sg, sg->duplicate_mail_tempate, NULL, receiver);
	return result;
}

int send_reset_password_mail(const sendgrid_instance_t* sg, const string_t* receiver, const string_t* url, const string_t* token) {
	const int max_url_length = url->length + token->length + 4;
	char reset_url[max_url_length];
	snprintf(reset_url, max_url_length, "%s?t=%s", url->ptr, token->ptr);
	json_t* values = json_object();
	json_object_set(values, "url", json_string(reset_url));
	json_object_set(values, "name", json_stringn(receiver->ptr, receiver->length));
	int result = send_mail(sg, sg->password_reset_template, values, receiver);
	return result;
}

int send_mail_not_associated(const sendgrid_instance_t* sg, const string_t* receiver, const string_t* url) {
	json_t* values = json_object();
	json_object_set(values, "url", json_string(url->ptr));
	return send_mail(sg, sg->no_associated_account_template, values, receiver);
}
