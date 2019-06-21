#ifndef __H_UTILS_LOGGING_LOG
#define __H_UTILS_LOGGING_LOG


struct logger;

struct logger* initialize_log(void);
void log_cleanup(struct logger* logger);

int log_verbose(struct logger* log, const char* tag, const char* format, ...);
int log_debug(struct logger* log, const char* tag, const char* format, ...);
int log_info(struct logger* log, const char* tag, const char* format, ...);
int log_warn(struct logger* log, const char* tag, const char* format, ...);
int log_error(struct logger* logger, const char* tag, const char* format, ...);

#endif // __H_UTILS_LOGGING_LOG
