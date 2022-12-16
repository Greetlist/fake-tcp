#ifndef __UDP_TCP_FUNC_H_
#define __UDP_TCP_FUNC_H_

#include <netinet/tcp.h>

extern unsigned short calc_tcp_checksum(const char* data);

extern void init_tcp_pseudo_header(struct pseudo_tcp_header* header);

extern void init_tcp_header(struct tcphdr* tcp_header);

extern void construct_packet_info(struct packet_info* info);

#endif
