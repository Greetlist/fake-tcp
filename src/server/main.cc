#include "server/server_epoll.h"

int main(int argc, char** argv) {
  std::string raw_addr{argv[1]};
  int raw_port = std::stoi(std::string{argv[2]});
  std::string real_addr{argv[3]};
  int real_port = std::stoi(std::string{argv[4]});
  ftcp::ServerEpoll* server = new ftcp::ServerEpoll(raw_addr, raw_port, real_addr, real_port);
  server->Init();
  server->Start();
  return 0;
}
