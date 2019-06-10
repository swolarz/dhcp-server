#include "context.h"
#include "utils/log/log.h"

#include <stdlib.h>


typedef struct logger logger;

typedef struct {
	logger* log;

} application_context;


static application_context* app_context = NULL;


int init_application_context(struct arguments* args) {
	app_context = malloc(sizeof(application_context));
	app_context->log = initialize_log();

	log_info(app_context->log, "CONTEXT", "Application context initialized");

	return 0;
}

void cleanup_application_context() {
	if (app_context == NULL)
		return;

	if (app_context->log != NULL)
		log_cleanup(app_context->log);

	free(app_context);
	app_context = NULL;
}

logger* context_get_logger() {
	if (app_context == NULL)
		return NULL;

	return app_context->log;
}

void init_control_pipe() {
}

void register_control_pipe() {
}


