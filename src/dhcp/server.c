#include "server.h"
#include "context.h"
//#include "dhcp/inet/packet.h"
//#include "dhcp/inet/endpoint.h"
#include "dhcp/store.h"
#include "utils/log/log.h"

#include <string.h>


static struct logger* loggr() {
	return context_get_logger();
}

void dhcp_server_start(struct server_args args) {
	log_info(loggr(), "SERVER", "Starting DHCP server at port %d...", args.server_port);

	struct db_connection* db_conn = init_db_connection();

	// TODO
	
	cleanup_db_connection(db_conn);
}

/*
int listen_dhcp_packets(int port) {
	int sock = prepare_dhcp_socket(port);

	if (sock < 0)
		return sock;
	
	while (1) {
		struct dhcp_packet dhcppkt;
		memset(&dhcppkt, 0, sizeof(struct dhcp_packet));

		ssize_t pkt_size = recv_dhcp_packet(sock, &dhcppkt);

		if (pkt_size < 0) {

		}
		else {
		}
	}
}
*/

