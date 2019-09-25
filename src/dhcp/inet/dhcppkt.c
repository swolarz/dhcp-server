#include "dhcppkt.h"
#include "dhcp/inet/dhcpopt.h"
#include "context.h"
#include "common.h"
#include "utils/net.h"
#include "utils/log/log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>


typedef struct dhcp_packet dhcp_packet;


const char* TAG = "DHCP PKT";

static struct logger* loggr() {
	return context_get_logger();
}


static size_t dhcp_packet_noopt_size() {
	static dhcp_packet pkt;
	return sizeof(dhcp_packet) - sizeof(pkt.options);
}

size_t dhcp_resp_packet_size(dhcp_packet* dhcp_resp) {
	struct dhcp_resp_options* dhcp_resp_opts = (struct dhcp_resp_options*) dhcp_resp->options;
	return dhcp_packet_noopt_size() + dhcp_resp_options_size(dhcp_resp_opts);
}

static dhcp_packet* dhcp_packet_allocate() {
	dhcp_packet* pkt = malloc(sizeof(dhcp_packet));
	memset(pkt, 0, sizeof(dhcp_packet));

	return pkt;
}

dhcp_packet* dhcp_packet_resp_create() {
	dhcp_packet* dhcp_resp = dhcp_packet_allocate();
	struct dhcp_resp_options* resp_opts = dhcp_resp_options_create();

	dhcp_resp->options = (void*) resp_opts;

	return dhcp_resp;
}

dhcp_packet* dhcp_packet_req_create() {
	dhcp_packet* dhcp_req = dhcp_packet_allocate();
	struct dhcp_req_options* req_opts = dhcp_req_options_create();

	dhcp_req->options = (void*) req_opts;

	return dhcp_req;
}

void dhcp_packet_resp_delete(struct dhcp_packet* dhcp_resp) {
	struct dhcp_resp_options* resp_opts = (struct dhcp_resp_options*) dhcp_resp->options;
	dhcp_resp_options_delete(resp_opts);

	free(dhcp_resp);
}

void dhcp_packet_req_delete(struct dhcp_packet* dhcp_req) {
	struct dhcp_req_options* req_opts = (struct dhcp_req_options*) dhcp_req->options;
	dhcp_req_options_delete(req_opts);

	free(dhcp_req);
}


static void dhcp_packet_hton(dhcp_packet* pkt_host) {
	pkt_host->xid = htonl(pkt_host->xid);
	pkt_host->secs = htons(pkt_host->secs);
	pkt_host->flags = htons(pkt_host->flags);
}

static void dhcp_packet_ntoh(dhcp_packet* pkt_net) {
	pkt_net->xid = ntohl(pkt_net->xid);
	pkt_net->secs = ntohs(pkt_net->secs);
	pkt_net->flags = ntohs(pkt_net->flags);
}

int dhcp_packet_validate(dhcp_packet* dhcp_pkt) {
	if (dhcp_pkt->op != 1 && dhcp_pkt->op != 2)
		return DHCP_INVALID_OP;

	if (dhcp_pkt->htype < 1 || dhcp_pkt->htype > 28)
		return DHCP_INVALID_HTYPE;

	//TODO

	return 0;
}

static void dhcp_packet_fill_cookie(dhcp_packet* pkt) {
	static char magic_cookie[] = { 0x63, 0x82, 0x53, 0x63 };
	memcpy(pkt->magic, magic_cookie, sizeof(magic_cookie));
}


ssize_t dhcp_packet_resp_marshall(dhcp_packet* response, char* buffer, size_t* size) {
	if (response == NULL || buffer == NULL)
		return -1;

	size_t real_size = dhcp_resp_packet_size(response);
	if (*size < real_size)
		return -1;

	log_debug(loggr(), TAG, "Calculated DHCP response packet size: %zu", real_size);

	size_t options_offset = dhcp_packet_noopt_size();
	memcpy(buffer, response, options_offset);

	dhcp_packet* dhcp_resp = (dhcp_packet*) buffer;
	dhcp_packet_fill_cookie(dhcp_resp);
	dhcp_packet_hton(dhcp_resp);

	struct dhcp_resp_options* resp_opts = (struct dhcp_resp_options*) response->options;
	size_t opts_size = dhcp_resp_options_marshall(resp_opts, buffer + options_offset);

	log_debug(loggr(), TAG, "Marshalled DHCP response packet size: %zu", options_offset + opts_size);

	return *size = real_size;
}

