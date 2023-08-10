#include "logger/logger.h"

namespace ftcp {

static char LOGTEXT[][16] = {"", "DEBUG", "INFO", "WARN", "ERROR", "FATAL", "SUCCESS"};
static char LOGCOLOR[][16] = {RESET, MAGENTA, BLUE, YELLOW, RED, RED, GREEN};

void gen_time_str(char* buf, size_t len) {
  memset(buf, 0, len);
  struct timeval t_val;
  gettimeofday(&t_val, NULL);

  struct tm t;
  strftime(buf, len, "%Y-%m-%d %H:%M:%S", localtime_r(&t_val.tv_sec, &t));
  sprintf(buf, "%s.%ld", buf, t_val.tv_usec);
}

void LOG(LEVEL level, const char* file, int line, const char* fmt, ...) {
  if (level < LEVEL::DEBUG) {
    return;
  }
  printf("%s", LOGCOLOR[level]);
  char time_str[128];
  gen_time_str(time_str, 128);
  printf("[%s][%s][%s:%d] ", LOGTEXT[level], time_str, file, line);
  
  va_list v_list;
  va_start(v_list, fmt);
  vfprintf(stdout, fmt, v_list);
  va_end(v_list);

  printf("%s\n", RESET);
  fflush(stdout);
}

} //namespace ftcp
