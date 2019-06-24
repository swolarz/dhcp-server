#include "endpoint.h"
#include "context.h"
#include "common.h"
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


int log_dhcp_packet(struct dhcp_packet* dhcp_pkt) {
	char buffer[1024];
	ssize_t bytes = dhcp_packet_format(dhcp_pkt, buffer, sizeof(buffer));

	if (bytes < 0) {
		log_error(loggr(), TAG, "Could not format DHCP packet");
		return -1;
	}

	log_info(loggr(), TAG, "DHCP packet data:\n%s", buffer);

	return 0;
}

int recv_dhcp_packet(int sfd, struct dhcp_packet* dhcppkt) {
	char buffer[sizeof(struct dhcp_packet)];
	ssize_t bytes = recvfrom(sfd, buffer, sizeof(buffer), 0, NULL, NULL);
	if (bytes < 0)
		return -1;

	int err = dhcp_packet_unmarshall(buffer, (size_t) bytes, dhcppkt);
	if (err < 0) {
		log_error(loggr(), TAG, "Failed to unmarshall DHCP packet");
		return -1;
	}

	log_dhcp_packet(dhcppkt);

	return 0;
}

int send_dhcp_packet(int sfd, struct dhcp_packet* dhcppkt, struct sockaddr* target_addr) {
	//TODO
	UNUSED(sfd);
	UNUSED(dhcppkt);
	UNUSED(target_addr);

	return -1;
}

struct sockaddr* broadcast_addr() {
	return NULL;
}


