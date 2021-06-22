#include <gtest/gtest.h>

#include "radicle/tests/radicle_fixture.hpp"
#include "radicle/types/string.h"

TEST_F(RadicleTests, TestStringNew) {
	string_t* str = string_new("hello", 5);
	test_string_len(str);
	string_free(&str);
	EXPECT_TRUE(str == NULL);
}

TEST_F(RadicleTests, TestStringCopy) {
	string_t* str = string_from_literal("hello");
	string_t* cpy = string_copy(str); 

	test_string_len(cpy);
	EXPECT_STREQ(cpy->ptr, str->ptr);

	string_free(&str);
	string_free(&cpy);
}

TEST_F(RadicleTests, TestStringEmpty) {
	string_t* str = string_new_empty(10);
	EXPECT_EQ(str->length, 10);
	char cpy[10] = {0};
	EXPECT_EQ(memcmp(str->ptr, cpy, 10), 0);
	string_free(&str);
}

TEST_F(RadicleTests, TestStringLiteral) {
	string_t* str = string_from_literal("Hello World!");
	test_string_len(str);
	EXPECT_STREQ(str->ptr, "Hello World!");
	string_free(&str);
}
