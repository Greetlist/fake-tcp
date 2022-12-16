#include "epoll_server/epoll_udp_server.h"

namespace ftcp {

EpollUDPServer::EpollUDPServer(const std::string& listen_addr, const int& listen_port) : listen_addr_(listen_addr), listen_port_(listen_port) {
  stop_ = false;
}

ReturnCode EpollUDPServer::Init() {
  ReturnCode ret = ReturnCode::SUCCESS;
  main_ep_fd_ = epoll_create1(0);
  ret = InitListenSocket();
  return ret;
}

void EpollUDPServer::Start() {
  StartMainEpoll();
}

void EpollUDPServer::Stop() {
  stop_ = true;
}

ReturnCode EpollUDPServer::InitListenSocket() {
  listen_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
  if (listen_fd_ < 0) {
    LOG_ERROR("Socket Error");
    return ReturnCode::CREATE_SOCK_ERROR;
  }

  struct sockaddr_in server;
  memset(&server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(listen_port_);
  server.sin_addr.s_addr = inet_addr(listen_addr_.c_str());

  bool flag = true;
  int l = sizeof(flag);
  setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &flag, l);
  int ss = bind(listen_fd_, (struct sockaddr*)&server, sizeof(server));
  if (ss < 0) {
    LOG_ERROR("Bind Error");
    return ReturnCode::BIND_ERROR;
  }
  ss = set_nonblock(listen_fd_);
  if (ss < 0) {
    return ReturnCode::FCNTL_ERROR;
  }

  struct epoll_event ev;
  memset(&ev, 0, sizeof(ev));
  ev.data.fd = listen_fd_;
  ev.events = EPOLLIN | EPOLLET;
  ss = epoll_ctl(main_ep_fd_, EPOLL_CTL_ADD, listen_fd_, &ev);
  if (ss < 0) {
    LOG_ERROR("Epoll Add Error");
    return ReturnCode::EP_CONTROL_ERROR;
  }
  LOG_SUCCESS("Create listen socket success");
  return ReturnCode::SUCCESS;
}

void EpollUDPServer::StartMainEpoll() {
  struct epoll_event events[kEventLen];
  LOG_INFO("Start UDP Epoll Loop");
  while (!stop_) {
    int nums = epoll_wait(main_ep_fd_, events, kEventLen, -1);
    for (int i = 0; i < nums; ++i) {
      if (events[i].data.fd == listen_fd_) {
        // read data from client_fd;
        int buf_size = 1024;
        int client_fd = events[i].data.fd;
        struct iovec read_vec[4];
        char buf_vec[4][buf_size];
        for (int i = 0; i < 4; ++i) {
          memset(buf_vec[i], 0, buf_size);
          read_vec[i].iov_base = buf_vec[i];
          read_vec[i].iov_len = buf_size;
        }
        int n_read = readv(client_fd, read_vec, 4);
        if (n_read < 0 && errno != EAGAIN) {
          LOG_ERROR("Read From Client Error");
        } else {
          int idx = n_read / buf_size + 1;
          for (int i = 0; i < idx; ++i) {
            LOG_INFO("Index: %d, Data: %s.", i, read_vec[i].iov_base);
          }
        }
      }
    }
  }
}

} //namespace ftcp
