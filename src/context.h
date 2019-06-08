#ifndef __H_CONTEXT
#define __H_CONTEXT

#include "utils/delegate.h"


struct execution_context {
	struct delegate_handler* cleanup_handler;
	struct delegate_handler* start_handler;
	char _padding[64];
};

int init_application_context(struct execution_context* exec_context);
int cleanup_application_context(void);

struct log_handle* context_get_logger(void);


#endif // __H_CONTEXT
