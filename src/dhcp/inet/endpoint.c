#include "endpoint.h"

#include <stddef.h>
#include <string.h>


int create_dhcp_socket() {
	int sock = socket(PF_INET, SOCK_DGRAM, 0);

	int trueval = 1;
	const void* truevalptr = (const void*) &trueval;
	setsockopt(sock, SOL_SOCKET, SO_BROADCAST, truevalptr, sizeof(1));

	return sock;
}

int bind_dhcp_socket(int sock, int port) {
	struct sockaddr_in saddr;
	memset(&saddr, 0, sizeof(struct sockaddr_in));

	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = INADDR_ANY;
	saddr.sin_port = htons(port);

	return bind(sock, (struct sockaddr*) &saddr, sizeof(struct sockaddr_in));
}

int prepare_dhcp_socket(int port) {
	int sock = create_dhcp_socket();
	
	if (sock < 0)
		return sock;

	return bind_dhcp_socket(sock, port);
}

ssize_t recv_dhcp_packet(int sock, struct dhcp_packet* dhcppkt) {
	if (dhcppkt == NULL)
		return -1;

	return recvfrom(sock, (void*) dhcppkt, sizeof(struct dhcp_packet), 0, NULL, NULL);
}


