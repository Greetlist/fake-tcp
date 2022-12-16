#include "epoll_server/server_factory.h"
#include "epoll_server/epoll_server_base.h"

int main(int argc, char** argv) {
  ftcp::EpollServerBase* server = ftcp::EpollServerFactory::GetServer(ftcp::ServerType::UDP);
  server->Init();
  server->Start();
  return 0;
}
