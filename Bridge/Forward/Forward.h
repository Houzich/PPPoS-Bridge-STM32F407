#ifndef __FORWARD_H
#define __FORWARD_H
#include "ppp.h"










err_t Forward_TCPIP_From_ETH_Packet(struct pbuf *p, struct netif *inp);
err_t Forward_ETH_From_ETH_Packet(struct pbuf *p, struct netif *inp);
err_t PPPoS_Forward_ETH_From_TCPIP(ppp_pcb *ppp, struct pbuf *p);
int Check_ETH_Packet_For_Forward(struct pbuf *p, struct netif *inp);
int Check_TCPIP_Packet_For_Forward(struct pbuf *p, struct netif *inp);
#endif /* __FORWARD_H */
