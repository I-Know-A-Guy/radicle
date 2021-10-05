/**
 * @file
 */

#include <gtest/gtest.h>
#include <jansson.h>
#include <libpq-fe.h>
#include <ulfius.h>

#include "radicle/auth/crypto.h"
#include "radicle/auth/types.h"
#include "radicle/pgdb.h"
#include "radicle/tests/api/api_fixture.hpp"
#include "radicle/api/endpoints/endpoint.h"
#include "radicle/types/string.h"
#include "radicle/api/endpoints/auth.h"
#include "subhook.h"

static char FAKE_UUID[16] = {0x1};

PGDB_FAKE_FETCH_STORY(FetchDuplicateEmail) {
	PGDB_FAKE_STORY_BRANCH(FetchDuplicateEmail, 0);
		PGDB_FAKE_RESULT_6(PGRES_TUPLES_OK, "uuid", "password", "role", "verified", "active", "created");
		PGDB_FAKE_UUID(FAKE_UUID);
		PGDB_FAKE_C_STR("password");
		PGDB_FAKE_C_STR(auth_account_role_to_str(ROLE_USER));
		PGDB_FAKE_BOOL(true);
		PGDB_FAKE_BOOL(true);
		PGDB_FAKE_TIMESTAMP(time(NULL));
		PGDB_FAKE_FINISH();
	PGDB_FAKE_STORY_BRANCH_END();

	// Session id
	PGDB_FAKE_STORY_BRANCH(FetchDuplicateEmail, 1);
		PGDB_FAKE_RESULT_1(PGRES_TUPLES_OK, "id");
		PGDB_FAKE_INT(100);
		PGDB_FAKE_FINISH();
	PGDB_FAKE_STORY_BRANCH_END();

	return NULL;
}

int send_duplicate_mail_notify_fake(const sendgrid_instance_t* sg, const string_t* receiver) {
	return 0;
}

TEST_F(APITests, TestAuthCallbackRegisterDuplicateMail) {

	install_hook(subhook_new((void*)send_duplicate_mail_notify, (void*)send_duplicate_mail_notify_fake, SUBHOOK_64BIT_OFFSET));
	install_execute_always_success();

	PGDB_FAKE_INIT_FETCH_STORY(FetchDuplicateEmail);
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(FetchDuplicateEmail));

	api_instance_t* instance = manage_instance();
	_u_request* request = manage_request();
	_u_response* response = manage_response();

	api_endpoint_t* endpoint = create_endpoint(response);
	endpoint->json_body = json_pack("{s:s, s:s}", "email", "email@mail.com", "password", "Password1!");

	ASSERT_EQ(api_auth_callback_register(request, response, instance), U_CALLBACK_COMPLETE);

	EXPECT_EQ(response->status, 200);
}

PGDB_FAKE_FETCH_STORY(FetchRegisterUuid) {

	// Duplicate mail lookup
	PGDB_FAKE_STORY_BRANCH(FetchRegisterUuid, 0);
		PGDB_FAKE_EMPTY_RESULT(PGRES_TUPLES_OK);
	PGDB_FAKE_STORY_BRANCH_END();	

	// Register insert
	PGDB_FAKE_STORY_BRANCH(FetchRegisterUuid, 1);
		PGDB_FAKE_RESULT_1(PGRES_TUPLES_OK, "uuid");
		PGDB_FAKE_UUID(FAKE_UUID);
		PGDB_FAKE_FINISH();
	PGDB_FAKE_STORY_BRANCH_END();

	// Session id
	PGDB_FAKE_STORY_BRANCH(FetchRegisterUuid, 2);
		PGDB_FAKE_RESULT_1(PGRES_TUPLES_OK, "id");
		PGDB_FAKE_INT(100);
		PGDB_FAKE_FINISH();
	PGDB_FAKE_STORY_BRANCH_END();

	return NULL;
}


