#ifndef HEADER_DHCP_ENDPOINT
#define HEADER_DHCP_ENDPOINT

#include "dhcp/inet/packet.h"


int prepare_dhcp_socket(int port);

ssize_t recv_dhcp_packet(int sock, struct dhcp_packet* dhcppkt);


#endif // HEADER_DHCP_ENDPOINT
