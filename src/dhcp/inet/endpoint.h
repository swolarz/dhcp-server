#ifndef HEADER_DHCP_ENDPOINT
#define HEADER_DHCP_ENDPOINT

#include "dhcp/inet/dhcppkt.h"

#include <netinet/in.h>


int dhcp_socket(struct sockaddr* saddr, socklen_t slen);

int recv_dhcp_packet(int sfd, struct dhcp_packet* dhcppkt);
int send_dhcp_packet(int sfd, struct dhcp_packet* dhcppkt, struct sockaddr* target_addr);


#endif // HEADER_DHCP_ENDPOINT
