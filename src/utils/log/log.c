#include "log.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>


#define LOG_BUFFER_SIZE 512 

#define LEVEL_DEBUG 0
#define LEVEL_INFO 1
#define LEVEL_WARN 2
#define LEVEL_ERROR 3


typedef struct logger {
} logger;


logger* initialize_log() {
	logger* log = malloc(sizeof(logger));
	return log;
}

void log_cleanup(logger* log) {
	if (log != NULL)
		free(log);
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

int log_message(logger* log, int level, const char* tag, const char* format, va_list va) {
	if (log == NULL)
		return -1;

	char time_buff[24];
	format_datetime(time_buff);

	const char* level_buff = format_level(level);

	char msg_buff[LOG_BUFFER_SIZE];
	vsnprintf(msg_buff, LOG_BUFFER_SIZE, format, va);
	va_end(va);

	int bytes = fprintf(stdout, "[%s] %-8s | %-16s: %s\n", time_buff, level_buff, tag, msg_buff);
	if (bytes < 0)
		return -1;

	return 0;
}

#define INIT_VA(fmt) va_list va; va_start(va, fmt)

int log_debug(logger* log, const char* tag, const char* format, ...) {
	INIT_VA(format);
	return log_message(log, LEVEL_DEBUG, tag, format, va);
}

int log_info(logger* log, const char* tag, const char* format, ...) {
	INIT_VA(format);
	return log_message(log, LEVEL_INFO, tag, format, va);
}

int log_warn(logger* log, const char* tag, const char* format, ...) {
	INIT_VA(format);
	return log_message(log, LEVEL_WARN, tag, format, va);
}

int log_error(struct logger* log, const char* tag, const char* format, ...) {
	INIT_VA(format);
	return log_message(log, LEVEL_ERROR, tag, format, va);
}


