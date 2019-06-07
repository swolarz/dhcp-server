#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <argp.h>

//#include "dhcp/endpoint.h"
//#include "dhcp/handler.h"
//#include "utils/delegate.h"
#include "utils/log/log.h"


typedef struct {
	struct log_handle* logger;

} application_context;

application_context* app_context;

int main(int argc, char** argv) {
	error_t parse_result = argp_parse(0, argc, argv, 0, 0, 0);
	
	int port = 67;

	app_context = malloc(sizeof(application_context));
	app_context->logger = log_get_handle();

	log_info(app_context->logger, "MAIN", "DHCP server started at port 67...");

	// atexit(exit_cleanup);
	// signal(SIGINT, interrupted_exit);

	//int result = listen_dhcp_packets(port, NULL);
	int result = 0;

	if (result < 0) {
		fprintf(stderr, "Failed to start dhcp listen loop: %s\n", strerror(errno));
		exit(1);
	}

	log_info(app_context->logger, "MAIN", "Shutting down...");
	
	return 0;
}

