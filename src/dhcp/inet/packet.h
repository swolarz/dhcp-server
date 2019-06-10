#ifndef __H_DHCP_INET_PACKET
#define __H_DHCP_INET_PACKET

#include <netinet/in.h>


#define DHCP_SNAME_LEN 40
#define DHCP_FILE_LEN 128
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
	unsigned char chwaddr[16];
	char sname[DHCP_SNAME_LEN];
	char file[DHCP_FILE_LEN];
	unsigned char options[DHCP_OPT_LEN];
} __attribute__ ((packed));


struct dhcp_packet dhcp_hton(struct dhcp_packet pkt);

struct dhcp_packet dhcp_ntoh(struct dhcp_packet pkt);


#endif // __H_DHCP_INET_PACKET
