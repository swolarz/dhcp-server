#ifndef __H_CONFIG_DHCPCONF
#define __H_CONFIG_DHCPCONF

#include <netinet/in.h>


struct dhcp_config {
	struct in_addr net_addr;
	struct in_addr net_mask;
	struct in_addr net_gateway;
};


#endif // __H_CONFIG_DHCPCONF
