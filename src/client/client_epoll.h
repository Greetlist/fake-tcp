#ifndef __CLIENT_EPOLL_H_
#define __CLIENT_EPOLL_H_

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

namespace ftcp {

class ClientEpoll {
 public:
  explicit ClientEpoll(const std::string&, const int&);
  ~ClientEpoll() {
    Stop();
  };
  ReturnCode Init();
  void Start();
  void Stop();
 private:
  ReturnCode InitListenSocket(); // listen socket recv packet from local application(client side).
  ReturnCode InitRawSendSocket(); // raw socket, send raw packet to server
  ReturnCode SendToServer();
  void StartMainEpoll();

  std::string listen_addr_;
  int listen_port_;
  int listen_fd_;
  std::string server_addr_;
  int server_port_;
  int packet_send_fd_;
  int main_ep_fd_;
  std::atomic<bool> stop_;
  static constexpr int kEventLen = 128;
  static constexpr int kListenBackLog = 128;
};

} //namespace ftcp

#endif
