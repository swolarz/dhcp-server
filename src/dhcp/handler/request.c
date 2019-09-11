#include "request.h"
#include "context.h"
#include "config/dhcpconf.h"
#include "dhcp/inet/dhcppkt.h"
#include "dhcp/inet/endpoint.h"
#include "utils/net.h"
#include "utils/log/log.h"

#include <string.h>


static const char* TAG = "REQUEST";

struct logger* loggr() {
	return context_get_logger();
}


static void prepare_broadcast_addr(struct sockaddr_in* saddr, int resp_port) {
	memset(saddr, 0, sizeof(struct sockaddr_in));

	saddr->sin_family = AF_INET;
	saddr->sin_addr.s_addr = htonl(INADDR_BROADCAST);
	saddr->sin_port = htons(resp_port);
}

static void prepare_dhcp_response(struct dhcp_packet* response, struct dhcp_packet* request, struct in_addr* as_addr, struct in_addr* gate_addr) {
	memset(response, 0, sizeof(struct dhcp_packet));

	response->op = OP_BOOTPREPLY;
	response->htype = HTYPE_ETHERNET;
	response->hwlen = HWLEN_ETHERNET;
	response->xid = request->xid;
	response->secs = 100;

	memcpy(&(response->yiaddr), as_addr, sizeof(struct in_addr));
	memcpy(&(response->giaddr), gate_addr, sizeof(struct in_addr));

	response->options[0] = 0xff;
}


int handle_dhcp_request(int client_fd, int resp_port, struct dhcp_config* dhcpconf) {
	log_debug(loggr(), TAG, "Handling incoming DHCP request");

	struct dhcp_packet request;
	memset(&request, 0, sizeof(struct dhcp_packet));

	int err = recv_dhcp_packet(client_fd, &request);
	if (err < 0)
		return -1;

	log_debug(loggr(), TAG, "Preparing DHCP response (xid = %u)", request.xid);

	struct dhcp_packet response;
	prepare_dhcp_response(&response, &request, &(dhcpconf->net_addr), &(dhcpconf->net_gateway));

	log_debug(loggr(), TAG, "Preparing DHCP response broadcast address");

	socklen_t braddr_len = sizeof(struct sockaddr_in);
	struct sockaddr_in braddr;
	prepare_broadcast_addr(&braddr, resp_port);

	log_debug(loggr(), TAG, "Sending DHCP response packet");
	
	err = send_dhcp_packet(client_fd, &response, (struct sockaddr*) &braddr, braddr_len);
	if (err < 0)
		return -1;

	return 0;
}

