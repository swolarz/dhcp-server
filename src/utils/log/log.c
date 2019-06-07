#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>


#define LEVEL_DEBUG 0
#define LEVEL_INFO 1
#define LEVEL_WARN 2
#define LEVEL_ERROR 3


typedef struct log_handle {
} log_handle;


log_handle* log_get_handle() {
	log_handle* logger = malloc(sizeof(log_handle));
	return logger;
}

void log_cleanup(log_handle* logger) {
	if (logger != NULL)
		free(logger);
}

int format_datetime(char* buffer) {
	struct timeval tv;
	gettimeofday(&tv, NULL);

	int millis = tv.tv_usec / 1000;
	struct tm* tm_info = localtime(&tv.tv_sec);

	char tm_buff[24];
	strftime(tm_buff, 24, "%Y-%m-%d %H:%M:%S", tm_info);
	
	return sprintf(buffer, "%s.%03d", tm_buff, millis);
}

const char* format_level(int level) {
	switch (level) {
		case LEVEL_DEBUG:
			return "DEBUG";
		case LEVEL_INFO:
			return "INFO";
		case LEVEL_WARN:
			return "WARN";
		case LEVEL_ERROR:
			return "ERROR";
		default:
			return "(none)";
	}
}

int log_message(log_handle* logger, int level, const char* tag, const char* msg) {
	if (logger == NULL)
		return -1;

	char time_buff[24];
	format_datetime(time_buff);

	const char* level_buff = format_level(level);

	fprintf(stdout, "[%s] %-8s | %-16s: %s\n", time_buff, level_buff, tag, msg);
}

int log_debug(log_handle* logger, const char* tag, const char* msg) {
	return log_message(logger, LEVEL_DEBUG, tag, msg);
}

int log_info(log_handle* logger, const char* tag, const char* msg) {
	return log_message(logger, LEVEL_INFO, tag, msg);
}

int log_warn(log_handle* logger, const char* tag, const char* msg) {
	return log_message(logger, LEVEL_WARN, tag, msg);
}

int log_error(struct log_handle* logger, const char* tag, const char* msg) {
	return log_message(logger, LEVEL_ERROR, tag, msg);
}


