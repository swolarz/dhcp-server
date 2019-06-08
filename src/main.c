#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include "context.h"
#include "args.h"
#include "utils/log/log.h"

const char* TAG = "MAIN";


int main(int argc, char** argv) {
	struct arguments args = get_parsed_arguments(argc, argv);

	if (args.parse_result != 0) {
		fprintf(stderr, "Failed to parse arguments: %s\n", strerror(args.parse_result));
		exit(1);
	}

	//TODO create appropriate execution context (server/client)
	int init_result = init_application_context(NULL);

	if (init_result < 0) {
		fprintf(stderr, "Failed to initialize context: %s\n", strerror(errno));
		exit(2);
	}

	struct log_handle* logger = context_get_logger();

	// atexit(exit_cleanup);
	// signal(SIGINT, interrupted_exit);
	
	if (args.mode == MODE_SERVER) {
		char server_start_msg[128];
		sprintf(server_start_msg, "DHCP server started at port %d...", args.server_port);
		
		log_info(logger, TAG, server_start_msg);
	}
	else if (args.mode == MODE_CLIENT) {
		char client_start_msg[512];
		sprintf(client_start_msg, "DHCP client using server (%s) at port %d...",
				args.server_host, args.server_port);

		log_info(logger, TAG, client_start_msg);
	}

	//TODO

	log_info(logger, TAG, "Shutting down...");
	
	return 0;
}

