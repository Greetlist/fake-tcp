#ifndef __PACKET_INFO_H_
#define __PACKET_INFO_H_

#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

struct TCPPseudoHeader {
  unsigned int src;
  unsigned int dst;
  unsigned short zero:8;
  unsigned short protocol:8;
  unsigned short total_len;
};

constexpr unsigned int ETH_HEADER_LEN = sizeof(struct ethhdr);
constexpr unsigned int IP_HEADER_LEN = sizeof(struct iphdr);
constexpr unsigned int UDP_HEADER_LEN = sizeof(struct udphdr);
constexpr unsigned int TCP_HEADER_LEN = sizeof(struct tcphdr);

#endif
