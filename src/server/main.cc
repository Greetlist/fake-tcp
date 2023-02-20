#include "server/server_epoll.h"
#include "gflags/gflags.h"

DEFINE_string(config_file, "server_config.yml", "core config file");

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  ftcp::ServerEpoll* server = new ftcp::ServerEpoll(FLAGS_config_file);
  server->Init();
  server->Start();
  return 0;
}
