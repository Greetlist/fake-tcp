#include "client/client_epoll.h"
#include "yaml-cpp/yaml.h"
#include "gflags/gflags.h"

DEFINE_string(config_file, "config.yml", "core config file");

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  ftcp::ClientEpoll* client_epoll = new ftcp::ClientEpoll(FLAGS_config_file);
  client_epoll->Init();
  client_epoll->Start();
  return 0;
}
