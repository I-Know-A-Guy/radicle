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

#include "radicle/types/linked_list.h"

static int counter = 0;

void free_counter(void* data) {
	counter++;
}	

TEST(LinkedListTests, TestLL) {
	int one = 1;
	int two = 2;
	int three = 3;

	list_t* root = NULL;

	list_tail(&root, &one);
	list_t* node_two = list_tail(&root, &two);
	list_t* node_three = list_tail(&root, &three);

	ASSERT_TRUE(root != NULL);

	EXPECT_EQ(root->data, &one);
	EXPECT_EQ(root->next, node_two);

	EXPECT_EQ(node_two->data, &two);
	EXPECT_EQ(node_two->prev, root);
	ASSERT_EQ(node_two->next, node_three);

	EXPECT_EQ(node_three->data, &three);
	EXPECT_EQ(node_three->prev, node_two);
	EXPECT_EQ(root->tail, node_three);

	list_free(root, &free_counter);

	EXPECT_EQ(counter, 3);
}

