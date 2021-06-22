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
