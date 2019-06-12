#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include "args.h"
#include "context.h"
#include "dhcp/server.h"
#include "dhcp/client.h"
#include "utils/log/log.h"


struct logger* loggr() {
	return context_get_logger();
}


void on_shutdown() {
	cleanup_application_context();
}

void on_exit_signal(int signo) {
	context_notify_exit_signal(signo);
}

void setup_exit_handler() {
	atexit(on_shutdown);
	
	struct sigaction sigact_exit;
	memset(&sigact_exit, 0, sizeof(struct sigaction));

	sigact_exit.sa_handler = on_exit_signal;

	sigaction(SIGINT, sigaction, NULL);
	sigaction(SIGTERM, sigaction, NULL);
	sigaction(SIGHUP, sigaction, NULL);
	sigaction(SIGQUIT, sigaction, NULL);
}


int main(int argc, char** argv) {
	struct arguments args = get_parsed_arguments(argc, argv);

	if (args.parse_result != 0) {
		fprintf(stderr, "Failed to parse arguments: %s\n", strerror(args.parse_result));
		exit(1);
	}

	int init_result = init_application_context(&args);
	if (init_result < 0) {
		fprintf(stderr, "Failed to initialize context: %s\n", strerror(errno));
		exit(2);
	}

	setup_exit_handler();

	if (args.mode == MODE_SERVER) {
		struct server_args sargs = {
			args.server_port,
			args.server_host
		};

		dhcp_server_start(sargs);
	}
	else if (args.mode == MODE_CLIENT) {
		struct client_args cargs = {
			args.server_port,
			args.server_host
		};

		dhcp_client_start(cargs);
	}

	return 0;
}