ssize_t dhcp_packet_req_unmarshall(char* buffer, size_t buf_size, dhcp_packet* dhcp_req) {
	if (buffer == NULL || dhcp_req == NULL)
		return -1;

	size_t options_offset = dhcp_packet_noopt_size();
	if (buf_size <= options_offset)
		return -1;

	memcpy(dhcp_req, buffer, options_offset);
	dhcp_packet_ntoh(dhcp_req);

	struct dhcp_req_options* req_opts = (struct dhcp_req_options*) dhcp_req->options;

	char* options_buffer = buffer + options_offset;
	size_t options_size = buf_size - options_offset;

	ssize_t parsed_bytes = dhcp_opt_parse_req(options_buffer, options_size, req_opts);
	if (parsed_bytes < 0)
		return -1;

	return options_offset + parsed_bytes;
}


static const char* dhcp_fmt_op(u_int8_t op) {
	if (op == 1)
		return "BOOTREQUEST";

	if (op == 2)
		return "BOOTREPLY";

	return "(unknown)";
}

static const char* dhcp_fmt_htype(u_int8_t htype) {
	if (htype == 1)
		return "ETHERNET";

	if (htype < 1 || htype > 28)
		return "(unknown)";

	return "(other)";
}

static char* dhcp_fmt_flags(u_int16_t flags) {
	const char flags_fmt[] = "0000000000000000";
	char* flags_buf = malloc(sizeof(flags_fmt));
	strcpy(flags_buf, flags_fmt);

	int i;
	for (i = 0; i < 16; ++i) {
		if ((flags & (1 << i)) > 0)
			flags_buf[15 - i] = '1';
	}

	return flags_buf;
}

static char* dhcp_fmt_addr(struct in_addr* addr) {
	char* addr_buf = malloc(64);
	int err = format_inaddr(addr, addr_buf, 64);
	
	if (err < 0)
		strcpy(addr_buf, "(invalid)");

	return addr_buf;
}

static char* dhcp_fmt_hwaddr(unsigned char* hwaddr, u_int8_t htype, u_int8_t hwlen) {
	char* hwaddr_buf = malloc(64);

	if (htype == 1) {
		int err = format_mac_addr(hwaddr, hwaddr_buf);
		if (err < 0 || hwlen != 6)
			strcpy(hwaddr_buf, "(invalid)");
	}
	else
		strcpy(hwaddr_buf, "(not mac)");

	return hwaddr_buf;
}

ssize_t dhcp_packet_format(dhcp_packet* dhcp_pkt, char* buffer, size_t buf_size) {
	struct in_addr ciaddr = dhcp_pkt->ciaddr;
	struct in_addr yiaddr = dhcp_pkt->yiaddr;
	struct in_addr siaddr = dhcp_pkt->siaddr;

	char* flags_fmt = dhcp_fmt_flags(dhcp_pkt->flags);
	char* caddr_fmt = dhcp_fmt_addr(&ciaddr);
	char* yaddr_fmt = dhcp_fmt_addr(&yiaddr);
	char* saddr_fmt = dhcp_fmt_addr(&siaddr);
	char* hwaddr_fmt = dhcp_fmt_hwaddr(dhcp_pkt->chwaddr, dhcp_pkt->htype, dhcp_pkt->hwlen);

	ssize_t bytes = snprintf(buffer, buf_size,
			"OP: %s\nHTYPE: %s\nHWLEN: %hhu\nHOPS: %hhu\nXID: %#08x\nSECS: %hu\nFLAGS: %s\n"
			"CLIENT ADDR: %s\nASSIGNED ADDR: %s\nSERVER ADDR: %s\nHARDWARE ADDR: %s",
			dhcp_fmt_op(dhcp_pkt->op), dhcp_fmt_htype(dhcp_pkt->htype), dhcp_pkt->hwlen, dhcp_pkt->hops, dhcp_pkt->xid,
			dhcp_pkt->secs, flags_fmt, caddr_fmt, yaddr_fmt, saddr_fmt, hwaddr_fmt);

	free(flags_fmt);
	free(caddr_fmt);
	free(yaddr_fmt);
	free(saddr_fmt);
	free(hwaddr_fmt);

	return bytes;
}

