#include "dhcpconf.h"

#include <string.h>
#include <stdlib.h>


typedef struct dhcp_config dhcp_config;


dhcp_config* create_dhcp_config() {
	dhcp_config* conf = malloc(sizeof(dhcp_config));
	if (conf == NULL)
		return NULL;

	memset(conf, 0, sizeof(dhcp_config));

	conf->lease_time_secs = 3600;	// 1h
	conf->default_ip_ttl = 64;

	conf->net_dns_addr = dhcp_conf_null_addr();

	return conf;
}

void dhcp_config_cleanup(dhcp_config* conf) {
	free(conf);
}


struct in_addr dhcp_conf_null_addr(void) {
	struct in_addr naddr;
	memset(&naddr, 0, sizeof(struct in_addr));

	naddr.s_addr = INADDR_NONE;

	return naddr;
}

int dhcp_conf_addr_not_set(struct in_addr* addr) {
	return (addr->s_addr == INADDR_NONE);
}
