#ifndef __H_CONTEXT
#define __H_CONTEXT

#include "args.h"


int init_application_context(struct arguments* args);
void cleanup_application_context(void);

struct logger* context_get_logger(void);


void init_control_pipe();
void register_control_pipe();


#endif // __H_CONTEXT
