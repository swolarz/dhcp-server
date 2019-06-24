#include "loader.h"
#include "context.h"
#include "utils/net.h"
#include "utils/log/log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>


typedef struct dhcp_config dhcp_config;


static const char* TAG = "CONFIG LOAD";

static struct logger* loggr() {
	return context_get_logger();
}


#define CONF_NET_IP 1
#define CONF_NET_MASK 2
#define CONF_NET_GATEWAY 3

const char* NET_IP_KEY = "LEASE_NET_IP";
const char* NET_MASK_KEY = "LEASE_NET_MASK";
const char* NET_ROUTER_KEY = "NET_ROUTER_IP";


static FILE* open_dhcp_config_file() {
	const char* config_path = context_config_path();
	FILE* fp = fopen(config_path, "r");
	
	if (fp == NULL)
		return NULL;

	return fp;
}

static void ignore_file_line(FILE* cfp) {
	const int size = 256;
	char buffer[size];
	
	while (fgets(buffer, size, cfp) != NULL) {
		if (buffer[strlen(buffer) - 1] == '\n')
			break;
	}
}

static int is_whitespace_line(const char* buffer) {
	while (isspace(*buffer))
		buffer++;

	if (*buffer == 0)
		return 1;
	
	return 0;
}

static char* extract_trimmed_string(const char* buffer, const char* end) {
	while (isspace(*buffer))
		buffer++;

	while (buffer < end && isspace(*(end - 1)))
		end--;
	
	int size = end - buffer;
	char* str = malloc(size + 1);

	str[size] = '\0';
	
	return strncpy(str, buffer, size);
}

static char* extract_config_key(const char* buffer) {
	char* sep_pos = strchr(buffer, '=');
	if (sep_pos == NULL)
		return NULL;

	return extract_trimmed_string(buffer, sep_pos);
}

static char* extract_config_value(const char* buffer) {
	char* sep_pos = strchr(buffer, '=');
	if (sep_pos == NULL)
		return NULL;

	return extract_trimmed_string(sep_pos + 1, buffer + strlen(buffer));
}

static int parse_config_line(const char* buffer, dhcp_config* conf) {
	char* key = extract_config_key(buffer);
	if (key == NULL)
		return -1;

	char* value = extract_config_value(buffer);
	if (value == NULL) {
		free(key);
		return -1;
	}

	int parsed_field = 0;

	if (strcmp(key, NET_IP_KEY) == 0) {
		log_debug(loggr(), TAG, "Network IP: %s", value);

		parse_inaddr(value, &(conf->net_addr));
		parsed_field = CONF_NET_IP;
	}
	else if (strcmp(key, NET_MASK_KEY) == 0) {
		log_debug(loggr(), TAG, "Network IP mask: %s", value);

		parse_inaddr(value, &(conf->net_mask));
		parsed_field = CONF_NET_MASK;
	}
	else if (strcmp(key, NET_ROUTER_KEY) == 0) {
		log_debug(loggr(), TAG, "Network gateway ip: %s", value);
		
		parse_inaddr(value, &(conf->net_gateway));
		parsed_field = CONF_NET_GATEWAY;
	}
	else {
		log_warn(loggr(), TAG, "Unknown DHCP config key: %s", key);
	}

	free(key);
	free(value);

	return parsed_field;
}

static int parse_dhcp_config(FILE* cfp, dhcp_config* conf) {
	const int size = 1024;
	char buffer[size];
	int line = 0;

	int net_ip_init = 0;
	int net_mask_init = 0;

	while (fgets(buffer, size, cfp) != NULL) {
		line += 1;

		int n = strlen(buffer);

		if (buffer[n - 1] != '\n') {
			log_warn(loggr(), TAG, "Ignoring too long line (%d) in configuration file", line);
			ignore_file_line(cfp);

			continue;
		}

		if (is_whitespace_line(buffer)) {
			continue;
		}

		int key = parse_config_line(buffer, conf);
		if (key < 0) {
			log_warn(loggr(), TAG, "Config line (%d) invalid format", line);
			continue;
		}

		if (key == CONF_NET_IP)
			net_ip_init = 1;
		else if (key == CONF_NET_MASK)
			net_mask_init = 1;
	}

	if (net_ip_init == 0) {
		log_error(loggr(), TAG, "Network ip address not set");
		return -1;
	}

	if (net_mask_init == 0) {
		log_error(loggr(), TAG, "Network mask not set");
		return -1;
	}

	return 0;
}

dhcp_config* dhcp_config_load() {
	FILE* fp = open_dhcp_config_file();
	if (fp == NULL) {
		log_error(loggr(), TAG, "Failed to open DHCP config file: %s", strerror(errno));
		return NULL;
	}

	dhcp_config* conf = malloc(sizeof(dhcp_config));
	int err = parse_dhcp_config(fp, conf);
	
	fclose(fp);

	if (err < 0) {
		log_error(loggr(), TAG, "Failed to parse DHCP config file");
		dhcp_config_cleanup(conf);

		return NULL;
	}

	return conf;
}

void dhcp_config_cleanup(dhcp_config* conf) {
	free(conf);
}

