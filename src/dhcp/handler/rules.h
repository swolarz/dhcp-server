#ifndef __H_DHCP_HANDLER_RULES
#define __H_DHCP_HANDLER_RULES

#include "dhcp/inet/dhcppkt.h"
#include "config/dhcpconf.h"


int dhcp_req_resp_process_pipeline(struct dhcp_packet* dhcp_req,
								   struct dhcp_packet* dhcp_resp,
								   struct dhcp_config* dhcp_conf);

#endif // __H_DHCP_HANDLER_RULES

