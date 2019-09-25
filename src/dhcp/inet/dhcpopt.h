#ifndef __H_DHCP_INET_DHCPOPT
#define __H_DHCP_INET_DHCPOPT

#include <stddef.h>
#include <stdint.h>
#include <netinet/in.h>


#define DHCP_DISCOVER_MSG 1
#define DHCP_OFFER_MSG 2
#define DHCP_REQUEST_MSG 3
#define DHCP_DECLINE_MSG 4
#define DHCP_ACK_MSG 5
#define DHCP_NAK_MSG 6
#define DHCP_RELEASE_MSG 7
#define DHCP_INFORM_MSG 8


#define DHCP_SUBNET_MASK_OPT 1
#define DHCP_ROUTER_ADDRESS_OPT 3
#define DHCP_DOMAIN_SERVER_OPT 6
#define DHCP_HOSTNAME_OPT 12
#define DHCP_DEFAULT_IP_TTL_OPT 23
#define DHCP_REQUESTED_IP_OPT 50
#define DHCP_ADDR_LEASE_TIME_OPT 51
#define DHCP_MSG_TYPE_OPT 53
#define DHCP_SERVER_ID_OPT 54
#define DHCP_REQ_PARAMETER_LIST_OPT 55
#define DHCP_RENEWAL_TIME_OPT 58
#define DHCP_REBINDING_TIME_OPT 59
#define DHCP_CLIENT_ID_OPT 61
#define DHCP_END_OPT 255

#define DHCP_MSG_TYPE_OPT_LEN 1
#define DHCP_SUBNET_MASK_OPT_LEN 4
#define DHCP_SERVER_ID_OPT_LEN 4
#define DHCP_REQUESTED_IP_OPT_LEN 4
#define DHCP_ROUTER_ADDRESS_OPT_LEN 4
#define DHCP_DOMAIN_SERVER_OPT_LEN 4
#define DHCP_ADDR_LEASE_TIME_OPT_LEN 4
#define DHCP_RENEWAL_TIME_OPT_LEN 4
#define DHCP_REBINDING_TIME_OPT_LEN 4
#define DHCP_DEFAULT_IP_TTL_OPT_LEN 1


struct dhcp_resp_options {
	u_int8_t type; // message type e.g. DHCP DISCOVER/OFFER/ACK
	u_int8_t ip_ttl; // default IP Time-to-Live
	struct in_addr server_id; // DHCP server identifier (IP)
	struct in_addr subnet_mask; // subnet mask of the assigned IP address
	struct in_addr router_addr; // gateway (router) IP address
	struct in_addr dns_addr; // IP address of DNS server
	u_int32_t lease_time;	// IP address lease time in seconds
	u_int32_t renewal_time; // time (in seconds) after which the client tries to renew the lease of IP address
	u_int32_t rebinding_time; // time (in seconds) after which the client tries to re-acquire the actual address from all servers
};


#define DHCP_REQ_HOSTNAME_MAX_LEN 256

struct dhcp_req_options {
	u_int8_t type; // message type e.g. DHCP DISCOVER/OFFER/ACK
	u_int8_t req_params[255]; // represents requested parameters by the client
	struct in_addr server_id; // DHCP server identifier (IP)
	struct in_addr req_addr; // IP address requested by the client
	char hostname[DHCP_REQ_HOSTNAME_MAX_LEN + 1]; // client hostname
};


struct dhcp_resp_options* dhcp_resp_options_create(void);
struct dhcp_req_options* dhcp_req_options_create(void);

void dhcp_resp_options_delete(struct dhcp_resp_options* resp_opts);
void dhcp_req_options_delete(struct dhcp_req_options* req_opts);


size_t dhcp_resp_options_size(struct dhcp_resp_options* resp_opts);
size_t dhcp_resp_options_marshall(struct dhcp_resp_options* resp_opts, char* opts_buff);

ssize_t dhcp_opt_parse_req(char* opt_buffer, size_t size, struct dhcp_req_options* opt);

int dhcp_option_format_type(u_int8_t dhcp_msg_type, char* buff, size_t buffsize);


#endif // __H_DHCP_INET_DHCPOPT

