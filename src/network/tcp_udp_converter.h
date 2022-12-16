#ifndef __TCP_UDP_CONVERTER_H_
#define __TCP_UDP_CONVERTER_H_

#include "converter_base.h"
#include "struct_def.h"

template <class Src, class Dst>
class TcpUdpConverter : public Converter {
private:
  TcpUdpConverter();
  ~TcpUdpConverter();
  Src ToSrc(const Dst&) override;
  Dst ToDst(const Src&) override;
};

#endif
