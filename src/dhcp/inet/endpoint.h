#ifndef HEADER_DHCP_ENDPOINT
#define HEADER_DHCP_ENDPOINT

#include "dhcp/inet/packet.h"

#include <netinet/in.h>


int dhcp_socket(struct sockaddr* saddr, socklen_t slen);

int recv_dhcp_packet(int sfd, struct dhcp_packet* dhcppkt);
int send_dhcp_packet(int sfd, struct dhcp_packet* dhcppkt, struct sockaddr* target_addr);

int translate_ifaddr(const char* ifaddr, struct in_addr* inaddr);
//int resolve_host_address(const char* addr, struct in_addr* inaddr);


#endif // HEADER_DHCP_ENDPOINT
