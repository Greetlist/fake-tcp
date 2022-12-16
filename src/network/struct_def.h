#ifndef __PACKET_INFO_H_
#define __PACKET_INFO_H_

#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

struct pseudo_tcp_header {
  unsigned int src_ip;
  unsigned int dst_ip;
  unsigned short zero:8;
  unsigned short protocol:8;
  unsigned short total_len;
};
struct packet_info {
  struct iphdr ip_header;
  struct pseudo_tcp_header p_header;
  struct tcphdr tcp_header;
  char src_ip[16];
  char dst_ip[16];
};

using PseudoTcpHeader = struct pseudo_tcp_header;
using IPHeader = struct iphdr;
using TCPHeader = struct tcphdr;
using UDPHeader = struct udphdr;

#endif
