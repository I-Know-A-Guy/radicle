#include <vector>

#include <gtest/gtest.h>

#include "radicle/types/string.h"
#include "radicle/types/uuid.h"
#include "radicle/tests/subhook.h"

class RadicleTests: public ::testing::Test {

	std::vector<string_t*> strings;
	std::vector<uuid_t*> uuids;
	std::vector<subhook_t> hooks;

	protected:

	uuid_t* common_uuid;
	string_t* common_string;

	void SetUp() override {
		const unsigned char temp[16] = {0x00};
		common_uuid = uuid_new(temp);
		common_string = string_from_literal("I am a test string!");
	}

	void TearDown() override {
		for(std::vector<string_t*>::iterator iter = strings.begin(); iter != strings.end(); iter++) {
			string_free(iter.base());
		}

		for(std::vector<uuid_t*>::iterator iter = uuids.begin(); iter != uuids.end(); iter++) {
			uuid_free(iter.base());
		}

		for(std::vector<subhook_t>::iterator iter = hooks.begin(); iter != hooks.end(); iter++) {
			subhook_remove(*iter.base());
			subhook_free(*iter.base());
		}
		uuid_free(&common_uuid);
		string_free(&common_string);
	}


	string_t* manage_string(const char* literal) {
		string_t* buf = string_from_literal(literal);
		strings.push_back(buf);
		return buf;
	}

	void test_string_len(string_t* buf) {
		EXPECT_EQ(buf->length, strlen(buf->ptr));
	}

	void install_hook(subhook_t hook) {
		hooks.push_back(hook);
		subhook_install(hook);
	}

	void remove_hook(subhook_t hook) {
		subhook_remove(hook);
		subhook_free(hook);
		int index = 0;
		for(int i = 0; i < hooks.size(); i++) {
			if(hooks[i] == hook) {
				index = i;
				break;
			}
		}
		hooks.erase(hooks.begin() + index);
	}

};

