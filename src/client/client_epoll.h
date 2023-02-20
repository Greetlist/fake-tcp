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
#include "network/struct_def.h"
#include "logger/logger.h"
#include "epoll_server/global_def.h"
#include "yaml-cpp/yaml.h"
#include "util/fd_util.h"

namespace ftcp {

class ClientEpoll {
 public:
  explicit ClientEpoll(const std::string& config_file);
  ~ClientEpoll() {
    Stop();
  };
  ReturnCode Init();
  void Start();
  void Stop();
 private:
  ReturnCode InitListenSocket(); // listen socket recv packet from local application(client side).
  ReturnCode InitRawSendSocket(); // raw socket, send raw packet to server
  std::unique_ptr<char> ConstructPacket(char* data, int data_len);
  struct iphdr ConstructIPHeader();
  struct tcphdr ConstructTCPHeader();
  struct TCPPseudoHeader ConstructTCPPseudoHeader(int data_len);
  unsigned short CalcCheckSum(const char* buf);
  ReturnCode SendToServer(std::unique_ptr<char>&& packet, int data_len);
  void StartMainEpoll();

  std::string config_file_;
  YAML::Node config_;
  uint64_t seq_num_ = 10;
  int listen_fd_;
  int packet_send_fd_;
  int main_ep_fd_;
  std::atomic<bool> stop_;
  static constexpr int kEventLen = 128;
  static constexpr int kListenBackLog = 128;
};

} //namespace ftcp

#endif
