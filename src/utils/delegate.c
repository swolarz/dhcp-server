#include "delegate.h"

#include <stdlib.h>


struct delegate_handler {
	void (*handler)(void*, void*);
	void* arg_data;
};

typedef struct delegate_handler delegate_handler;


delegate_handler* make_handler(
		void (*delegate_function)(void*, void*), void* delegate_data) {

	delegate_handler* dhandler = malloc(sizeof(delegate_handler));

	dhandler->handler = delegate_function;
	dhandler->arg_data = delegate_data;

	return dhandler;
}

void call_handler(delegate_handler* dhandler, void* args) {
	dhandler->handler(dhandler->arg_data, args);
}

void delete_handler(delegate_handler* dhandler) {
	free(dhandler);
}

