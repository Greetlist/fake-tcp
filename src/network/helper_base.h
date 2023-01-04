#ifndef __CONVERTER_BASE_H_
#define __CONVERTER_BASE_H_

#include "network/struct_def.h"

class ProtocolHelper {
public:
  virtual PacketHeader GenFakeHeader(const char* payload);
  virtual PacketHeader ExtractRawHeader(const char* data);
};

#endif
