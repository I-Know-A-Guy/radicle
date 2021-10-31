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

TEST_F(RadicleTests, TestStringCat) {
	string_t* first  = string_from_literal("Hello");
	string_t* second = string_from_literal(" World!");
	string_t* result = string_cat(first, second);
	EXPECT_STREQ(result->ptr, "Hello World!");
	EXPECT_EQ(strlen(result->ptr), result->length);
	string_free(&first);
	string_free(&second);
	string_free(&result);
}
