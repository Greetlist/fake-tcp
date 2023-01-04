#ifndef __SERVER_EPOLL_H_
#define __SERVER_EPOLL_H_

#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <linux/if_ether.h>

#include <atomic>
#include <mutex>

#include "common/return_code.h"
#include "common/common_def.h"
#include "network/helper_base.h"
#include "logger/logger.h"
#include "epoll_server/global_def.h"
#include "util/fd_util.h"

namespace ftcp {

class ServerEpoll {
 public:
  explicit ServerEpoll(const std::string&, const int&, const std::string&, const int&);
  ~ServerEpoll() {
    Stop();
  };
  ReturnCode Init();
  void Start();
  void Stop();
 private:
  ReturnCode InitRawRecvSocket(); // raw socket, recv raw packet from client
  ReturnCode InitSockFilter(); // Init filter for raw socket
  ReturnCode InitRealSendSocket(); // send to real server
  void StartMainEpoll();

  std::string raw_addr_;
  int raw_port_;
  int raw_recv_fd_;
  std::string real_addr_;
  int real_port_;
  int real_fd_;
  int main_ep_fd_;
  std::atomic<bool> stop_;
  static constexpr int kEventLen = 128;
  static constexpr int kListenBackLog = 128;
};

} //namespace ftcp

#endif
