#include "debug.hpp"

void
log_info(const char* msg)
{
  printf("[    INFO] /_\\ %s\n", msg);
}

void
log_success(const char* msg)
{
  printf(std::format("[ SUCCESS] /_\\ {}\n", msg).c_str());
}

std::string
get_output_string(const char* filename, int line)
{
  return std::format("{} | {}", filename, line);
}

void
log_warning(const char* msg, const char* file, int line)
{
  printf(std::format("[ WARNING] /_\\ {} /_\\ {}\n",
                     msg,
                     get_output_string(file, line).c_str())
           .c_str());
}

void
log_error(const char* msg, const char* file, int line)
{
  printf(std::format("[  FAILED] /_\\ {} /_\\ {}\n",
                     msg,
                     get_output_string(file, line).c_str())
           .c_str());
  exit(EXIT_FAILURE);
}