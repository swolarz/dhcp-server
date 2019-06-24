#ifndef __H_CONFIG_LOADER
#define __H_CONFIG_LOADER

#include "dhcpconf.h"


struct dhcp_config* dhcp_config_load(void);
void dhcp_config_cleanup(struct dhcp_config* conf);


#endif // __H_CONFIG_LOADER
