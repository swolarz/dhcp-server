#include "dhcpopt.h"
#include "common.h"

#include <stdlib.h>
#include <string.h>


typedef struct dhcp_resp_options dhcp_resp_options;
typedef struct dhcp_req_options dhcp_req_options;


dhcp_resp_options* dhcp_resp_options_create() {
	dhcp_resp_options* resp_opts = malloc(sizeof(dhcp_resp_options));
	memset(resp_opts, 0, sizeof(dhcp_resp_options));

	return resp_opts;
}

dhcp_req_options* dhcp_req_options_create() {
	dhcp_req_options* req_opts = malloc(sizeof(dhcp_req_options));
	memset(req_opts, 0, sizeof(dhcp_req_options));

	return req_opts;
}

void dhcp_resp_options_delete(dhcp_resp_options* resp_opts) {
	free(resp_opts);
}

void dhcp_req_options_delete(dhcp_req_options* req_opts) {
	free(req_opts);
}


static char* dhcp_write_option(char* buffer, u_int8_t opt_tag, u_int8_t opt_size, char* opt_content) {
	memcpy(buffer, &opt_tag, sizeof(u_int8_t));
	memcpy(buffer + sizeof(u_int8_t), &opt_size, sizeof(u_int8_t));
	memcpy(buffer + 2 * sizeof(u_int8_t), opt_content, opt_size);

	return buffer + 2 * sizeof(u_int8_t) + opt_size;
}

static char* dhcp_write_end_option(char* buffer) {
	*buffer = 0xff;
	return buffer + 1;
}

size_t dhcp_resp_options_size(dhcp_resp_options* resp_opts) {
	size_t opt_size = 0;
	const size_t opt_base_len = sizeof(u_int8_t) * 2;

	opt_size += DHCP_MSG_TYPE_OPT_LEN + DHCP_SUBNET_MASK_OPT_LEN + DHCP_SERVER_ID_OPT_LEN + opt_base_len * 3;

	if (resp_opts->router_addr.s_addr)
		opt_size += DHCP_ROUTER_ADDRESS_OPT_LEN + opt_base_len;

	if (resp_opts->dns_addr.s_addr)
		opt_size += DHCP_DOMAIN_SERVER_OPT_LEN + opt_base_len;

	if (resp_opts->lease_time)
		opt_size += DHCP_ADDR_LEASE_TIME_OPT_LEN + DHCP_RENEWAL_TIME_OPT_LEN + DHCP_REBINDING_TIME_OPT_LEN + opt_base_len * 3;

	if (resp_opts->ip_ttl)
		opt_size += DHCP_DEFAULT_IP_TTL_OPT_LEN + opt_base_len;

	// DHCP end option
	opt_size += 1;

	return opt_size;
}

size_t dhcp_resp_options_marshall(dhcp_resp_options* resp_opts, char* opts_buff) {
	char* pt = opts_buff;

	pt = dhcp_write_option(pt, DHCP_MSG_TYPE_OPT, DHCP_MSG_TYPE_OPT_LEN, (char*) &resp_opts->type);
	pt = dhcp_write_option(pt, DHCP_SUBNET_MASK_OPT, DHCP_SUBNET_MASK_OPT_LEN, (char*) &resp_opts->subnet_mask);
	pt = dhcp_write_option(pt, DHCP_SERVER_ID_OPT, DHCP_SERVER_ID_OPT_LEN, (char*) &resp_opts->server_id);

	if (resp_opts->router_addr.s_addr)
		pt = dhcp_write_option(pt, DHCP_ROUTER_ADDRESS_OPT, DHCP_ROUTER_ADDRESS_OPT_LEN, (char*) &resp_opts->router_addr);

	if (resp_opts->dns_addr.s_addr)
		pt = dhcp_write_option(pt, DHCP_DOMAIN_SERVER_OPT, DHCP_DOMAIN_SERVER_OPT_LEN, (char*) &resp_opts->dns_addr);

	if (resp_opts->lease_time) {
		pt = dhcp_write_option(pt, DHCP_ADDR_LEASE_TIME_OPT, DHCP_ADDR_LEASE_TIME_OPT_LEN, (char*) &resp_opts->lease_time);
		pt = dhcp_write_option(pt, DHCP_RENEWAL_TIME_OPT, DHCP_RENEWAL_TIME_OPT_LEN, (char*) &resp_opts->renewal_time);
		pt = dhcp_write_option(pt, DHCP_REBINDING_TIME_OPT, DHCP_REBINDING_TIME_OPT_LEN, (char*) &resp_opts->rebinding_time);
	}

	if (resp_opts->ip_ttl)
		pt = dhcp_write_option(pt, DHCP_DEFAULT_IP_TTL_OPT, DHCP_DEFAULT_IP_TTL_OPT_LEN, (char*) &resp_opts->ip_ttl);

	pt = dhcp_write_end_option(pt);

	return pt - opts_buff;
}


static void dhcp_read_req_params_option_content(char* buffer, size_t opt_len, dhcp_req_options* req_opts) {
	size_t i;
	for (i = 0; i < opt_len; i += sizeof(u_int8_t)) {
		u_int8_t* param = (u_int8_t*) (buffer + i);
		req_opts->req_params[*param] = 1;
	}
}

static ssize_t dhcp_read_option(char* buffer, size_t buffsize, dhcp_req_options* req_opts) {
	// Check if DHCP end option
	if (buffsize >= sizeof(u_int8_t) && (u_int8_t) *buffer == 0xff)
		return 0;

	size_t base_size = sizeof(u_int8_t) * 2;
	u_int8_t dhcp_tag;
	u_int8_t opt_len;

	if (buffsize < base_size)
		return -1;

	memcpy(&dhcp_tag, buffer, sizeof(u_int8_t));
	memcpy(&opt_len, buffer + sizeof(u_int8_t), sizeof(u_int8_t));

	buffer += base_size;

	switch (dhcp_tag) {
		case DHCP_MSG_TYPE_OPT:
			memcpy(&req_opts->type, buffer, DHCP_MSG_TYPE_OPT_LEN);
			break;
		case DHCP_REQ_PARAMETER_LIST_OPT:
			dhcp_read_req_params_option_content(buffer, opt_len, req_opts);
			break;
		case DHCP_SERVER_ID_OPT:
			memcpy(&req_opts->server_id, buffer, DHCP_SERVER_ID_OPT_LEN);
			break;
		case DHCP_REQUESTED_IP_OPT:
			memcpy(&req_opts->req_addr, buffer, DHCP_REQUESTED_IP_OPT_LEN);
			break;
		case DHCP_HOSTNAME_OPT:
			memcpy(req_opts->hostname, buffer, opt_len);
			break;
	}

	return base_size + opt_len;
}

ssize_t dhcp_opt_parse_req(char* opt_buffer, size_t size, dhcp_req_options* req_opts) {
	size_t bytes = 0;

	while (bytes < size) {
		size_t read = dhcp_read_option(opt_buffer + bytes, size - bytes, req_opts);
		if (read == 0) // If encountered DHCP end option
			break;

		bytes += read;
	}

	if (bytes > size)
		return -1;

	return bytes;
}


int dhcp_option_format_type(u_int8_t dhcp_msg_type, char* buff, size_t buffsize) {
	UNUSED(dhcp_msg_type);
	UNUSED(buff);
	UNUSED(buffsize);
	//TODO
	return -1;
}

