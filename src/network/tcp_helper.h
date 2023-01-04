#ifndef __TCP_HELEPER_H_
#define __TCP_HELEPER_H_

#include "network/helper_base.h"

class TcpHelper : public ProtocolHelper {
 public:
  TcpHelper() = default;
  ~TcpHelper() = default;
  virtual PacketHeader GenFakeHeader(const char* payload) override;
  virtual PacketHeader ExtractRawHeader(const char* data) override;
};

#endif
