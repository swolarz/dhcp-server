#ifndef __H_DHCP_HANDLER_REQUEST
#define __H_DHCP_HANDLER_REQUEST

#include "config/dhcpconf.h"
#include "dhcp/data/store.h"


#define RESP_DEST_IP_LEN 32

struct handler_args {
	char resp_dest_ip[RESP_DEST_IP_LEN + 1];
};

int handle_dhcp_request(int client_fd, int resp_port, struct db_connection* db_conn, struct dhcp_config* dhcpconf, struct handler_args* hargs);


#endif // __H_DHCP_HANDLER_REQUEST

