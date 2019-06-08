#include "context.h"
#include "utils/log/log.h"

#include <stdlib.h>


typedef struct execution_context execution_context;
typedef struct log_handle log_handle;

typedef struct {
	execution_context* exec_context;
	log_handle* logger;

} application_context;


application_context* app_context = NULL;


int init_application_context(execution_context* exec_context) {
	app_context = malloc(sizeof(application_context));
	app_context->exec_context = exec_context;
	app_context->logger = log_get_handle();

	return 0;
}

int cleanup_application_context() {
	if (app_context == NULL)
		return;

	if (app_context->exec_context != NULL)
		call_handler(app_context->exec_context->cleanup_handler);

	if (app_context->logger != NULL)
		log_cleanup(app_context->logger);

	free(app_context);
	app_context = NULL;

	return 0;
}

log_handle* context_get_logger() {
	if (app_context == NULL)
		return NULL;

	return app_context->logger;
}

