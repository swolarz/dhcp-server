#ifndef __H_DHCP_CLIENT_CONTEXT
#define __H_DHCP_CLIENT_CONTEXT


struct client_args {
	int server_port;
	const char* server_host;
};

void dhcp_client_start(struct client_args args);

#endif // __H_DHCP_CLIENT_CONTEXT

