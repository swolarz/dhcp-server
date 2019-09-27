#include "rules.h"
#include "context.h"
#include "common.h"
#include "config/dhcpconf.h"
#include "dhcp/data/store.h"
#include "dhcp/inet/dhcppkt.h"
#include "dhcp/inet/dhcpopt.h"
#include "utils/net.h"
#include "utils/log/log.h"

#include <string.h>
#include <arpa/inet.h>


typedef struct dhcp_packet dhcp_packet;
typedef struct dhcp_config dhcp_config;
typedef struct dhcp_resp_options dhcp_resp_options;
typedef struct dhcp_req_options dhcp_req_options;
typedef struct db_connection db_connection;


static const char* TAG = "RULES";

static struct logger* loggr() {
	return context_get_logger();
}


typedef struct dhcp_packet dhcp_packet;
typedef struct dhcp_config dhcp_config;


static int fill_base_response(dhcp_packet* dhcp_req, dhcp_packet* dhcp_resp, dhcp_config* dhcp_conf) {
	log_debug(loggr(), TAG, "Copying response basic BOOTP fields");

	dhcp_resp->op = OP_BOOTPREPLY;
	dhcp_resp->htype = HTYPE_ETHERNET;
	dhcp_resp->hwlen = HWLEN_ETHERNET;
	dhcp_resp->xid = dhcp_req->xid;
	dhcp_resp->secs = 0;
	dhcp_resp->flags = BOOTP_FLAG_BROADCAST;
	
	memcpy(dhcp_resp->chwaddr, dhcp_req->chwaddr, sizeof(dhcp_req->chwaddr));
	memcpy(&dhcp_resp->siaddr, &dhcp_conf->dhcp_server_id, sizeof(struct in_addr));

	dhcp_resp_options* resp_opts = (dhcp_resp_options*) dhcp_resp->options;
	memcpy(&resp_opts->server_id, &dhcp_conf->dhcp_server_id, sizeof(struct in_addr));

	return DHCP_PROCESS_OK;
}

static int fill_address_lease_time(dhcp_resp_options* resp_opts, dhcp_config* dhcp_conf) {
	u_int32_t lease_time = dhcp_conf->lease_time_secs;
	u_int32_t renewal_time = (u_int32_t) (lease_time * 0.5);
	u_int32_t rebinding_time = (u_int32_t) (lease_time * 0.875);

	resp_opts->lease_time = htonl(lease_time);
	resp_opts->renewal_time = htonl(renewal_time);
	resp_opts->rebinding_time = htonl(rebinding_time);

	return DHCP_PROCESS_OK;
}

static int fill_ip_address(struct in_addr ipaddr, dhcp_packet* dhcp_resp, dhcp_config* dhcp_conf) {
	dhcp_resp_options* resp_opts = (dhcp_resp_options*) dhcp_resp->options;

	memcpy(&dhcp_resp->yiaddr, &ipaddr, sizeof(struct in_addr));
	memcpy(&resp_opts->subnet_mask, &dhcp_conf->net_mask, sizeof(struct in_addr));

	return fill_address_lease_time(resp_opts, dhcp_conf);
}

static int fill_request_params(dhcp_resp_options* resp_opts, dhcp_req_options* req_opts, dhcp_config* dhcp_conf) {
	if (req_opts->req_params[DHCP_ROUTER_ADDRESS_OPT] == TRUEVAL)
		memcpy(&resp_opts->router_addr, &dhcp_conf->net_gateway, sizeof(struct in_addr));
	
	if (req_opts->req_params[DHCP_DOMAIN_SERVER_OPT] == TRUEVAL)
		memcpy(&resp_opts->dns_addr, &dhcp_conf->net_dns_addr, sizeof(struct in_addr));

	if (req_opts->req_params[DHCP_DEFAULT_IP_TTL_OPT] == TRUEVAL)
		resp_opts->ip_ttl = dhcp_conf->default_ip_ttl;

	return DHCP_PROCESS_OK;
}


