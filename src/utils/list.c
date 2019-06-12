#include "list.h"

#include <stdlib.h>
#include <string.h>


typedef struct list_item list_item;

typedef struct list_node {
	struct list_node* next;
	struct list_node* prev;
	list_item* item;

} list_node;

typedef struct list {
	list_node* head;
	list_node* tail;

} list;

typedef struct list_predicate list_predicate;
typedef struct list_lambda list_lambda;


list* list_create(void) {
	list* lst = malloc(sizeof(list));
	lst->head = NULL;
	lst->tail = NULL;

	return lst;
}

void list_delete(struct list* lst) {
	list_node* node = lst->head;

	while (node != NULL) {
		list_node* next = node->next;

		free(node->item);
		free(node);

		node = next;
	}

	lst->head = NULL;
	lst->tail = NULL;
	free(lst);
}


void list_append(list* lst, list_item* item) {
	list_node* node = malloc(sizeof(list_node));
	
	node->next = NULL;
	node->prev = lst->tail;
	
	list_item* copy = malloc(sizeof(*item));
	memcpy(copy, item, sizeof(*item));
	node->item = copy;

	if (lst->head == NULL)
		lst->head = node;
	else
		lst->tail->next = node;

	lst->tail = node;
}

int list_check_predicate(list_predicate predicate, list_item* item) {
	return predicate.predicate(item, predicate.data);
}

void list_remove(list* lst, list_predicate predicate) {
	list_node** head = &(lst->head);

	while (*head != NULL) {
		if (list_check_predicate(predicate, (*head)->item)) {
			if ((*head)->next != NULL)
				(*head)->next->prev = (*head)->prev;

			*head = (*head)->next;
		}
	}

	if (lst->head == NULL)
		lst->tail = NULL;
}

list_item* list_find_first(list* lst, list_predicate predicate) {
	list_node* node = lst->head;
	
	while (node != NULL) {
		if (list_check_predicate(predicate, node->item))
			return node->item;

		node = node->next;
	}

	return NULL;
}

void list_apply_lambda(list_item* item, list_lambda lambda) {
	lambda.lambda(item, lambda.data);
}

void list_foreach(list* lst, list_lambda lambda) {
	list_node* node = lst->head;

	while (node != NULL) {
		list_apply_lambda(node->item, lambda);
		node = node->next;
	}
}

