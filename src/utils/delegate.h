#ifndef __H_UTILS_DELEGATE
#define __H_UTILS_DELEGATE


struct delegate_handler;


struct delegate_handler* make_handler(
		void (*delegate_function)(void* data, void* args), void* delegate_data);

void call_handler(struct delegate_handler* dhandler, void* args);

void delete_handler(struct delegate_handler* dhandler);


#endif // __H_UTILS_DELEGATE
