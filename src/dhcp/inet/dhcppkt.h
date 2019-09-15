#ifndef __H_DHCP_INET_DHCPPKT
#define __H_DHCP_INET_DHCPPKT

#include <netinet/in.h>


#define DHCP_CHWADDR_LEN 16
#define DHCP_SNAME_LEN 64
#define DHCP_FILE_LEN 128
#define DHCP_MAGIC_COOKIE_LEN 4
#define DHCP_OPT_LEN 4


struct dhcp_packet {
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
};

#define OP_BOOTPREQUEST 1
#define OP_BOOTPREPLY 2
#define HTYPE_ETHERNET 1
#define HWLEN_ETHERNET 6
#define BOOTP_FLAG_BROADCAST 0x8000

size_t dhcp_packet_allocation(void);
size_t dhcp_packet_size(struct dhcp_packet* pkt);

int dhcp_packet_marshall(struct dhcp_packet* pkt, char* buffer, size_t* size);
int dhcp_packet_unmarshall(char* buffer, size_t buf_size, struct dhcp_packet* pkt);


#define DHCP_INVALID_OP 1
#define DHCP_INVALID_HTYPE 2

int dhcp_packet_validate(struct dhcp_packet* dhcp_pkt);
ssize_t dhcp_packet_format(struct dhcp_packet* dhcp_pkt, char* buffer, size_t buf_size);


#endif // __H_DHCP_INET_DHCPPKT

