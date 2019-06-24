#ifndef __H_UTILS_NET
#define __H_UTILS_NET

#include <netinet/in.h>


int validate_inet_mask(struct in_addr* inmask);
int inet_mask_len(struct in_addr* inmask);


int parse_inaddr(const char* inaddr_str, struct in_addr* inaddr);
int format_inaddr(struct in_addr* inaddr, char* buffer, socklen_t size);

int format_mac_addr(unsigned char* mac, char* buffer);


#endif // __H_UTILS_NET
