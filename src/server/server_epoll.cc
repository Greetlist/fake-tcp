#include "server/server_epoll.h"
#include "server/filter_code.h"

namespace ftcp {

ServerEpoll::ServerEpoll(const std::string& raw_addr, const int& raw_port, const std::string& real_addr, const int& real_port) : raw_addr_(raw_addr), raw_port_(raw_port), real_addr_(real_addr), real_port_(real_port) {
  stop_ = false;
}

ReturnCode ServerEpoll::Init() {
  ReturnCode ret = ReturnCode::SUCCESS;
  main_ep_fd_ = epoll_create1(0);
  if (main_ep_fd_ < 0) {
    LOG_ERROR("Create epoll error");
    ret = ReturnCode::EP_CREATE_ERROR;
  }
  ret = InitRawRecvSocket();
  ret = InitRealSendSocket();
  return ret;
}

void ServerEpoll::Start() {
  StartMainEpoll();
}

void ServerEpoll::Stop() {
  stop_ = true;
}

ReturnCode ServerEpoll::InitRawRecvSocket() {
  raw_recv_fd_ = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_IP));
  if (raw_recv_fd_ < 0) {
    LOG_ERROR("Create send raw socket error.");
    return ReturnCode::CREATE_SOCK_ERROR;
  }
  int ss = set_nonblock(raw_recv_fd_);
  if (ss < 0) {
    return ReturnCode::FCNTL_ERROR;
  }

  struct epoll_event ev;
  memset(&ev, 0, sizeof(ev));
  ev.data.fd = raw_recv_fd_;
  ev.events = EPOLLIN | EPOLLET;
  ss = epoll_ctl(main_ep_fd_, EPOLL_CTL_ADD, raw_recv_fd_, &ev);
  if (ss < 0) {
    LOG_ERROR("Epoll Add Error");
    return ReturnCode::EP_CONTROL_ERROR;
  }
  return InitSockFilter();
}

ReturnCode ServerEpoll::InitSockFilter() {
  struct sock_fprog bpf = {
    .len = sizeof(udp_filter_code) / sizeof(struct sock_filter),
    .filter = udp_filter_code,
  };
  int dummy;
  int ret = setsockopt(raw_recv_fd_, SOL_SOCKET, SO_DETACH_FILTER, &dummy, sizeof(dummy));
  ret = setsockopt(raw_recv_fd_, SOL_SOCKET, SO_ATTACH_FILTER, &bpf, sizeof(bpf));
  if (ret < 0) {
    LOG_ERROR("attach filter error");
    return ReturnCode::SETSOCKOPT_ERROR;
  }
  return ReturnCode::SUCCESS;
}

ReturnCode ServerEpoll::InitRealSendSocket() {
  return ReturnCode::SUCCESS;
}

void ServerEpoll::StartMainEpoll() {
  struct epoll_event events[kEventLen];
  LOG_INFO("Start UDP Epoll Loop");
  struct sockaddr_storage peer_addr;
  socklen_t peer_addr_len = sizeof(struct sockaddr_storage);
  while (!stop_) {
    int nums = epoll_wait(main_ep_fd_, events, kEventLen, -1);
    for (int i = 0; i < nums; ++i) {
      if (events[i].data.fd == raw_recv_fd_) { // read data from fake-tcp client, extract payload and send to real server
        char read_buf[MAX_PACKET_SIZE];
        int read_bytes_num = recvfrom(
            raw_recv_fd_, read_buf, MAX_PACKET_SIZE, 0,
            (struct sockaddr*)&peer_addr, &peer_addr_len);
        if (read_bytes_num < 0 && errno != EAGAIN) {
          LOG_ERROR("Read From Client Error");
        } else {
          LOG_INFO("Recv Raw.");
          //ExtractRawPacket();
          //SendToLocalApplication();
        }
      }
    }
  }
}

} //namespace ftcp