int send_verification_mail_fake(const sendgrid_instance_t* sg, const string_t* receiver, const string_t* url, const string_t* token) {
	return 0;
}

TEST_F(APITests, TestAuthCallbackRegisterSuccess) {

	install_hook(subhook_new((void*)send_verification_mail, (void*)send_verification_mail_fake, SUBHOOK_64BIT_OFFSET));
	install_execute_always_success();

	PGDB_FAKE_INIT_FETCH_STORY(FetchRegisterUuid);
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(FetchRegisterUuid));

	api_instance_t* instance = manage_instance();
	_u_request* request = manage_request();
	_u_response* response = manage_response();

	api_endpoint_t* endpoint = create_endpoint(response);
	endpoint->json_body = json_pack("{s:s, s:s}", "email", "email@mail.com", "password", "Password1!");

	ASSERT_EQ(api_auth_callback_register(request, response, instance), U_CALLBACK_COMPLETE);

	EXPECT_EQ(response->status, 200);
}

PGDB_FAKE_FETCH(MakeSessionId) {
	PGDB_FAKE_RESULT_1(PGRES_TUPLES_OK, "id");
	PGDB_FAKE_INT(10);
	PGDB_FAKE_FINISH();
}

TEST_F(APITests, TestAuthCallbackResendVerificationMailAlreadyVerified) {

	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(MakeSessionId));

	api_instance_t* instance = manage_instance();
	_u_request* request = manage_request();
	_u_response* response = manage_response();
	api_endpoint_t* endpoint = create_authenticated_endpoint(response);

	endpoint->account->verified = true;

	ASSERT_EQ(api_auth_callback_resend_verification_mail(request, response, instance), U_CALLBACK_COMPLETE);

	EXPECT_EQ(response->status, 400);
}

TEST_F(APITests, TestAuthCallbackResendVerificationMailSuccess) {

	install_hook(subhook_new((void*)send_verification_mail, (void*)send_verification_mail_fake, SUBHOOK_64BIT_OFFSET));
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(MakeSessionId));
	install_execute_always_success();

	api_instance_t* instance = manage_instance();
	_u_request* request = manage_request();
	_u_response* response = manage_response();
	api_endpoint_t* endpoint = create_authenticated_endpoint(response);

	endpoint->account->verified = false;

	ASSERT_EQ(api_auth_callback_resend_verification_mail(request, response, instance), U_CALLBACK_COMPLETE);

	EXPECT_EQ(response->status, 200);
}

TEST_F(APITests, TestAuthCallbackRegisterVerifyMissingParameter) {

	api_instance_t* instance = manage_instance();
	_u_request* request = manage_request();
	_u_response* response = manage_response();
	api_endpoint_t* endpoint = create_endpoint(response);

	ASSERT_EQ(api_auth_callback_register_verify(request, response, instance), U_CALLBACK_COMPLETE);

	EXPECT_EQ(response->status, 400);
}

PGDB_FAKE_FETCH(EmptyVerifyToken) {
	PGDB_FAKE_EMPTY_RESULT(PGRES_TUPLES_OK);
}

TEST_F(APITests, TestAuthCallbackRegisterVerifyTokenNotFound) {

	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(EmptyVerifyToken));
	install_execute_always_success();

	api_instance_t* instance = manage_instance();
	_u_request* request = manage_request();
	u_map_put(request->map_url, "t", "token");
	_u_response* response = manage_response();
	api_endpoint_t* endpoint = create_endpoint(response);

	ASSERT_EQ(api_auth_callback_register_verify(request, response, instance), U_CALLBACK_COMPLETE);

	EXPECT_EQ(response->status, 307);
}

