#ifndef __H_DHCP_SERVER_CONTEXT
#define __H_DHCP_SERVER_CONTEXT


struct server_args {
	int server_port;
	const char* server_if;
	const char* resp_dest_ip;
};

void dhcp_server_start(struct server_args* args);


#endif // __H_DHCP_SERVER_CONTEXT

