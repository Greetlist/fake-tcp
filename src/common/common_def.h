#ifndef __COMMON_DEF_H_
#define __COMMON_DEF_H_

/*
 * According to normal net-work device, MTU is 1500 bytes.
 * so [MAX payload bytes number which will not be fragmented by link layer] is:
 * 1472 = 1500 - 20(IP Header) - 8(UDP Header)
 * If IP packet is fragemented. It will increase the rate of packege loss.
*/
constexpr int MAX_PAYLOAD = 1472;
constexpr int MAX_PACKET_SIZE = 1500;

#endif
