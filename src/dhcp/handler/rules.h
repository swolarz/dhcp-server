#ifndef __H_DHCP_HANDLER_RULES
#define __H_DHCP_HANDLER_RULES

#include "dhcp/inet/dhcppkt.h"
#include "dhcp/data/store.h"
#include "config/dhcpconf.h"


#define DHCP_PROCESS_OK 0
#define DHCP_PROCESS_IGNORE 1
#define DHCP_PROCESS_FAILED -1


int dhcp_req_resp_process_pipeline(struct dhcp_packet* dhcp_req,
								   struct dhcp_packet* dhcp_resp,
								   struct dhcp_config* dhcp_conf,
								   struct db_connection* db_conn);

#endif // __H_DHCP_HANDLER_RULES

