#include "client/client_epoll.h"

namespace ftcp {

ClientEpoll::ClientEpoll(const std::string& config_file): config_file_(config_file) {
  stop_ = false;
}

ReturnCode ClientEpoll::Init() {
  config_ = YAML::LoadFile(config_file_);
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
  server.sin_port = htons(config_["listen_port"].as<int>());
  server.sin_addr.s_addr = inet_addr(config_["listen_addr"].as<std::string>().c_str());

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

std::unique_ptr<char> ClientEpoll::ConstructPacket(char* raw_data, int data_len) {
  struct TCPPseudoHeader tcp_p_header = ConstructTCPPseudoHeader(data_len);
  struct tcphdr tcp_header = ConstructTCPHeader();
  char total_tcp_segment[TCP_P_HEADER_LEN + TCP_HEADER_LEN + data_len];
  memmove(total_tcp_segment, &tcp_p_header, TCP_P_HEADER_LEN);
  memmove(total_tcp_segment + TCP_P_HEADER_LEN, &tcp_header, TCP_HEADER_LEN);
  memmove(total_tcp_segment + TCP_P_HEADER_LEN + TCP_HEADER_LEN, raw_data, data_len);
  tcp_header.check = CalcCheckSum(total_tcp_segment);

  struct iphdr ip_header = ConstructIPHeader();
  std::unique_ptr<char> res(new char[IP_HEADER_LEN + TCP_HEADER_LEN + data_len]);
  memmove(res.get(), &ip_header, IP_HEADER_LEN);
  memmove(res.get() + IP_HEADER_LEN, &tcp_header, TCP_HEADER_LEN);
  memmove(res.get() + IP_HEADER_LEN + TCP_HEADER_LEN, raw_data, data_len);
  return res;
}

struct iphdr ClientEpoll::ConstructIPHeader() {
  struct iphdr header;
  memset(&header, 0, IP_HEADER_LEN);
  header.version = IPVERSION;
  header.ihl = IP_HEADER_LEN / 4;
  header.tos = 0;
  header.tot_len = htons(IP_HEADER_LEN + TCP_HEADER_LEN);
  header.id = 0;
  header.frag_off = 0;
  header.ttl = MAXTTL;
  header.protocol = IPPROTO_TCP;
  header.check = 0;
  inet_pton(AF_INET, config_["send_addr"].as<std::string>().c_str(), &header.saddr);
  inet_pton(AF_INET, config_["server_addr"].as<std::string>().c_str(), &header.daddr);
  return header;
}

struct tcphdr ClientEpoll::ConstructTCPHeader() {
  struct tcphdr header;
  memset(&header, 0, TCP_HEADER_LEN);
  header.source = htons(config_["send_port"].as<int>());
  header.dest = htons(config_["server_port"].as<int>());
  header.doff = TCP_HEADER_LEN / 4;
  header.syn = 0;
  header.ack = 1;
  header.urg = 1;
  header.window = htons(4096);
  header.check = 0;
  header.seq = htons(seq_num_);
  header.ack_seq = 0;
  return header;
}

struct TCPPseudoHeader ClientEpoll::ConstructTCPPseudoHeader(int data_len) {
  struct TCPPseudoHeader header;
  memset(&header, 0, TCP_P_HEADER_LEN);
  inet_pton(AF_INET, config_["send_addr"].as<std::string>().c_str(), &header.src);
  inet_pton(AF_INET, config_["server_addr"].as<std::string>().c_str(), &header.dst);
  header.zero = 0;
  header.protocol = IPPROTO_TCP;
  header.total_len = htons(TCP_HEADER_LEN + data_len);
  return header;
}

unsigned short ClientEpoll::CalcCheckSum(const char* buf) {
  size_t size = TCP_HEADER_LEN + TCP_P_HEADER_LEN;
  unsigned int checkSum = 0;
  for (int i = 0; i < size; i += 2) {
    unsigned short first = (unsigned short)buf[i] << 8;
    unsigned short second = (unsigned short)buf[i+1] & 0x00ff;
    checkSum += first + second;
  }
  while (1) {
    unsigned short c = (checkSum >> 16);
    if (c > 0) {
      checkSum = (checkSum << 16) >> 16;
      checkSum += c;
    } else {
      break;
    }
  }
  return ~checkSum;
}

ReturnCode ClientEpoll::SendToServer(std::unique_ptr<char>& packet) {
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  inet_pton(AF_INET, config_["send_addr"].as<std::string>().c_str(), &addr.sin_addr.s_addr);
  addr.sin_port = htons(config_["send_port"].as<int>());
  socklen_t len = sizeof(struct sockaddr_in);
  int n = sendto(packet_send_fd_, packet.get(), strlen(packet.get()), 0, (struct sockaddr*)&addr, len);
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
          std::unique_ptr<char> packet = ConstructPacket(read_buf, read_bytes_num);
          SendToServer(packet);
        }
      }
    }
  }
}

} //namespace ftcp
