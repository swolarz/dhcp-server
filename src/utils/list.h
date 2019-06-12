#ifndef __H_UTILS_LIST
#define __H_UTILS_LIST


struct list;

struct list_item {
	char payload[1];
};

struct list_predicate {
	int (*predicate)(struct list_item*, void*);
	void* data;
};

struct list_lambda {
	void (*lambda)(struct list_item*, void*);
	void* data;
};


struct list* list_create(void);
void list_delete(struct list* lst);

void list_append(struct list* lst, struct list_item* item);
void list_remove(struct list* lst, struct list_predicate predicate);

struct list_item* list_find_first(struct list* lst, struct list_predicate predicate);

void list_foreach(struct list* lst, struct list_lambda lambda);


#endif // __H_UTILS_LIST
