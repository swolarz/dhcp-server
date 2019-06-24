#ifndef __H_DHCP_HANDLER_REQUEST
#define __H_DHCP_HANDLER_REQUEST

#include "config/dhcpconf.h"


int handle_dhcp_request(int client_fd, int resp_port, struct dhcp_config* dhcpconf);


#endif // __H_DHCP_HANDLER_REQUEST
