#ifndef __GLOBAL_DEF_H_
#define __GLOBAL_DEF_H_

#include <thread>

namespace ftcp {

enum class EpollRunMode {
  UseProcess = 0,
  UseThread,
  UseSingleThread,
  UnknownRunMode,
};

enum class ServerType {
  TCP = 0,
  UDP,
  ICMP,
};

struct ProcessInfo {
  pid_t process_id;
  int pair_fd;
};

struct ThreadInfo {
  std::thread thread_instance;
  int pair_fd;
};

} //namespace ftcp
#endif
