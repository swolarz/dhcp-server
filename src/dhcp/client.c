#include "client.h"
#include "context.h"
#include "utils/log/log.h"


void dhcp_client_start(struct client_args args) {
	struct logger* log = context_get_logger();

	log_info(log, "CLIENT", "Starting DHCP client...");

	//TODO print server host and port
}
