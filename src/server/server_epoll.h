#ifndef __SERVER_EPOLL_H_
#define __SERVER_EPOLL_H_

#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <linux/if_ether.h>
#include <net/if.h>

#include <atomic>
#include <mutex>

#include "common/return_code.h"
#include "common/common_def.h"
#include "network/struct_def.h"
#include "logger/logger.h"
#include "epoll_server/global_def.h"
#include "yaml-cpp/yaml.h"
#include "util/fd_util.h"

namespace ftcp {

class ServerEpoll {
 public:
  explicit ServerEpoll(const std::string&);
  ~ServerEpoll() {
    Stop();
  };
  ReturnCode Init();
  void Start();
  void Stop();
 private:
  ReturnCode InitRawRecvSocket(); // raw socket, recv raw packet from client
  ReturnCode BindDevice();
  ReturnCode InitSockFilter(); // Init filter for raw socket
  ReturnCode InitRealSendSocket(); // send to real server
  void StartMainEpoll();

  void MainProcess(char*, int);
  std::unique_ptr<char> ExtractData(char*, int);
  ReturnCode SendToLocalApplication(std::unique_ptr<char>&&);

  std::string TransportProtocol(unsigned char);
 private:
  std::string config_file_;
  YAML::Node config_;
  int raw_recv_fd_;
  int send_fd_;
  int main_ep_fd_;
  std::atomic<bool> stop_;
  static constexpr int kEventLen = 128;
  static constexpr int kListenBackLog = 128;
};

} //namespace ftcp

#endif
