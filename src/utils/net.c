#include "net.h"

#include <stdio.h>
#include <arpa/inet.h>


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


int parse_inaddr(const char* ifaddr_str, struct in_addr* inaddr) {
	return inet_pton(AF_INET, ifaddr_str, inaddr);
}

int format_inaddr(struct in_addr* inaddr, char* buffer, socklen_t size) {
	const char* result = inet_ntop(AF_INET, (const void*) inaddr, buffer, size);

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

