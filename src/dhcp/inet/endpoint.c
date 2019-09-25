#include "endpoint.h"
#include "context.h"
#include "common.h"
#include "utils/log/log.h"

#include <stdlib.h>
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


static int log_dhcp_request(struct dhcp_packet* dhcp_pkt) {
	char buffer[1024];
	ssize_t bytes = dhcp_packet_format(dhcp_pkt, buffer, sizeof(buffer));

	if (bytes < 0) {
		log_error(loggr(), TAG, "Could not format DHCP request packet");
		return -1;
	}

	log_info(loggr(), TAG, "DHCP REQUEST:\n## DHCP packet data:\n%s", buffer);

	return 0;
}

static int log_dhcp_response(struct dhcp_packet* dhcp_pkt) {
	char buffer[1024];
	ssize_t bytes = dhcp_packet_format(dhcp_pkt, buffer, sizeof(buffer));

	if (bytes < 0) {
		log_error(loggr(), TAG, "Could not format DHCP response packet");
		return -1;
	}

	log_info(loggr(), TAG, "DHCP RESPONSE:\n## DHCP packet data:\n%s", buffer);

	return 0;
}

int recv_dhcp_req_packet(int sfd, struct dhcp_packet* dhcp_req) {
	size_t pkt_size = DHCP_PKT_REQ_MAX_SIZE;
	char* buffer = malloc(pkt_size + 1);

	ssize_t msg_size = recv(sfd, buffer, pkt_size + 1, 0);
	if (msg_size < 0) {
		log_error(loggr(), TAG, "Error occurred while receiving DHCP packet: %s", strerror(errno));
		free(buffer);

		return -1;
	}

	size_t bytes = (size_t) msg_size;

	if (bytes > pkt_size) {
		log_warn(loggr(), TAG, "Ignored DHCP request packet exceeding maximum length of %zu bytes", pkt_size);
		free(buffer);

		return -1;
	}

	ssize_t marshalled = dhcp_packet_req_unmarshall(buffer, bytes, dhcp_req);
	if (marshalled < 0) {
		log_error(loggr(), TAG, "Failed to unmarshall DHCP packet");
		free(buffer);

		return -1;
	}

	log_dhcp_request(dhcp_req);
	free(buffer);

	return 0;
}

int send_dhcp_resp_packet(int sfd, struct dhcp_packet* dhcp_resp, struct sockaddr* target_addr, socklen_t addrlen) {
	log_info(loggr(), TAG, "Sending DHCP response packet");
	log_dhcp_response(dhcp_resp);

	size_t pkt_size = dhcp_resp_packet_size(dhcp_resp);
	char* buffer = malloc(pkt_size);

	log_debug(loggr(), TAG, "Allocated buffer for DHCP response of size: %zu", pkt_size);

	ssize_t bytes = dhcp_packet_resp_marshall(dhcp_resp, buffer, &pkt_size);
	if (bytes < 0) {
		log_error(loggr(), TAG, "Failed to marshall DHCP response packet");
		free(buffer);

		return -1;
	}

	ssize_t sent = sendto(sfd, buffer, pkt_size, 0, target_addr, addrlen);
	if (sent < 0) {
		log_error(loggr(), TAG, "Failed to send DHCP response packet: %s", strerror(errno));
		free(buffer);

		return -1;
	}

	free(buffer);

	return 0;
}

