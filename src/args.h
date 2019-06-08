#ifndef __H_ARGS
#define __H_ARGS

#include <argp.h>


#define SERVER_HOST_MAX_LEN 255

#define MODE_SERVER_ARG "server"
#define MODE_CLIENT_ARG "client"

enum exec_mode {
	MODE_SERVER,
	MODE_CLIENT,
	MODE_UNKNOWN
};

struct arguments {
	enum exec_mode mode;
	int server_port;
	char server_host[SERVER_HOST_MAX_LEN + 1];
	error_t parse_result;
};

struct arguments get_parsed_arguments(int argc, char** argv);

#endif //  __H_ARGS

