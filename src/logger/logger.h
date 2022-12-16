#ifndef __LOGGER_H_
#define __LOGGER_H_

#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>

namespace ftcp {

enum LEVEL {
  NONE = 0,
  DEBUG,
  INFO,
  WARN,
  ERROR,
  FATAL,
  SUCCESS,
  NUM
};

#define RED "\x1B[31m"
#define GREEN "\x1B[32m"
#define YELLOW "\x1B[33m"
#define BLUE "\x1B[34m"
#define MAGENTA "\x1B[35m"
#define CYAN "\x1B[36m"
#define WHITE "\x1B[37m"
#define RESET "\x1B[0m"

static char LOGTEXT[][16] = {"", "DEBUG", "INFO", "WARN", "ERROR", "FATAL", "SUCCESS"};
static char LOGCOLOR[][16] = {RESET, MAGENTA, BLUE, YELLOW, RED, RED, GREEN};

void gen_time_str(char* buf);

void LOG(LEVEL level, const char* file, int line, const char* fmt, ...);

#define LOG_DEBUG(fmt, ...) LOG(DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) LOG(INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) LOG(WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) LOG(ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_FATAL(fmt, ...) LOG(FATAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_SUCCESS(fmt, ...) LOG(SUCCESS, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

} //namespace ftcp

#endif
