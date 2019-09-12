#include "dhcppkt.h"
#include "common.h"
#include "utils/net.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>


typedef struct dhcp_packet dhcp_packet;

struct dhcp_packet_packed {
	u_int8_t op;
	u_int8_t htype;
	u_int8_t hwlen;
	u_int8_t hops;
	u_int32_t xid;
	u_int16_t secs;
	u_int16_t flags;
	struct in_addr ciaddr;
	struct in_addr yiaddr;
	struct in_addr siaddr;
	struct in_addr giaddr;
	unsigned char chwaddr[DHCP_CHWADDR_LEN];
	char sname[DHCP_SNAME_LEN];
	char file[DHCP_FILE_LEN];
	char magic[DHCP_MAGIC_COOKIE_LEN];
	unsigned char options[DHCP_OPT_LEN];
} __attribute__ ((packed));

typedef struct dhcp_packet_packed dhcp_pkt_packed;


size_t dhcp_packet_allocation(void) {
	return sizeof(dhcp_pkt_packed) + 1;
}

size_t dhcp_packet_size(struct dhcp_packet* pkt) {
	UNUSED(pkt);
	return sizeof(dhcp_pkt_packed);
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

static void dhcp_packet_copy_tx(dhcp_pkt_packed* pkt_tx_packed, dhcp_packet* pkt_tx) {
	pkt_tx_packed->op = pkt_tx->op;
	pkt_tx_packed->htype = pkt_tx->htype;
	pkt_tx_packed->hwlen = pkt_tx->hwlen;
	pkt_tx_packed->hops = pkt_tx->hops;
	pkt_tx_packed->xid = pkt_tx->xid;
	pkt_tx_packed->secs  = pkt_tx->secs;
	pkt_tx_packed->flags = pkt_tx->flags;

	memcpy(&(pkt_tx_packed->ciaddr), &(pkt_tx->ciaddr), sizeof(struct in_addr));
	memcpy(&(pkt_tx_packed->yiaddr), &(pkt_tx->yiaddr), sizeof(struct in_addr));
	memcpy(&(pkt_tx_packed->siaddr), &(pkt_tx->siaddr), sizeof(struct in_addr));
	memcpy(&(pkt_tx_packed->giaddr), &(pkt_tx->giaddr), sizeof(struct in_addr));

	memcpy(pkt_tx_packed->chwaddr, pkt_tx->chwaddr, sizeof(pkt_tx->chwaddr));
	memcpy(pkt_tx_packed->sname, pkt_tx->sname, sizeof(pkt_tx->sname));
	memcpy(pkt_tx_packed->file, pkt_tx->file, sizeof(pkt_tx->file));
	memcpy(pkt_tx_packed->options, pkt_tx->options, sizeof(pkt_tx->options));
}

static void dhcp_packet_copy_rx(dhcp_packet* pkt_rx, dhcp_pkt_packed* pkt_rx_packed) {
	pkt_rx->op = pkt_rx_packed->op;
	pkt_rx->htype = pkt_rx_packed->htype;
	pkt_rx->hwlen = pkt_rx_packed->hwlen;
	pkt_rx->hops = pkt_rx_packed->hops;
	pkt_rx->xid = pkt_rx_packed->xid;
	pkt_rx->secs  = pkt_rx_packed->secs;
	pkt_rx->flags = pkt_rx_packed->flags;

	memcpy(&(pkt_rx->ciaddr), &(pkt_rx_packed->ciaddr), sizeof(struct in_addr));
	memcpy(&(pkt_rx->yiaddr), &(pkt_rx_packed->yiaddr), sizeof(struct in_addr));
	memcpy(&(pkt_rx->siaddr), &(pkt_rx_packed->siaddr), sizeof(struct in_addr));
	memcpy(&(pkt_rx->giaddr), &(pkt_rx_packed->giaddr), sizeof(struct in_addr));

	memcpy(pkt_rx->chwaddr, pkt_rx_packed->chwaddr, sizeof(pkt_rx->chwaddr));
	memcpy(pkt_rx->sname, pkt_rx_packed->sname, sizeof(pkt_rx->sname));
	memcpy(pkt_rx->file, pkt_rx_packed->file, sizeof(pkt_rx->file));
	memcpy(pkt_rx->options, pkt_rx_packed->options, sizeof(pkt_rx->options));
}

static void dhcp_packet_fill_cookie(dhcp_packet* pkt) {
	static char magic_cookie[] = { 0x63, 0x82, 0x53, 0x63 };
	memcpy(pkt->magic, magic_cookie, sizeof(magic_cookie));
}


int dhcp_packet_marshall(dhcp_packet* pkt, char* buffer, size_t* size) {
	size_t real_size = dhcp_packet_size(pkt);
	if (*size < real_size)
		return -1;

	dhcp_packet_fill_cookie(pkt);
	dhcp_packet_hton(pkt);

	dhcp_pkt_packed* pkt_packed = (dhcp_pkt_packed*) buffer;
	dhcp_packet_copy_tx(pkt_packed, pkt);

	*size = real_size;

	return 0;
}

int dhcp_packet_unmarshall(char* buffer, size_t buf_size, dhcp_packet* pkt) {
	if (buf_size < sizeof(dhcp_pkt_packed))
		return -1;

	dhcp_pkt_packed* pkt_packed = (dhcp_pkt_packed*) buffer;
	dhcp_packet_copy_rx(pkt, pkt_packed);

	dhcp_packet_ntoh(pkt);

	return 0;
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
	char* flags_fmt = dhcp_fmt_flags(dhcp_pkt->flags);
	char* caddr_fmt = dhcp_fmt_addr(&(dhcp_pkt->ciaddr));
	char* yaddr_fmt = dhcp_fmt_addr(&(dhcp_pkt->yiaddr));
	char* gaddr_fmt = dhcp_fmt_addr(&(dhcp_pkt->giaddr));
	char* hwaddr_fmt = dhcp_fmt_hwaddr(dhcp_pkt->chwaddr, dhcp_pkt->htype, dhcp_pkt->hwlen);

	ssize_t bytes = snprintf(buffer, buf_size,
			"OP: %s\nHTYPE: %s\nHWLEN: %hhu\nHOPS: %hhu\nXID: %#08x\nSECS: %hu\nFLAGS: %s\n"
			"CLIENT ADDR: %s\nASSIGNED ADDR: %s\nGATEWAY ADDR: %s\nHARDWARE ADDR: %s",
			dhcp_fmt_op(dhcp_pkt->op), dhcp_fmt_htype(dhcp_pkt->htype), dhcp_pkt->hwlen, dhcp_pkt->hops, dhcp_pkt->xid,
			dhcp_pkt->secs, flags_fmt, caddr_fmt, yaddr_fmt, gaddr_fmt, hwaddr_fmt);

	free(flags_fmt);
	free(caddr_fmt);
	free(yaddr_fmt);
	free(gaddr_fmt);
	free(hwaddr_fmt);

	return bytes;
}


