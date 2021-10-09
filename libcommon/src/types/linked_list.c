#include <stdlib.h>

#include "radicle/types/linked_list.h"

list_t* list_tail(list_t** root, void* data) {

	list_t* node = (list_t*)calloc(1, sizeof(list_t));
	node->data = data;

	if(*root == NULL) {
		*root = node;
		node->tail = node;
	} else {
		(*root)->tail->next = node;
		node->prev = (*root)->tail;
		(*root)->tail = node;
	}

	return node;
}

void list_free(list_t* root, void(*free_data)(void*)) {

	list_t* iter = root->tail;

	while(iter != NULL) {
		if(free_data != NULL)
			free_data(iter->data);

		list_t* temp = iter;
		iter = iter->prev;
		free(temp);
	}

}