PGDB_FAKE_FETCH_STORY(FetchWrongTokenType) {
	PGDB_FAKE_STORY_BRANCH(FetchWrongTokenType, 0);
		PGDB_FAKE_RESULT_2(PGRES_TUPLES_OK, "owner", "type");
		PGDB_FAKE_UUID(FAKE_UUID);
		PGDB_FAKE_C_STR(token_type_to_str(PASSWORD_RESET));
		PGDB_FAKE_FINISH();
	PGDB_FAKE_STORY_BRANCH_END();

	PGDB_FAKE_STORY_BRANCH(FetchWrongTokenType, 1);
		PGDB_FAKE_RESULT_1(PGRES_TUPLES_OK, "id");
		PGDB_FAKE_INT(100);
		PGDB_FAKE_FINISH();
	PGDB_FAKE_STORY_BRANCH_END();
	return NULL;
}

TEST_F(APITests, TestAuthCallbackRegisterVerifyWrongTokenFound) {

	PGDB_FAKE_INIT_FETCH_STORY(FetchWrongTokenType);
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(FetchWrongTokenType));
	install_execute_always_success();

	api_instance_t* instance = manage_instance();
	_u_request* request = manage_request();
	u_map_put(request->map_url, "t", "token");
	_u_response* response = manage_response();
	api_endpoint_t* endpoint = create_endpoint(response);

	ASSERT_EQ(api_auth_callback_register_verify(request, response, instance), U_CALLBACK_COMPLETE);

	EXPECT_EQ(response->status, 307);
}

PGDB_FAKE_FETCH_STORY(RegisterVerifySuccess) {
	PGDB_FAKE_STORY_BRANCH(RegisterVerifySuccess, 0);
		PGDB_FAKE_RESULT_2(PGRES_TUPLES_OK, "owner", "type");
		PGDB_FAKE_UUID(FAKE_UUID);
		PGDB_FAKE_C_STR(token_type_to_str(REGISTRATION));
		PGDB_FAKE_FINISH();
	PGDB_FAKE_STORY_BRANCH_END();

	PGDB_FAKE_STORY_BRANCH(RegisterVerifySuccess, 1);
		PGDB_FAKE_RESULT_1(PGRES_TUPLES_OK, "id");
		PGDB_FAKE_INT(100);
		PGDB_FAKE_FINISH();
	PGDB_FAKE_STORY_BRANCH_END();
	return NULL;
}

TEST_F(APITests, TestAuthCallbackRegisterVerifySuccess) {

	PGDB_FAKE_INIT_FETCH_STORY(RegisterVerifySuccess);
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(RegisterVerifySuccess));
	install_execute_always_success();

	api_instance_t* instance = manage_instance();
	_u_request* request = manage_request();
	u_map_put(request->map_url, "t", "token");
	_u_response* response = manage_response();
	api_endpoint_t* endpoint = create_endpoint(response);

	ASSERT_EQ(api_auth_callback_register_verify(request, response, instance), U_CALLBACK_COMPLETE);

	EXPECT_EQ(response->status, 303);
}

PGDB_FAKE_FETCH_STORY(AuthSignIn) {
	API_FAKE_VERIFIED_ACCOUNT(AuthSignIn, 0);
	API_FAKE_SESSION(AuthSignIn, 1);
	return NULL;
}

int auth_verify_password_fake(const string_t* encoded, const string_t* password) {
	return 0;
}

TEST_F(APITests, TestAuthCallbackSignInSuccess) {
	PGDB_FAKE_INIT_FETCH_STORY(AuthSignIn);
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(AuthSignIn));
	install_execute_always_success();

	install_hook(subhook_new((void*)auth_verify_password, (void*)auth_verify_password_fake, SUBHOOK_64BIT_OFFSET));

	api_instance_t* instance = manage_instance();
	_u_request* request = manage_request();
	_u_response* response = manage_response();
	api_endpoint_t* endpoint = create_endpoint(response);

	endpoint->json_body = json_pack("{s:s, s:s}", "email", "test@mail.com", "password", "Password1!");

	ASSERT_EQ(api_auth_callback_sign_in(request, response, instance), U_CALLBACK_COMPLETE);

	EXPECT_EQ(response->status, 200);
}

