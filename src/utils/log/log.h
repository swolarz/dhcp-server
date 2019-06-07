#ifndef __H_UTILS_LOGGING_LOG
#define __H_UTILS_LOGGING_LOG


struct log_handle;

struct log_handle* log_get_handle(void);
void log_cleanup(struct log_handle* logger);


int log_debug(struct log_handle* logger, const char* tag, const char* message);

int log_info(struct log_handle* logger, const char* tag, const char* msg);

int log_warn(struct log_handle* logger, const char* tag, const char* msg);

int log_error(struct log_handle* logger, const char* tag, const char* msg);

#endif // __H_UTILS_LOGGING_LOG
