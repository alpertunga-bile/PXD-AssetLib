#include <cstdio>
#include <format>
#include <string>

void
log_info(const char* msg);

void
log_success(const char* msg);

std::string
get_output_string(const char* filename, int line);

void
log_warning(const char* msg, const char* file, int line);

void
log_error(const char* msg, const char* file, int line);

#if defined(_DEBUG)
#define LOG_INFO(msg)    log_info(msg)
#define LOG_SUCCESS(msg) log_success(msg);
#define LOG_WARNING(msg) log_warning(msg, __FILE__, __LINE__);
#define LOG_ERROR(msg)   log_error(msg, __FILE__, __LINE__);
#elif defined(_NDEBUG)
#define LOG_INFO(msg)
#define LOG_SUCCESS(msg)
#define LOG_WARNING(msg)
#define LOG_ERROR(msg)
#endif