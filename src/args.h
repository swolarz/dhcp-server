#ifndef __H_ARGS
#define __H_ARGS

#include <argp.h>

#include <net/if.h>


#define SERVER_IF_MAX_LEN IFNAMSIZ

struct arguments {
	int server_port;
	char server_if[SERVER_IF_MAX_LEN + 1];
	error_t parse_result;
};

struct arguments get_parsed_arguments(int argc, char** argv);

#endif //  __H_ARGS

