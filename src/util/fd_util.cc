#include "util/fd_util.h"

namespace ftcp {

int set_nonblock(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  int status = fcntl(fd, F_SETFL, flags);
  if (status < 0) {
    perror("File Control Error");
  }
  return status;
}

} //namespace ftcp
