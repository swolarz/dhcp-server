#ifndef __H_ARGS
#define __H_ARGS

#include <argp.h>

#include <net/if.h>


#define SERVER_IF_MAX_LEN IFNAMSIZ
#define RESP_DEST_IP_MAX_LEN 32

struct arguments {
	int server_port;
	char server_if[SERVER_IF_MAX_LEN + 1];
	char resp_dest_ip[RESP_DEST_IP_MAX_LEN + 1];
	error_t parse_result;
};

struct arguments get_parsed_arguments(int argc, char** argv);

#endif //  __H_ARGS

