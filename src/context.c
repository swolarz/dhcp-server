#include "context.h"
#include "utils/log/log.h"
#include "utils/list.h"
#include "control.h"
#include "common.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>


typedef struct control_stream control_stream;
typedef struct logger logger;

typedef struct {
	logger* log;
	struct list* ctl_streams;

} application_context;


static application_context* app_context = NULL;


static const char* TAG = "CONTEXT";

static logger* loggr() {
	return context_get_logger();
}


int init_application_context(struct arguments* args) {
	UNUSED(args);

	app_context = malloc(sizeof(application_context));
	app_context->log = initialize_log();
	app_context->ctl_streams = list_create();

	log_info(loggr(), TAG, "Application context initialized");

	return 0;
}

void context_close_log() {
	if (app_context == NULL || app_context->log == NULL)
		return;

	log_cleanup(app_context->log);
	app_context->log = NULL;
}

void close_ctl_streams() {
	if (app_context == NULL || app_context->ctl_streams == NULL)
		return;

	list_delete(app_context->ctl_streams);
	app_context->ctl_streams = NULL;
}

void cleanup_application_context() {
	log_info(app_context->log, TAG, "Cleaning application context...");

	if (app_context == NULL)
		return;

	context_close_log();
	list_delete(app_context->ctl_streams);

	free(app_context);
	app_context = NULL;
}

logger* context_get_logger() {
	if (app_context == NULL)
		return NULL;

	return app_context->log;
}


typedef struct {
	int pipe_fd[2];

} control_pipe;

/*
void print_item(struct list_item* item, void* data) {
	control_pipe* ctl_pipe  = (control_pipe*) item;
	log_verbose(loggr(), TAG, "List item (control pipe): (pull = %d, push = %d)",
			ctl_pipe->pipe_fd[0], ctl_pipe->pipe_fd[1]);
}
*/

control_stream* register_management_listener() {
	if (app_context == NULL || app_context->ctl_streams == NULL)
		return NULL;

	control_pipe ctl_pipe;
	pipe(ctl_pipe.pipe_fd);

	log_verbose(loggr(), TAG, "Created control pipe (pull = %d, push = %d)",
			ctl_pipe.pipe_fd[0], ctl_pipe.pipe_fd[1]);

	list_append(app_context->ctl_streams, (struct list_item*) &ctl_pipe, sizeof(control_pipe));
	
	control_stream* ctl_stream = malloc(sizeof(control_stream));
	memset(ctl_stream, 0, sizeof(control_stream));
	ctl_stream->ctl_pipe = ctl_pipe.pipe_fd[0];

	return ctl_stream;
}

int ctl_stream_matches(struct list_item* ctl_pipe_item, void* ctl_strm) {
	control_pipe* ctl_pipe = (control_pipe*) ctl_pipe_item;
	control_stream* ctl_stream = (control_stream*) ctl_strm;

	log_verbose(loggr(), TAG, "Matching control stream (fd = %d) with (pull = %d, push = %d)",
			ctl_stream->ctl_pipe, ctl_pipe->pipe_fd[0], ctl_pipe->pipe_fd[1]);

	return (ctl_pipe->pipe_fd[0] == ctl_stream->ctl_pipe);
}

void unregister_management_listener(control_stream* ctl_stream) {
	if (app_context == NULL || app_context->ctl_streams == NULL)
		return;

	log_debug(loggr(), TAG, "Closing control stream with control pipe: %d", ctl_stream->ctl_pipe);

	struct list_predicate rm_pred = { ctl_stream_matches, ctl_stream };
	int removed = list_remove(app_context->ctl_streams, rm_pred);

	log_debug(loggr(), TAG, "Closed %d control pipes", removed);

	free(ctl_stream);
}

void control_notify_pipe(struct list_item* ctl_pipe_item, void* data) {
	UNUSED(data);

	control_pipe* ctl_pipe = (control_pipe*) ctl_pipe_item;

	log_debug(loggr(), TAG, "Pushing message to control pipe: %d", ctl_pipe->pipe_fd[1]);
	int bytes = write(ctl_pipe->pipe_fd[1], CONTROL_STOP, CONTROL_STOP_SIZE);

	if (bytes < 0) {
		log_warn(loggr(), TAG, "Failed push message via control pipe: %s", strerror(errno));
	}
}

void control_notify_exit() {
	if (app_context == NULL || app_context->ctl_streams == NULL)
		return;

	struct list_lambda notify_lambda = { control_notify_pipe, NULL };
	list_foreach(app_context->ctl_streams, notify_lambda);
}

void context_notify_exit_signal(int signo) {
	if (signo == SIGHUP)
		context_close_log();

	log_info(app_context->log, "CONTEXT", "Received signal (no = %d). Pushing exit notification...", signo);

	control_notify_exit();
}

