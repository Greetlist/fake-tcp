#ifndef __IPC_UNIX_H_
#define __IPC_UNIX_H_

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <string.h>

#define CONTROLLEN CMSG_LEN(sizeof(int))

namespace ftcp {

int send_fd(int pair_fd, int target_fd);

int recv_fd(int pair_fd);

} //namespace ftcp

#endif
