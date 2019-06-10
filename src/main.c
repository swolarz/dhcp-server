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


int main(int argc, char** argv) {
	struct arguments args = get_parsed_arguments(argc, argv);

	if (args.parse_result != 0) {
		fprintf(stderr, "Failed to parse arguments: %s\n", strerror(args.parse_result));
		exit(1);
	}

	int init_result = init_application_context(&args);
	struct logger* log = context_get_logger();

	if (init_result < 0) {
		fprintf(stderr, "Failed to initialize context: %s\n", strerror(errno));
		exit(2);
	}
	
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

	log_info(log, "MAIN", "Cleaning application context...");
	cleanup_application_context();
	
	log_info(log, "MAIN", "Shutting down...");

	return 0;
}

