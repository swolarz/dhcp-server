#include "dhcpconf.h"

#include <string.h>


struct in_addr dhcp_conf_null_addr(void) {
	struct in_addr naddr;
	memset(&naddr, 0, sizeof(struct in_addr));

	naddr.s_addr = INADDR_NONE;

	return naddr;
}

int dhcp_conf_addr_not_set(struct in_addr* addr) {
	return (addr->s_addr == INADDR_NONE);
}
