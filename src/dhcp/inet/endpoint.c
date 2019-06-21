#include "endpoint.h"
#include "context.h"
#include "utils/log/log.h"

#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>


static const char* TAG = "ENDPOINT";

static struct logger* loggr() {
	return context_get_logger();
}


int create_dhcp_socket() {
	int sfd = socket(PF_INET, SOCK_DGRAM, 0);
	if (sfd < 0)
		return -1;

	int trueval = 1;
	const void* truevalptr = (const void*) &trueval;
	
	int err = setsockopt(sfd, SOL_SOCKET, SO_BROADCAST, truevalptr, sizeof(1));
	if (err < 0) {
		close(sfd);
		return -1;
	}

	return sfd;
}

int dhcp_socket(struct sockaddr* saddr, socklen_t slen) {
	int sfd = create_dhcp_socket();
	if (sfd < 0) {
		log_error(loggr(), TAG, "Failed to create socket with broadcast capabilities: %s", strerror(errno));
		return -1;
	}

	int err = bind(sfd, saddr, slen);
	if (err < 0) {
		log_error(loggr(), TAG, "Failed to bind DHCP socket: %s", strerror(errno));
		
		close(sfd);
		return -1;
	}

	return sfd;
}

int recv_dhcp_packet(int sfd, struct dhcp_packet* dhcppkt) {
	if (dhcppkt == NULL)
		return -1;

	ssize_t bytes = recvfrom(sfd, (void*) dhcppkt, sizeof(struct dhcp_packet), 0, NULL, NULL);
	if (bytes < 0)
		return -1;

	// dhcp_ntoh(dhcppkt);

	return 0;
}

int send_dhcp_packet(int sfd, struct dhcp_packet* dhcppkt, struct sockaddr* target_addr) {
	if (dhcppkt == NULL)
		return -1;

	// dhcp_hton(dhcppkt);

	return -1;
}

struct sockaddr* broadcast_addr() {
	return NULL;
}

int translate_ifaddr(const char* ifaddr, struct in_addr* inaddr) {
	return inet_aton(ifaddr, inaddr);
}

