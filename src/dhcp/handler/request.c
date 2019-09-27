#include "request.h"
#include "context.h"
#include "config/dhcpconf.h"
#include "dhcp/inet/dhcppkt.h"
#include "dhcp/inet/dhcpopt.h"
#include "dhcp/inet/endpoint.h"
#include "dhcp/handler/rules.h"
#include "utils/net.h"
#include "utils/log/log.h"

#include <string.h>


static const char* TAG = "REQUEST";

static struct logger* loggr() {
	return context_get_logger();
}


static void prepare_broadcast_addr(struct sockaddr_in* saddr, const char* dest_ip, int resp_port) {
	memset(saddr, 0, sizeof(struct sockaddr_in));

	if (parse_inaddr(dest_ip, &(saddr->sin_addr)) != 0)
		saddr->sin_addr.s_addr = htonl(INADDR_BROADCAST);

	saddr->sin_family = AF_INET;
	saddr->sin_port = htons(resp_port);
}

static int prepare_dhcp_response(struct dhcp_packet* response, struct dhcp_packet* request, struct dhcp_config* dhcpconf, struct db_connection* db_conn) {
	int err = dhcp_req_resp_process_pipeline(request, response, dhcpconf, db_conn);

	if (err == DHCP_PROCESS_FAILED) {
		log_error(loggr(), TAG, "Failed to prepare DHCP response based on the request");
		return -1;
	}

	if (err == DHCP_PROCESS_IGNORE)
		log_info(loggr(), TAG, "Ignoring DHCP request");
	
	return 0;
}


static int send_dhcp_response(int client_fd, int resp_port, struct dhcp_packet* response, struct handler_args* hargs) {
	log_debug(loggr(), TAG, "Preparing DHCP response broadcast address");

	socklen_t braddr_len = sizeof(struct sockaddr_in);
	struct sockaddr_in braddr;
	prepare_broadcast_addr(&braddr, hargs->resp_dest_ip, resp_port);

	char brip[32 + 1] = {0};
	format_inaddr(&braddr.sin_addr, brip, 32);
	log_debug(loggr(), TAG, "Sending DHCP response to: %s", brip);
	
	return send_dhcp_resp_packet(client_fd, response, (struct sockaddr*) &braddr, braddr_len);
}


int handle_dhcp_request(int client_fd, int resp_port, struct db_connection* db_conn, struct dhcp_config* dhcpconf, struct handler_args* hargs) {
	log_debug(loggr(), TAG, "Handling incoming DHCP request");

	struct dhcp_packet* request = dhcp_packet_req_create();
	if (request == NULL)
		return -1;

	int err = recv_dhcp_req_packet(client_fd, request);
	if (err < 0) {
		dhcp_packet_req_delete(request);
		return -1;
	}

	log_debug(loggr(), TAG, "Preparing DHCP response (xid = %#08x)", request->xid);

	struct dhcp_packet* response = dhcp_packet_resp_create();
	if (response == NULL) {
		dhcp_packet_req_delete(request);
		return -1;
	}

	err = prepare_dhcp_response(response, request, dhcpconf, db_conn);
	dhcp_packet_req_delete(request);

	if (err) {
		dhcp_packet_resp_delete(response);
		return -1;
	}

	err = send_dhcp_response(client_fd, resp_port, response, hargs);
	dhcp_packet_resp_delete(response);

	if (err < 0)
		return -1;

	return 0;
}

