#include "rules.h"
#include "context.h"
#include "config/dhcpconf.h"
#include "dhcp/data/store.h"
#include "dhcp/inet/dhcppkt.h"
#include "dhcp/inet/dhcpopt.h"
#include "utils/log/log.h"

#include <string.h>
#include <arpa/inet.h>


typedef struct dhcp_packet dhcp_packet;
typedef struct dhcp_config dhcp_config;


static const char* TAG = "RULES";

static struct logger* loggr() {
	return context_get_logger();
}


typedef struct dhcp_packet dhcp_packet;
typedef struct dhcp_config dhcp_config;


static int fill_base_response(dhcp_packet* dhcp_req, dhcp_packet* dhcp_resp) {
	log_debug(loggr(), TAG, "Copying response basic BOOTP fields");

	dhcp_resp->op = OP_BOOTPREPLY;
	dhcp_resp->htype = HTYPE_ETHERNET;
	dhcp_resp->hwlen = HWLEN_ETHERNET;
	dhcp_resp->xid = dhcp_req->xid;
	dhcp_resp->secs = 0;
	dhcp_resp->flags = BOOTP_FLAG_BROADCAST;

	return 0;
}

static int fill_address_lease_time(struct dhcp_resp_options* resp_opts, dhcp_config* dhcp_conf) {
	u_int32_t lease_time = dhcp_conf->lease_time_secs;
	u_int32_t renewal_time = (u_int32_t) (lease_time * 0.5);
	u_int32_t rebinding_time = (u_int32_t) (lease_time * 0.875);

	resp_opts->lease_time = htonl(lease_time);
	resp_opts->renewal_time = htonl(renewal_time);
	resp_opts->rebinding_time = htonl(rebinding_time);

	return 0;
}

static int fill_default_ip_ttl(struct dhcp_resp_options* resp_opts, dhcp_config* dhcp_conf) {
	resp_opts->ip_ttl = dhcp_conf->default_ip_ttl;
	return 0;
}

int dhcp_req_resp_process_pipeline(dhcp_packet* dhcp_req, dhcp_packet* dhcp_resp, dhcp_config* dhcp_conf) {
	int err = fill_base_response(dhcp_req, dhcp_resp);
	if (err) {
		log_error(loggr(), TAG, "Failed to set basic BOOTP params");
		return -1;
	}

	memcpy(&(dhcp_resp->yiaddr), &dhcp_conf->net_addr, sizeof(struct in_addr));
	memcpy(&(dhcp_resp->siaddr), &dhcp_conf->dhcp_server_id, sizeof(struct in_addr));
	memcpy(dhcp_resp->chwaddr, dhcp_req->chwaddr, sizeof(dhcp_req->chwaddr));

	log_debug(loggr(), TAG, "Constructing DHCP response options");

	struct dhcp_req_options* req_opts = (struct dhcp_req_options*) dhcp_req->options;
	struct dhcp_resp_options* resp_opts = (struct dhcp_resp_options*) dhcp_resp->options;

	resp_opts->type = DHCP_OFFER_MSG;

	memcpy(&resp_opts->server_id, &req_opts->server_id, sizeof(struct in_addr));
	memcpy(&resp_opts->subnet_mask, &dhcp_conf->net_mask, sizeof(struct in_addr));
	
	fill_address_lease_time(resp_opts, dhcp_conf);
	fill_default_ip_ttl(resp_opts, dhcp_conf);

	return 0;
}

