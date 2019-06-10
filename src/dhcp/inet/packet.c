#include "packet.h"

#include <string.h>
#include <netinet/in.h>


typedef struct dhcp_packet dhcp_packet;


dhcp_packet dhcp_hton(dhcp_packet pkt_host) {
	dhcp_packet pkt_net;
	memcpy(&pkt_net, &pkt_host, sizeof(dhcp_packet));

	pkt_net.xid = htonl(pkt_net.xid);
	pkt_net.secs = htons(pkt_net.secs);
	pkt_net.flags = htons(pkt_net.flags);

	return pkt_net;
}

dhcp_packet dhcp_ntoh(dhcp_packet pkt_net) {
	dhcp_packet pkt_host;
	memcpy(&pkt_host, &pkt_net, sizeof(dhcp_packet));
	
	pkt_host.xid = ntohl(pkt_host.xid);
	pkt_host.secs = ntohs(pkt_host.secs);
	pkt_host.flags = ntohs(pkt_host.flags);

	return pkt_host;
}