int auth_verify_password_fake_failure(const string_t* encoded, const string_t* password) {
	return 1;
}

TEST_F(APITests, TestAuthCallbackSignInWrongPassword) {
	PGDB_FAKE_INIT_FETCH_STORY(AuthSignIn);
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(AuthSignIn));
	install_execute_always_success();

	install_hook(subhook_new((void*)auth_verify_password, (void*)auth_verify_password_fake_failure, SUBHOOK_64BIT_OFFSET));

	api_instance_t* instance = manage_instance();
	_u_request* request = manage_request();
	_u_response* response = manage_response();
	api_endpoint_t* endpoint = create_endpoint(response);

	endpoint->json_body = json_pack("{s:s, s:s}", "email", "test@mail.com", "password", "Password1!");

	ASSERT_EQ(api_auth_callback_sign_in(request, response, instance), U_CALLBACK_COMPLETE);

	EXPECT_EQ(response->status, 401);
}

API_EMPTY_RESULT(EmptyAccount);

TEST_F(APITests, TestAuthCallbackSignInAccountNotFound) {
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(EmptyAccount));
	install_execute_always_success();

	api_instance_t* instance = manage_instance();
	_u_request* request = manage_request();
	_u_response* response = manage_response();
	api_endpoint_t* endpoint = create_endpoint(response);

	endpoint->json_body = json_pack("{s:s, s:s}", "email", "test@mail.com", "password", "Password1!");

	ASSERT_EQ(api_auth_callback_sign_in(request, response, instance), U_CALLBACK_COMPLETE);

	EXPECT_EQ(response->status, 401);
}

PGDB_FAKE_FETCH_STORY(SendPasswordReset) {
	API_FAKE_VERIFIED_ACCOUNT(SendPasswordReset, 0);
	API_FAKE_SESSION(SendPasswordReset, 1);
	return NULL;
}

int send_reset_password_mail_fake(const sendgrid_instance_t* sg, const string_t* receiver, const string_t* url, const string_t* token) {
	return 0;
}

TEST_F(APITests, AuthCallbackSendPasswordResetSuccess) {
	install_execute_always_success();
	install_hook(subhook_new((void*)send_reset_password_mail, (void*)send_reset_password_mail_fake, SUBHOOK_64BIT_OFFSET));
	PGDB_FAKE_INIT_FETCH_STORY(SendPasswordReset);
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(SendPasswordReset));

	api_instance_t* instance = manage_instance();
	_u_request* request = manage_request();
	_u_response* response = manage_response();
	api_endpoint_t* endpoint = create_endpoint(response);

	endpoint->json_body = json_pack("{s:s}", "email", "test@mail.com");

	ASSERT_EQ(api_auth_callback_send_password_reset(request, response, instance), U_CALLBACK_COMPLETE);

	EXPECT_EQ(response->status, 200);
}

API_EMPTY_RESULT(SendPasswordResetNoAccount);

int send_mail_not_associated_fake(const sendgrid_instance_t* sg, const string_t* receiver, const string_t* url) {
	return 0;
}
TEST_F(APITests, AuthCallbackSendPasswordResetNoAccount) {
	install_execute_always_success();
	install_hook(PGDB_FAKE_CREATE_FETCH_HOOK(SendPasswordResetNoAccount));
	install_hook(subhook_new((void*)send_mail_not_associated, (void*)send_mail_not_associated_fake, SUBHOOK_64BIT_OFFSET));

	api_instance_t* instance = manage_instance();
	_u_request* request = manage_request();
	_u_response* response = manage_response();
	api_endpoint_t* endpoint = create_endpoint(response);

	endpoint->json_body = json_pack("{s:s}", "email", "test@mail.com");

	ASSERT_EQ(api_auth_callback_send_password_reset(request, response, instance), U_CALLBACK_COMPLETE);

	EXPECT_EQ(response->status, 200);
}

TEST_F(APITests, AuthCallbackResetPasswordSuccess) {
/** @todo continue here */	
}
