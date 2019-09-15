#ifndef __H_CONFIG_DHCPCONF
#define __H_CONFIG_DHCPCONF

#include <netinet/in.h>


struct dhcp_config {
	struct in_addr net_addr;
	struct in_addr net_mask;
	struct in_addr net_gateway;

	u_int32_t lease_time_secs;
	u_int8_t default_ip_ttl;

	struct in_addr net_dhcp_addr;
	struct in_addr net_dns_addr;
};

struct in_addr dhcp_conf_null_addr(void);
int dhcp_conf_addr_not_set(struct in_addr* addr);


#endif // __H_CONFIG_DHCPCONF
