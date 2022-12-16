#ifndef __EPOLL_UDP_SERVER_H_
#define __EPOLL_UDP_SERVER_H_

#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#include <atomic>
#include <mutex>

#include "common/return_code.h"
#include "logger/logger.h"
#include "epoll_server/global_def.h"
#include "epoll_server/epoll_server_base.h"

namespace ftcp {

class EpollUDPServer : public EpollServerBase {
 public:
  explicit EpollUDPServer(const std::string&, const int&);
  ~EpollUDPServer() {
    Stop();
  };
  virtual ReturnCode Init();
  virtual void Start();
  virtual void Stop();
 private:
  ReturnCode InitListenSocket();
  void StartMainEpoll();

  std::string listen_addr_;
  int listen_port_;
  int listen_fd_;
  int main_ep_fd_;
  std::atomic<bool> stop_;
  static constexpr int kEventLen = 128;
  static constexpr int kListenBackLog = 128;
};

} //namespace ftcp

#endif
