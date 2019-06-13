#ifndef __H_ARGS
#define __H_ARGS

#include <argp.h>


#define SERVER_HOST_MAX_LEN 255


struct arguments {
	int server_port;
	char server_host[SERVER_HOST_MAX_LEN + 1];
	error_t parse_result;
};

struct arguments get_parsed_arguments(int argc, char** argv);

#endif //  __H_ARGS

