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

#include <vector>

#include <gtest/gtest.h>

#include "subhook.h"

#include "radicle/types/string.h"
#include "radicle/types/uuid.h"
#include "radicle/types/linked_list.h"


/**
 * @brief Provies common_uuid, common_string and manages strings, uuids and hook.
 */
class RadicleTests: public ::testing::Test {

	typedef std::pair<list*, void (*)(void*)> list_pair;

	std::vector<string_t*> strings; /**< Will be freed on TearDown */
	std::vector<uuid_t*> uuids; /**< Will be freed on TearDown */
	std::vector<list_pair> lists; /**< Will be freed on TearDown */
	std::vector<subhook_t> hooks; /**< Will be freed on TearDown */

	protected:

	uuid_t* common_uuid; /**< Common string which can be used in place of real strings */
	string_t* common_string; /**< Common uuid which can be used in place of a real uuid */

	/**
	 * @brief Initialzes common members.
	 */
	void SetUp() override {
		const unsigned char temp[16] = {0x00};
		common_uuid = uuid_new(temp);
		common_string = string_from_literal("I am a test string!");
	}

	/**
	 * @brief Frees all objects of which ownership was taken.
	 */
	void TearDown() override {
		for(std::vector<string_t*>::iterator iter = strings.begin(); iter != strings.end(); iter++) {
			string_free(iter.base());
		}

		for(std::vector<uuid_t*>::iterator iter = uuids.begin(); iter != uuids.end(); iter++) {
			uuid_free(iter.base());
		}

		for(std::vector<list_pair>::iterator iter = lists.begin(); iter != lists.end(); iter++) {
			list_free(iter->first, iter->second);
		}

		for(std::vector<subhook_t>::iterator iter = hooks.begin(); iter != hooks.end(); iter++) {
			subhook_remove(*iter.base());
			subhook_free(*iter.base());
		}
		uuid_free(&common_uuid);
		string_free(&common_string);
	}

	/**
	 * @brief Takes ownership of string and frees it on TearDown()
	 */
	string_t* manage_string(const char* literal) {
		string_t* buf = string_from_literal(literal);
		strings.push_back(buf);
		return buf;
	}

	void take_list(list_t* list, void(*free_func)(void*)) {
		lists.push_back({list, free_func});
	}

	/**
	 * @brief Takes ownership of existring string
	 */
	void take_string(string_t* string) {
		strings.push_back(string);
	}

	/**
	 * @brief Takes ownership of existring uuid 
	 */
	void take_uuid(uuid_t* uuid) {
		uuids.push_back(uuid);
	}

	/**
	 * @brief Tests if \ref string_t.ptr length matches given \ref string.length
	 */
	void test_string_len(string_t* buf) {
		EXPECT_EQ(buf->length, strlen(buf->ptr));
	}

	/**
	 * @brief Takes ownership of a hook and installs it.
	 */
	subhook_t install_hook(subhook_t hook) {
		hooks.push_back(hook);
		subhook_install(hook);
		return hook;
	}

	/**
	 * @brief Uninstalls hook and removes it from \ref hooks
	 */
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

