#ifndef __PACKET_INFO_H_
#define __PACKET_INFO_H_

#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

struct PseudoHeader {
  unsigned int src_ip;
  unsigned int dst_ip;
  unsigned short zero:8;
  unsigned short protocol:8;
  unsigned short total_len;
};

struct PacketHeader {
  struct iphdr ip_header;
  union protocol_header {
    struct tcphdr;
    struct udphdr;
  };
};

#endif