static int dhcp_make_ip_response(dhcp_packet* dhcp_resp, dhcp_packet* dhcp_req, struct in_addr yaddr, dhcp_config* dhcp_conf) {
	dhcp_resp_options* resp_opts = (dhcp_resp_options*) dhcp_resp->options;
	dhcp_req_options* req_opts = (dhcp_req_options*) dhcp_req->options;

	int err = fill_base_response(dhcp_req, dhcp_resp, dhcp_conf);
	if (err != DHCP_PROCESS_OK)
		return err;

	err = fill_ip_address(yaddr, dhcp_resp, dhcp_conf);
	if (err != DHCP_PROCESS_OK)
		return err;

	return fill_request_params(resp_opts, req_opts, dhcp_conf);
}

static int dhcp_make_ip_offer_response(dhcp_packet* dhcp_resp, dhcp_packet* dhcp_req, struct in_addr yaddr, dhcp_config* dhcp_conf) {
	dhcp_resp_options* resp_opts = (dhcp_resp_options*) dhcp_resp->options;
	resp_opts->type = DHCP_OFFER_MSG;

	return dhcp_make_ip_response(dhcp_resp, dhcp_req, yaddr, dhcp_conf);
}

static int dhcp_make_ip_ack_response(dhcp_packet* dhcp_resp, dhcp_packet* dhcp_req, struct in_addr yaddr, dhcp_config* dhcp_conf) {
	dhcp_resp_options* resp_opts = (dhcp_resp_options*) dhcp_resp->options;
	resp_opts->type = DHCP_ACK_MSG;

	return dhcp_make_ip_response(dhcp_resp, dhcp_req, yaddr, dhcp_conf);
}

static int dhcp_make_nak_response(dhcp_packet* dhcp_resp, dhcp_packet* dhcp_req, dhcp_config* dhcp_conf) {
	dhcp_resp_options* resp_opts = (dhcp_resp_options*) dhcp_resp->options;
	resp_opts->type = DHCP_NAK_MSG;

	int err = fill_base_response(dhcp_req, dhcp_resp, dhcp_conf);
	if (err != DHCP_PROCESS_OK)
		return err;

	return DHCP_PROCESS_OK;
}

static int dhcp_process_discover_msg(dhcp_packet* dhcp_resp, dhcp_packet* dhcp_req, dhcp_config* dhcp_conf, db_connection* db_conn) {
	log_debug(loggr(), TAG, "Processing DHCP DISCOVER request");

	char ipaddr_fmt[32 + 1] = { 0 };
	dhcp_req_options* req_opts = (dhcp_req_options*) dhcp_req->options;

	struct in_addr yaddr;

	int lease_err = DHCP_LEASE_IP_IN_USE;
	if (req_opts->req_addr.s_addr) {
		format_inaddr(&req_opts->req_addr, ipaddr_fmt, 32);
		log_info(loggr(), TAG, "Client requested ip address: %s", ipaddr_fmt);

		if (is_same_network(req_opts->req_addr, dhcp_conf->net_addr, dhcp_conf->net_mask) == TRUEVAL)
			lease_err = dhcp_lease_is_available_ip(db_conn, yaddr, dhcp_req->chwaddr);
	}

	if (lease_err == DHCP_LEASE_IP_IN_USE) {
		int err = dhcp_lease_allocate_temp_ip(db_conn, &yaddr, dhcp_req->chwaddr);

		if (err == DHCP_LEASE_OK) {
			format_inaddr(&yaddr, ipaddr_fmt, 32);
			log_info(loggr(), TAG, "Resolved IP address for client (%s): %s", req_opts->hostname, ipaddr_fmt);
		}
		else if (err == DHCP_LEASE_NO_AVAILABLE) {
			log_warn(loggr(), TAG, "No available IP addresses left");
			return DHCP_PROCESS_IGNORE;
		}
		else {
			log_error(loggr(), TAG, "Error occurred during resolution of an available IP address");
			return DHCP_PROCESS_FAILED;
		}
	}

	return dhcp_make_ip_offer_response(dhcp_resp, dhcp_req, yaddr, dhcp_conf);
}

