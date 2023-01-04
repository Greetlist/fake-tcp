#include "client/client_epoll.h"

namespace ftcp {

ClientEpoll::ClientEpoll(const std::string& listen_addr, const int& listen_port, const std::string& server_addr, const int& server_port) : listen_addr_(listen_addr), listen_port_(listen_port), server_addr_(server_addr), server_port_(server_port) {
  stop_ = false;
}

ReturnCode ClientEpoll::Init() {
  ReturnCode ret = ReturnCode::SUCCESS;
  main_ep_fd_ = epoll_create1(0);
  if (main_ep_fd_ < 0) {
    LOG_ERROR("Create epoll error");
    ret = ReturnCode::EP_CREATE_ERROR;
  }
  ret = InitListenSocket();
  ret = InitRawSendSocket();
  return ret;
}

void ClientEpoll::Start() {
  StartMainEpoll();
}

void ClientEpoll::Stop() {
  stop_ = true;
}

ReturnCode ClientEpoll::InitListenSocket() {
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
  setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
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

ReturnCode ClientEpoll::InitRawSendSocket() {
  packet_send_fd_ = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
  if (packet_send_fd_ < 0) {
    LOG_ERROR("Create send raw socket error.");
    return ReturnCode::CREATE_SOCK_ERROR;
  }
  bool flag = 1;
  int ss = setsockopt(packet_send_fd_, IPPROTO_IP, IP_HDRINCL, &flag, sizeof(flag));
  if (ss < 0) {
    LOG_ERROR("Setsocketopt error");
    return ReturnCode::SETSOCKOPT_ERROR;
  }
  ss = set_nonblock(packet_send_fd_);
  if (ss < 0) {
    return ReturnCode::FCNTL_ERROR;
  }
  return ReturnCode::SUCCESS;
}

ReturnCode ClientEpoll::SendToServer() {
  return ReturnCode::SUCCESS;
}

void ClientEpoll::StartMainEpoll() {
  struct epoll_event events[kEventLen];
  LOG_INFO("Start UDP Epoll Loop");
  struct sockaddr_storage peer_addr;
  socklen_t peer_addr_len = sizeof(struct sockaddr_storage);
  while (!stop_) {
    int nums = epoll_wait(main_ep_fd_, events, kEventLen, -1);
    for (int i = 0; i < nums; ++i) {
      if (events[i].data.fd == listen_fd_) {  // read data from local application, send to fake-tcp server
        int client_fd = events[i].data.fd;
        char read_buf[MAX_PAYLOAD];
        int read_bytes_num = recvfrom(
            client_fd, read_buf, MAX_PAYLOAD, 0,
            (struct sockaddr*)&peer_addr, &peer_addr_len);
        if (read_bytes_num < 0 && errno != EAGAIN) {
          LOG_ERROR("Read From Client Error");
        } else {
          char host[NI_MAXHOST], port[NI_MAXSERV];
          getnameinfo(
            (struct sockaddr*)&peer_addr, peer_addr_len,
            host, NI_MAXHOST, port, NI_MAXSERV, NI_NUMERICSERV);
          LOG_INFO("Peer addr is: %s, port is: %d", host, std::stoi(std::string{port}));
          LOG_INFO("Data: %s.", read_buf);
          //SendToSever();
        }
      }
    }
  }
}

} //namespace ftcp
