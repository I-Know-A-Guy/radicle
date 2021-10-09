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

/**
 * @file
 * @brief Header of LinkedList
 * @author Nils Egger
 *
 * @addtogroup Common 
 * @{
 * @addtogroup Types 
 * @{
 * @addtogroup LinkedList 
 * @{
 */

#ifndef RADICLE_COMMON_INCLUDE_RADICLE_TYPES_LINKED_LIST_H 
#define RADICLE_COMMON_INCLUDE_RADICLE_TYPES_LINKED_LIST_H 

#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Only use if length is not known
 */
typedef struct list {
	struct list* next; /**< Child node. */
	struct list* prev; /**< parent node. */
	struct list* tail; /**< Tail of list, only accessible with root node. */
	void* data; /**< Custom data */
} list_t;

/**
 * @brief adds data to tail of list
 *
 * @param root First node in list
 * @param data new data to be added as tail
 *
 * @return Returns new list node.
 */
list_t* list_tail(list_t** root, void* data); 

/**
 * @brief Removes node from list
 *
 * @param node Node to remove.
 */
// void list_remove(list_t* node);

/**
 * @brief Frees list and data
 *
 * @param root First node of list
 * @param free_data Func which takes data as argument and frees it, if NULL will
 * not be called
 */
void list_free(list_t* root, void(*free_data)(void*));

#if defined(__cplusplus)
}
#endif

#endif // RADICLE_COMMON_INCLUDE_RADICLE_TYPES_LINKED_LIST_H 

/** @} */
/** @} */
/** @} */