static int dhcp_process_request_msg(dhcp_packet* dhcp_resp, dhcp_packet* dhcp_req, dhcp_config* dhcp_conf, db_connection* db_conn) {
	log_debug(loggr(), TAG, "Processing DHCP REQUEST request");

	dhcp_req_options* req_opts = (dhcp_req_options*) dhcp_req->options;
	struct in_addr yaddr = req_opts->req_addr;

	char ipaddr_fmt[32 + 1] = { 0 };
	format_inaddr(&yaddr, ipaddr_fmt, 32);

	int lease_err = DHCP_LEASE_IP_IN_USE;
	if (is_same_network(req_opts->req_addr, dhcp_conf->net_addr, dhcp_conf->net_mask) == TRUEVAL)
		lease_err = dhcp_lease_allocate_ip(db_conn, yaddr, dhcp_req->chwaddr);

	if (lease_err == DHCP_LEASE_FAILED) {
		log_error(loggr(), TAG, "Failed to allocate requested IP address (%s)", ipaddr_fmt);
		return DHCP_PROCESS_FAILED;
	}

	if (lease_err == DHCP_LEASE_IP_IN_USE) {
		return dhcp_make_nak_response(dhcp_resp, dhcp_req, dhcp_conf);
	}

	return dhcp_make_ip_ack_response(dhcp_resp, dhcp_req, yaddr, dhcp_conf);
}

static int dhcp_process_release_msg(dhcp_packet* dhcp_req, db_connection* db_conn) {
	log_debug(loggr(), TAG, "Processing DHCP RELEASE request");

	dhcp_req_options* req_opts = (dhcp_req_options*) dhcp_req->options;
	// TODO error handling
	dhcp_lease_release_ip(db_conn, req_opts->req_addr, dhcp_req->chwaddr);
	
	return DHCP_LEASE_OK;
}

static int dhcp_process_inform_msg(dhcp_packet* dhcp_resp, dhcp_packet* dhcp_req, dhcp_config* dhcp_conf) {
	log_debug(loggr(), TAG, "Processing DHCP INFORM request");

	dhcp_resp_options* resp_opts = (dhcp_resp_options*) dhcp_resp->options;
	dhcp_req_options* req_opts = (dhcp_req_options*) dhcp_req->options;

	fill_base_response(dhcp_req, dhcp_resp, dhcp_conf);
	fill_address_lease_time(resp_opts, dhcp_conf);
	fill_request_params(resp_opts, req_opts, dhcp_conf);

	return DHCP_PROCESS_OK;
}


int dhcp_req_resp_process_pipeline(dhcp_packet* dhcp_req, dhcp_packet* dhcp_resp, dhcp_config* dhcp_conf, db_connection* db_conn) {
	char msg_type_fmt[32 + 1] = { 0 };
	dhcp_req_options* req_opts = (dhcp_req_options*) dhcp_req->options;

	dhcp_option_format_type(req_opts->type, msg_type_fmt, 32);
	log_info(loggr(), TAG, "Handling DHCP request: %s", msg_type_fmt);

	// Ignore request if not corresponding server id
	if (req_opts->server_id.s_addr && memcmp(&req_opts->server_id, &dhcp_conf->dhcp_server_id, sizeof(struct in_addr)) != 0) {
		char target_server[32 + 1] = { 0 };
		char actual_server[32 + 1] = { 0 };

		format_inaddr(&req_opts->server_id, target_server, 32);
		format_inaddr(&dhcp_conf->dhcp_server_id, actual_server, 32);

		log_info(loggr(), TAG, "The server is not a target of the request. Target: %s (actual: %s)", target_server, actual_server);

		return DHCP_PROCESS_IGNORE;
	}

	int err = DHCP_PROCESS_OK;
	switch (req_opts->type) {
		case DHCP_DISCOVER_MSG:
			err = dhcp_process_discover_msg(dhcp_resp, dhcp_req, dhcp_conf, db_conn);
			break;

		case DHCP_REQUEST_MSG:
			err = dhcp_process_request_msg(dhcp_resp, dhcp_req, dhcp_conf, db_conn);
			break;

		case DHCP_DECLINE_MSG:
			log_warn(loggr(), TAG, "Received DECLINE packet from: %s", req_opts->hostname);
			err = DHCP_PROCESS_IGNORE;
			break;

		case DHCP_RELEASE_MSG:
			err = dhcp_process_release_msg(dhcp_req, db_conn);
			break;

		case DHCP_INFORM_MSG:
			err = dhcp_process_inform_msg(dhcp_resp, dhcp_req, dhcp_conf);
			break;

		default:
			// formatting ignored request message type
			dhcp_option_format_type(req_opts->type, msg_type_fmt, 32);
			log_info(loggr(), TAG, "Ignoring request message with type: %s", msg_type_fmt);

			err = DHCP_PROCESS_IGNORE;
	}

	return err;
}

