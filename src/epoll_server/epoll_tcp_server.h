#ifndef __EPOLL_TCP_SERVER_H_
#define __EPOLL_TCP_SERVER_H_

#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#include <thread>
#include <vector>
#include <atomic>
#include <mutex>

#include "util/fd_util.h"
#include "util/ipc_unix.h"
#include "common/return_code.h"
#include "logger/logger.h"
#include "epoll_server/global_def.h"
#include "epoll_server/epoll_server_base.h"

namespace ftcp {

class EpollTCPServer : public EpollServerBase {
 public:
  explicit EpollTCPServer(const EpollRunMode&, const int&, const std::string&, const int&);
  ~EpollTCPServer() {
    Stop();
  };
  virtual ReturnCode Init();
  virtual void Start();
  virtual void Stop();
 private:
  ReturnCode InitListenSocket();
  void CreateThreads();
  void CreateProcesses();
  void MainWorker(int);
  void StartMainEpoll();
  int GetNextWorkerIndex();
  void DealClientConnection(int);
  void SendToChildProcess(int);
  void SendToChildThread(int);
  void AddToMainEpoll(int);

  std::vector<ThreadInfo> epoll_thread_info_vec_;
  std::vector<ProcessInfo> epoll_process_info_vec_;

  std::string listen_addr_;
  int listen_port_;
  EpollRunMode mode_;
  int listen_fd_;
  int main_ep_fd_;
  int parallel_num_;
  int current_worker_idx_ = 0;
  std::atomic<bool> stop_;
  static constexpr int kEventLen = 128;
  static constexpr int kListenBackLog = 128;
};

} //namespace ftcp

#endif
