#include "epoll_server/epoll_tcp_server.h"
#include "epoll_server/epoll_udp_server.h"
#include "epoll_server/global_def.h"

namespace ftcp {

class EpollServerFactory {
 public:
   EpollServerFactory() = delete;
   ~EpollServerFactory();
   static EpollServerBase* GetServer(const ServerType& server_type, const std::string& listen_addr="0.0.0.0", const int& listen_port=1000) {
     EpollServerFactory::InitServer(server_type, listen_addr, listen_port);
     return instance_;
   }

   static void InitServer(const ServerType& server_type, const std::string& listen_addr, const int& listen_port) {
     if (instance_ == nullptr) {
       std::call_once(instance_guard_, [=]() {
         if (server_type == ServerType::TCP) {
           EpollRunMode mode = EpollRunMode::UseProcess;
           int parallel = 5;
           instance_ = new EpollTCPServer(mode, parallel, listen_addr, listen_port);
         } else if (server_type == ServerType::UDP) {
           instance_ = new EpollUDPServer(listen_addr, listen_port);
         }
       });
     }
   }
 private:
  static std::once_flag instance_guard_;
  static EpollServerBase* instance_;
};

/* static */
std::once_flag EpollServerFactory::instance_guard_;
EpollServerBase* EpollServerFactory::instance_ = nullptr;

} //namespace ftcp
