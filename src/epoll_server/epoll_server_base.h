#ifndef __EPOLL_SERVER_BASE_H_
#define __EPOLL_SERVER_BASE_H_

#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#include <vector>
#include <atomic>
#include <mutex>
#include <functional>

#include "util/fd_util.h"
#include "util/ipc_unix.h"
#include "common/return_code.h"
#include "logger/logger.h"

namespace ftcp {

class EpollServerBase {
public:
  EpollServerBase() {};
  virtual ~EpollServerBase() {};
  virtual ReturnCode Init() {return ReturnCode::SUCCESS;}
  virtual void Start() {}
  virtual void Stop() {}
  virtual void SetDataCallback(std::function<void(int)> f) {
    callback_func_ = f;
  }
private:
  std::function<void(int)> callback_func_;
};

} //namespace ftcp

#endif
