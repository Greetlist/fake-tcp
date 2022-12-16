#include "util/ipc_unix.h"

namespace ftcp {

int send_fd(int pair_fd, int target_fd) {
  struct iovec iov[1];
  struct msghdr msg;
  memset(&msg, 0, sizeof(msg));
  char buf[128];

  iov[0].iov_base = buf;
  iov[0].iov_len = 128;

  msg.msg_iov = iov;
  msg.msg_iovlen = 1;
  msg.msg_name = NULL;
  msg.msg_namelen = 0;

  struct cmsghdr* cmsg = static_cast<struct cmsghdr*>(malloc(CONTROLLEN));
  if (target_fd < 0) {
    return -1;
  } else {
    memset(cmsg, 0, sizeof(cmsg));
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CONTROLLEN;

    msg.msg_control = cmsg;
    msg.msg_controllen = CONTROLLEN;
    *(int*)CMSG_DATA(cmsg) = target_fd;
  }
  int numSend;
  if ((numSend = sendmsg(pair_fd, &msg, 0)) < 0) {
    perror("Send Msg Error");
    return -1;
  }
  free(cmsg);
  return 0;
}

int recv_fd(int pair_fd) {
  int target_fd, nr;
  char buf[128];
  struct iovec iov[1];
  struct msghdr msg;
  struct cmsghdr* cmsg = static_cast<struct cmsghdr*>(malloc(sizeof(struct cmsghdr)));
  memset(cmsg, 0, sizeof(struct cmsghdr));

  memset(buf, 0, 128);
  while (1) {
    iov[0].iov_base = buf;
    iov[0].iov_len = 128;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_control = cmsg;
    msg.msg_controllen = CONTROLLEN;

    if ((nr = recvmsg(pair_fd, &msg, 0)) < 0) {
      perror("Recv Msg Error");
      return -1;
    } else if (nr == 0) {
      printf("Connection closed by peer.\n");
      return -1;
    } else {
      target_fd = *(int*)CMSG_DATA(cmsg);
      break;
    }
  }
  return target_fd;
}

} //namespace ftcp
