#include "args.h"

#include <argp.h>
#include <string.h>


const char* argp_program_version = "DHCP v2.0.0";
const char* doc = "A custom implementation of DHCP server.\nContains DHCP client utility for testing purposes.";
const char* args_doc = NULL;


#define ARG_PORT_DEFAULT 6767
#define ARG_INTERFACE_DEFAULT "any"

#define OPT_PORT_KEY 'p'
#define OPT_INTERFACE_KEY 'i'


int parse_port_arg(char* arg) {
	if (strnlen(arg, 8) > 8)
		return -1;

	int port = -1;
	int matched = sscanf(arg, "%d", &port);

	if (matched != 1 || port < 0 || port > 65535)
		port = -1;

	return port;
}

int parse_host_arg(char* arg) {
	if (strnlen(arg, SERVER_IF_MAX_LEN + 1) > SERVER_IF_MAX_LEN)
		return -1;

	return 0;
}

static error_t parse_opt(int key, char* arg, struct argp_state* state) {
	struct arguments* args = (struct arguments*) state->input;
	int port = -1;
	int result = -1;

	switch (key) {
		case OPT_PORT_KEY:
			port = parse_port_arg(arg);
			if (port < 0)
				argp_error(state, "Invalid port number");

			args->server_port = port;
			break;

		case OPT_INTERFACE_KEY:
			result = parse_host_arg(arg);
			if (result != 0)
				argp_error(state, "Invalid host");
			
			strncpy(args->server_if, arg, SERVER_IF_MAX_LEN);
			break;

		case ARGP_KEY_ARG:
			argp_usage(state);
			break;

		case ARGP_KEY_END:
			break;

		default:
			return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

struct arguments get_parsed_arguments(int argc, char** argv) {
	struct argp_option options[] = {
		{ "port", OPT_PORT_KEY, "PORT", 0, "DHCP server port", 0 },
		{ "interface", OPT_INTERFACE_KEY, "INTERFACE", 0, "DHCP server bind interface", 0 },
		{ 0 }
	};

	struct arguments args = { ARG_PORT_DEFAULT, ARG_INTERFACE_DEFAULT, 0 };
	struct argp argp = { options, parse_opt, args_doc, doc, NULL, NULL, NULL };
	
	error_t argp_result = argp_parse(&argp, argc, argv, 0, 0, &args);
	args.parse_result = argp_result;
	
	return args;
}

