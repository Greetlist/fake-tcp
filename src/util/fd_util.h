#ifndef __FD_UTIL_H_
#define __FD_UTIL_H_

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

namespace ftcp {

int set_nonblock(int fd);

} //namespace ftcp

#endif
