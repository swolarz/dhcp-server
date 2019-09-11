#ifndef __H_CONTEXT
#define __H_CONTEXT

#include "args.h"


int application_context_init(struct arguments* args);
void application_context_cleanup(void);

struct logger* context_get_logger(void);


const char* context_db_path(void);
const char* context_config_path(void);


struct control_stream {
	int ctl_pipe;
};

struct control_stream* register_management_listener();
void unregister_management_listener(struct control_stream* ctl_stream);

void context_notify_exit_signal(int signo);


#endif // __H_CONTEXT
