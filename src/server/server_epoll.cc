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
  raw_recv_fd_ = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
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
  ReturnCode ret = BindDevice();
  if (ret != ReturnCode::SUCCESS) {
    return ret;
  }
  return InitSockFilter();
}

ReturnCode ServerEpoll::BindDevice() {
  const char *opt = "enp88s0";
  if (setsockopt(raw_recv_fd_, SOL_SOCKET, SO_BINDTODEVICE, opt, strlen(opt) + 1) < 0) {
    LOG_ERROR("bind device error");
    return ReturnCode::BIND_DEVICE_ERROR;
  }
  /* set the network card in promiscuos mode*/
  // An ioctl() request has encoded in it whether the argument is an in parameter or out parameter
  // SIOCGIFFLAGS 0x8913      /* get flags            */
  // SIOCSIFFLAGS 0x8914      /* set flags            */
  struct ifreq ethreq;
  strncpy(ethreq.ifr_name, "enp88s0", IF_NAMESIZE);
  if (ioctl(raw_recv_fd_, SIOCGIFFLAGS, &ethreq) == -1) {
    LOG_ERROR("ioctl error");
    return ReturnCode::IOCTL_ERROR;
  }
  ethreq.ifr_flags |= IFF_PROMISC;
  if (ioctl(raw_recv_fd_, SIOCSIFFLAGS, &ethreq) == -1) {
    LOG_ERROR("ioctl error");
    return ReturnCode::IOCTL_ERROR;
  }
  return ReturnCode::SUCCESS;
}

ReturnCode ServerEpoll::InitSockFilter() {
  struct sock_fprog bpf = {
    .len = sizeof(tcp_filter_code) / sizeof(struct sock_filter),
    .filter = tcp_filter_code,
  };
  int ret = setsockopt(raw_recv_fd_, SOL_SOCKET, SO_ATTACH_FILTER, &bpf, sizeof(bpf));
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
      if (events[i].data.fd == raw_recv_fd_) { // filter data from fake-tcp client, extract payload and send to real server
        char read_buf[MAX_PACKET_SIZE];
        memset(read_buf, 0, MAX_PACKET_SIZE);
        int read_bytes_num = recvfrom(
            raw_recv_fd_, read_buf, MAX_PACKET_SIZE, 0,
            (struct sockaddr*)&peer_addr, &peer_addr_len);
        if (read_bytes_num < 0 && errno != EAGAIN) {
          LOG_ERROR("Read From Client Error");
        } else {
          struct iphdr* ip_header = (struct iphdr*)(read_buf + 14);
          LOG_INFO("Recv len: %d, content: [%s]", read_bytes_num, read_buf);
          LOG_INFO("src addr is : %s", inet_ntoa(((struct sockaddr_in*)&peer_addr)->sin_addr));
          LOG_INFO("Layer-4 protocol %s\n", TransportProtocol(ip_header->protocol).c_str());
          //ExtractRawPacket();
          //SendToLocalApplication();
        }
      }
    }
  }
}

std::string ServerEpoll::TransportProtocol(unsigned char code) {
  switch(code) {
      case 1: return "icmp";
      case 2: return "igmp";
      case 6: return "tcp";
      case 17: return "udp";
      default: return "unknown";
  }
}

} //namespace ftcp