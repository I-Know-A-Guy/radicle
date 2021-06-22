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

#include "radicle/types/uuid.h"

class RadicleUUIDTests: public ::testing::Test {
	protected:
		const unsigned char bin[16] = {0x4F, 0x01, 0xAA, 0x1A, 0xF3, 0x9D, 0x4B, 0x00, 0xAD, 0x73, 0x38, 0x49, 0x0A, 0xFE, 0x1C, 0x8E};
		const char* txt = "4f01aa1a-f39d-4b00-ad73-38490afe1c8e";
};

TEST_F(RadicleUUIDTests, TestUuidNew) {
	uuid_t* uuid = uuid_new(bin);
	EXPECT_EQ(memcmp(uuid->bin, bin, 16), 0);
	string_t* textual = uuid_to_str(uuid);
	EXPECT_STREQ(textual->ptr, txt);
	string_free(&textual);
	uuid_free(&uuid);
}
