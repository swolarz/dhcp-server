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


void exit_cleanup() {
	// No action
}

void interrupted_exit(int signo) {
	exit(0);
}

int main(int argc, char** argv) {
	// TODO arg parsing
	
	int port = 67;

	atexit(exit_cleanup);
	signal(SIGINT, interrupted_exit);

	//int result = listen_dhcp_packets(port, NULL);
	int result = 0;

	if (result < 0) {
		fprintf(stderr, "Failed to start dhcp listen loop: %s\n", strerror(errno));
		exit(1);
	}
	
	return 0;
}

