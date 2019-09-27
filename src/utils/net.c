#include "net.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>


int validate_inet_mask(struct in_addr* inmask) {
	int flag = 0;
	uint32_t bitmask = inmask->s_addr;

	int i;
	for (i = 0; i < 32; ++i) {
		int set = ((bitmask & (1 << i)) > 0) ? 1 : 0;

		if (flag && !set)
			return -1;

		if (!flag && set)
			flag = 1;
	}

	return 0;
}

int inet_mask_len(struct in_addr* inmask) {
	if (validate_inet_mask(inmask) != 0)
		return -1;

	uint32_t bitmask = inmask->s_addr;

	int i;
	for (i = 0; i < 32; ++i) {
		if ((bitmask & (1 << i)) > 0)
			return 32 - i;
	}

	return 0;
}

int is_same_network(struct in_addr x_addr, struct in_addr y_addr, struct in_addr mask) {
	struct in_addr x_net = { x_addr.s_addr & mask.s_addr };
	struct in_addr y_net = { y_addr.s_addr & mask.s_addr };

	return (x_net.s_addr == y_net.s_addr) ? TRUEVAL : FALSEVAL;
}


static int get_interface_any_ip(struct in_addr* ifaddr) {
	return parse_inaddr("0.0.0.0", ifaddr);
}

static int get_interface_existing_ip(const char* ifname, struct in_addr* ifaddr) {
	int fd = socket(PF_INET, SOCK_DGRAM, 0);
	struct ifreq ifr;

	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);

	int err = ioctl(fd, SIOCGIFADDR, &ifr);
	close(fd);

	if (err < 0)
		return -1;

	memcpy(ifaddr, &((struct sockaddr_in*) &ifr.ifr_addr)->sin_addr, sizeof(struct in_addr));

	return 0;
}

int get_interface_ip(const char* ifname, struct in_addr* ifaddr) {
	if (ifname == NULL || strncmp(ifname, "any", strlen("any")) == 0)
		return get_interface_any_ip(ifaddr);

	return get_interface_existing_ip(ifname, ifaddr);
}


int parse_inaddr(const char* ifaddr_str, struct in_addr* inaddr) {
	int err = inet_pton(AF_INET, ifaddr_str, inaddr);
	if (err <= 0)
		return -1;

	return 0;
}

int format_inaddr(struct in_addr* inaddr, char* buffer, socklen_t bufsize) {
	const char* result = inet_ntop(AF_INET, (const void*) inaddr, buffer, bufsize);

	if (result == NULL)
		return -1;

	return 0;
}

int format_mac_addr(unsigned char* mac, char* buffer) {
	int res = sprintf(buffer, "%02x:%02x:%02x:%02x:%02x:%02x",
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	if (res < 0)
		return -1;

	return 0;
}

