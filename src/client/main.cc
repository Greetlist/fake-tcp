#include "client/client_epoll.h"

int main(int argc, char** argv) {
  std::string listen_addr{argv[1]};
  int listen_port = std::stoi(std::string{argv[2]});
  std::string server_addr{argv[3]};
  int server_port = std::stoi(std::string{argv[4]});
  ftcp::ClientEpoll* client_epoll = new ftcp::ClientEpoll(listen_addr, listen_port, server_addr, server_port);
  client_epoll->Init();
  client_epoll->Start();
  return 0;
}
