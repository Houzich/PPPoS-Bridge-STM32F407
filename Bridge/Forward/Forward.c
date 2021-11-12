
#include "lwip/opt.h"
#include "lwip/ip.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/ip4_frag.h"
#include "lwip/inet_chksum.h"
#include "lwip/netif.h"
#include "lwip/icmp.h"
#include "lwip/igmp.h"
#include "lwip/raw.h"
#include "lwip/udp.h"
#include "lwip/priv/tcp_priv.h"
#include "lwip/autoip.h"
#include "lwip/stats.h"
#include "lwip/prot/dhcp.h"

#include "lwip/priv/tcpip_priv.h"
#include "lwip/sys.h"
#include "lwip/memp.h"
#include "lwip/init.h"
#include "lwip/pbuf.h"
#include "lwip/etharp.h"


#include "netif/ethernet.h"

#include "netif/ppp/ppp_impl.h"
#include "netif/ppp/pppos.h"
#include "netif/ppp/vj.h"
#include "netif/ppp/fsm.h"
#include "netif/ppp/lcp.h"
#include "netif/ppp/magic.h"

#include <string.h>

#include "Bridge_Data.h"

static const u16_t fcstab[256] = {
  0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
  0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
  0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
  0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
  0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
  0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
  0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
  0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
  0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
  0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
  0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
  0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
  0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
  0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
  0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
  0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
  0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
  0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
  0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
  0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
  0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
  0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
  0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
  0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
  0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
  0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
  0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
  0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
  0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
  0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
  0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
  0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};
#define PPP_FCS(fcs, c) (((fcs) >> 8) ^ fcstab[((fcs) ^ (c)) & 0xff])

extern struct netif gnetif;

static err_t Change_IP_Rebuild_Packet(struct pbuf *p, ip_addr_t *ip, uint16_t src_dest);
static void PPPoS_Input_For_Forward(ppp_pcb *ppp, u8_t *s, int l);
static void PPP_Input_For_Forward(ppp_pcb *pcb, struct pbuf *pb);
/**
 * Forwards an IP packet. It finds an appropriate route for the
 * packet, decrements the TTL value of the packet, adjusts the
 * checksum and outputs the packet on the appropriate interface.
 *
 * @param p the packet to forward (p->payload points to IP header)
 * @param iphdr the IP header of the input packet
 * @param inp the netif on which this packet was received
 */
int Check_ETH_Packet_For_Forward(struct pbuf *p, struct netif *inp)
{
	struct eth_hdr* ethhdr;	
	struct ip_hdr *iphdr;
  ip_addr_t current_src; //Source IP address of current_header  
  ip_addr_t current_dest;	//Destination IP address of current_header
	u16_t type;
	
  ethhdr = (struct eth_hdr *)p->payload; /* points to packet payload, which starts with an Ethernet header */
	type = ethhdr->type;
	//ETHERNET Header_____________________________________________________________________________________________________
	//ETHERNET Header_____________________________________________________________________________________________________
	//ETHERNET Header_____________________________________________________________________________________________________
	  if (p->len <= SIZEOF_ETH_HDR) {
    /* a packet with only an ethernet header (or less) is not valid for us */
    return 0;
  }
	
	if(type!=PP_HTONS(ETHTYPE_IP)) //если не IP пакет
	{
		return 0;
	}
	//IPv4 Header_____________________________________________________________________________________________________
	//IPv4 Header_____________________________________________________________________________________________________
	//IPv4 Header_____________________________________________________________________________________________________
	//ip4_debug_print(p);	
	iphdr = (struct ip_hdr *)((uint8_t *)p->payload + SIZEOF_ETH_HDR);/* points to packet payload, which starts with an IP header */
  if (IPH_V(iphdr) != 4) { //Не IP пакет
    BRIDGE_DEBUG_FORWARD(("MY! IP packet dropped due to bad version number\r\n"));
    return 0;
  }
	
	if(IPH_TTL(iphdr)<=1)return 0; //мало ли приняли на последнем исдыхании пакет
	
	  /* copy IP addresses to aligned ip_addr_t */
  ip_addr_copy_from_ip4(current_dest, iphdr->dest);
  ip_addr_copy_from_ip4(current_src, iphdr->src);
	
    /* non-broadcast packet? */
    if (!ip4_addr_isbroadcast(&current_dest, inp)) {
      /* try to forward IP packet on (other) interfaces */
			if(current_dest.addr != inp->ip_addr.addr)
			{
				u32_t addr = lwip_htonl(ip4_addr_get_u32(&current_dest));

				if (p->flags & PBUF_FLAG_LLBCAST) {
					/* don't route link-layer broadcasts */
					return 0;
				}
				if ((p->flags & PBUF_FLAG_LLMCAST) && !IP_MULTICAST(addr)) {
					/* don't route link-layer multicasts unless the destination address is an IP
						 multicast address */
					return 0;
				}
				if (IP_BADCLASS(addr)) {
					return 0;
				}
				if (IP_CLASSA(addr)) {
					u32_t net = addr & IP_CLASSA_NET;
					if ((net == 0) || (net == ((u32_t)IP_LOOPBACKNET << IP_CLASSA_NSHIFT))) {
						/* don't route loopback packets */
						return 0;
					}
				}
				return 1; //прошли все проверки, ок, можно пересылать
			}
    } 	
return 0;	
}
/**
 * Forwards an IP packet. It finds an appropriate route for the
 * packet, decrements the TTL value of the packet, adjusts the
 * checksum and outputs the packet on the appropriate interface.
 *
 * @param p the packet to forward (p->payload points to IP header)
 * @param iphdr the IP header of the input packet
 * @param inp the netif on which this packet was received
 */
int Check_TCPIP_Packet_For_Forward(struct pbuf *p, struct netif *inp)
{
	struct ip_hdr *iphdr;
  ip_addr_t current_src; //Source IP address of current_header  
  ip_addr_t current_dest;	//Destination IP address of current_header

	iphdr = (struct ip_hdr *)p->payload;/* points to packet payload, which starts with an IP header */
  if (IPH_V(iphdr) != 4) { //Не IP пакет
    BRIDGE_DEBUG_FORWARD(("FORWARD: IP packet dropped due to bad version number\r\n"));
    return 0;
  }
	
	if(IPH_TTL(iphdr)<=1)return 0; //мало ли приняли на последнем исдыхании пакет
	
	  /* copy IP addresses to aligned ip_addr_t */
  ip_addr_copy_from_ip4(current_dest, iphdr->dest);
  ip_addr_copy_from_ip4(current_src, iphdr->src);
	
    /* non-broadcast packet? */
    if (!ip4_addr_isbroadcast(&current_dest, inp)) {
      /* try to forward IP packet on (other) interfaces */
			if(current_dest.addr != inp->ip_addr.addr)
			{
				u32_t addr = lwip_htonl(ip4_addr_get_u32(&current_dest));

				if (p->flags & PBUF_FLAG_LLBCAST) {
					/* don't route link-layer broadcasts */
					return 0;
				}
				if ((p->flags & PBUF_FLAG_LLMCAST) && !IP_MULTICAST(addr)) {
					/* don't route link-layer multicasts unless the destination address is an IP
						 multicast address */
					return 0;
				}
				if (IP_BADCLASS(addr)) {
					return 0;
				}
				if (IP_CLASSA(addr)) {
					u32_t net = addr & IP_CLASSA_NET;
					if ((net == 0) || (net == ((u32_t)IP_LOOPBACKNET << IP_CLASSA_NSHIFT))) {
						/* don't route loopback packets */
						return 0;
					}
				}
				return 1; //прошли все проверки, ок, можно пересылать
			}
    } 	
return 0;	
}
/**
 * Forwards an IP packet. It finds an appropriate route for the
 * packet, decrements the TTL value of the packet, adjusts the
 * checksum and outputs the packet on the appropriate interface.
 *
 * @param p the received IP packet (p->payload points to IP header)
 * @param inp the netif on which this packet was received
 * @return ERR_OK if the packet was processed (could return ERR_* if it wasn't
 *         processed, but currently always returns ERR_OK)
 */
err_t
Forward_TCPIP_From_ETH_Packet(struct pbuf *p, struct netif *inp)
{
	struct ip_hdr *iphdr;
	err_t err;  
  ip_addr_t current_src; //Source IP address of current_header  
  ip_addr_t current_dest;	//Destination IP address of current_header
	
			/* skip Ethernet header */
	if ((p->len < SIZEOF_ETH_HDR) || pbuf_header(p, (s16_t)- SIZEOF_ETH_HDR)) {
			BRIDGE_DEBUG_FORWARD(("FORWARD: ethernet_input: IPv4 packet dropped, too short (%"S16_F"/%"S16_F")\n",
				p->tot_len, SIZEOF_ETH_HDR));
			BRIDGE_DEBUG_FORWARD(("FORWARD: Can't move over header in packet\r\n"));
			return ERR_MEM;
	}
		
	iphdr = (struct ip_hdr *)p->payload;	
	
		  /* copy IP addresses to aligned ip_addr_t */
  ip_addr_copy_from_ip4(current_dest, iphdr->dest);
  ip_addr_copy_from_ip4(current_src, iphdr->src);
	temp_pc_addr.addr = current_src.addr;
  /* decrement TTL */
  IPH_TTL_SET(iphdr, IPH_TTL(iphdr) - 1);

	err = Change_IP_Rebuild_Packet(p, &inp->ip_addr, BRIDGE_CHANGE_IP_SRC);
	if(err != ERR_OK) return err;
	
  BRIDGE_DEBUG_FORWARD(("FORWARD: ETH forwarding packet from %"U16_F".%"U16_F".%"U16_F".%"U16_F"\n",
    ip4_addr1_16(&current_src), ip4_addr2_16(&current_src),
    ip4_addr3_16(&current_src), ip4_addr4_16(&current_src)));
		
  BRIDGE_DEBUG_FORWARD(("FORWARD: ETH forwarding packet to %"U16_F".%"U16_F".%"U16_F".%"U16_F"\n",
    ip4_addr1_16(&current_dest), ip4_addr2_16(&current_dest),
    ip4_addr3_16(&current_dest), ip4_addr4_16(&current_dest)));
	  err = inp->output(inp, p, &current_dest);
	if (err != ERR_OK)
	{
		BRIDGE_DEBUG_FORWARD(("\r\n FORWARD: ERROR OUT: %d\r\n",err));
		BRIDGE_DEBUG_FORWARD
		((		
			"ERR_OK         = 0\r\n\
			ERR_MEM        = -1\r\n\
			ERR_BUF        = -2\r\n\
			ERR_TIMEOUT    = -3\r\n\
			ERR_RTE        = -4\r\n\
			ERR_INPROGRESS = -5\r\n\
			ERR_VAL        = -6\r\n\
			ERR_WOULDBLOCK = -7\r\n\
			ERR_USE        = -8\r\n\
			ERR_ALREADY    = -9\r\n\
			ERR_ISCONN     = -10\r\n\
			ERR_CONN       = -11\r\n\
			ERR_IF         = -12\r\n\
			ERR_ABRT       = -13\r\n\
			ERR_RST        = -14\r\n\
			ERR_CLSD       = -15\r\n\
			ERR_ARG        = -16\r\n\
		"));
	}
	else
	{
		BRIDGE_DEBUG_FORWARD(("\r\nFORWARD: NO ERROR OUT - OK\r\n"));		
	}
return err;
}
/**
 * Forwards an IP packet. It finds an appropriate route for the
 * packet, decrements the TTL value of the packet, adjusts the
 * checksum and outputs the packet on the appropriate interface.
 *
 * @param p the received IP packet (p->payload points to IP header)
 * @param inp the netif on which this packet was received
 * @return ERR_OK if the packet was processed (could return ERR_* if it wasn't
 *         processed, but currently always returns ERR_OK)
 */
err_t
Forward_ETH_From_ETH_Packet(struct pbuf *p, struct netif *inp)
{
	struct ip_hdr *iphdr;
	err_t err;  
  ip_addr_t current_src; //Source IP address of current_header  
  ip_addr_t current_dest;	//Destination IP address of current_header
	
			/* skip Ethernet header */
	if ((p->len < SIZEOF_ETH_HDR)) {
		BRIDGE_DEBUG_FORWARD(("FORWARD: ethernet_input: IPv4 packet dropped, too short (%"S16_F"/%"S16_F")\n",
				p->tot_len, SIZEOF_ETH_HDR));
		BRIDGE_DEBUG_FORWARD(("MYFORWARD: Can't move over header in packet"));
			return ERR_MEM;
	}
		
	iphdr = (struct ip_hdr *)p->payload;	
	
		  /* copy IP addresses to aligned ip_addr_t */
  ip_addr_copy_from_ip4(current_dest, iphdr->dest);
  ip_addr_copy_from_ip4(current_src, iphdr->src);
	
  /* decrement TTL */
//  IPH_TTL_SET(iphdr, IPH_TTL(iphdr) - 1);

//	err = Change_IP_Rebuild_Packet(p, &inp->ip_addr, BRIDGE_CHANGE_IP_SRC);
//	if(err != ERR_OK) return err;
	
  BRIDGE_DEBUG_FORWARD(("ip4_forward: forwarding packet from %"U16_F".%"U16_F".%"U16_F".%"U16_F"\n",
    ip4_addr1_16(&current_src), ip4_addr2_16(&current_src),
    ip4_addr3_16(&current_src), ip4_addr4_16(&current_src)));
		
  BRIDGE_DEBUG_FORWARD(("ip4_forward: forwarding packet to %"U16_F".%"U16_F".%"U16_F".%"U16_F"\n",
    ip4_addr1_16(&current_dest), ip4_addr2_16(&current_dest),
    ip4_addr3_16(&current_dest), ip4_addr4_16(&current_dest)));
	  err = inp->linkoutput(inp, p);

	if (err != ERR_OK)
	{
		BRIDGE_DEBUG_FORWARD(("\r\nERROR OUT FORWARD: %d\r\n",err));
		BRIDGE_DEBUG_FORWARD
		((		
			"ERR_OK         = 0\r\n\
			ERR_MEM        = -1\r\n\
			ERR_BUF        = -2\r\n\
			ERR_TIMEOUT    = -3\r\n\
			ERR_RTE        = -4\r\n\
			ERR_INPROGRESS = -5\r\n\
			ERR_VAL        = -6\r\n\
			ERR_WOULDBLOCK = -7\r\n\
			ERR_USE        = -8\r\n\
			ERR_ALREADY    = -9\r\n\
			ERR_ISCONN     = -10\r\n\
			ERR_CONN       = -11\r\n\
			ERR_IF         = -12\r\n\
			ERR_ABRT       = -13\r\n\
			ERR_RST        = -14\r\n\
			ERR_CLSD       = -15\r\n\
			ERR_ARG        = -16\r\n\
		"));
	}
	else
	{
		BRIDGE_DEBUG_FORWARD(("\r\nFORWARD: NO ERROR OUT - OK\r\n"));		
	}
return err;
}
/**
 * Forwards an IP packet. It finds an appropriate route for the
 * packet, decrements the TTL value of the packet, adjusts the
 * checksum and outputs the packet on the appropriate interface.
 *
 * @param p the received IP packet (p->payload points to IP header)
 * @param inp the netif on which this packet was received
 * @return ERR_OK if the packet was processed (could return ERR_* if it wasn't
 *         processed, but currently always returns ERR_OK)
 */
err_t
Forward_ETH_From_TCPIP_Packet(struct pbuf *p, struct netif *inp)
{
	struct ip_hdr *iphdr;
	err_t err;  
  ip_addr_t current_src; //Source IP address of current_header  
  ip_addr_t current_dest;	//Destination IP address of current_header
	
	iphdr = (struct ip_hdr *)p->payload;	
	
		  /* copy IP addresses to aligned ip_addr_t */
  ip_addr_copy_from_ip4(current_dest, iphdr->dest);
  ip_addr_copy_from_ip4(current_src, iphdr->src);
	
  /* decrement TTL */
  IPH_TTL_SET(iphdr, IPH_TTL(iphdr) - 1);
	ip_addr_t ipaddr_pc;
	ipaddr_pc.addr = temp_pc_addr.addr;
	err = Change_IP_Rebuild_Packet(p, &ipaddr_pc, BRIDGE_CHANGE_IP_DEST);
	if(err != ERR_OK) return err;
	
  BRIDGE_DEBUG_FORWARD(("forward_from_PPP: forwarding packet from %"U16_F".%"U16_F".%"U16_F".%"U16_F"\n",
    ip4_addr1_16(&current_src), ip4_addr2_16(&current_src),
    ip4_addr3_16(&current_src), ip4_addr4_16(&current_src)));
		
  BRIDGE_DEBUG_FORWARD(("forward_from_PPP: forwarding packet to %"U16_F".%"U16_F".%"U16_F".%"U16_F"\n",
    ip4_addr1_16(&current_dest), ip4_addr2_16(&current_dest),
    ip4_addr3_16(&current_dest), ip4_addr4_16(&current_dest)));
		
	  err = inp->output(inp, p, &ipaddr_pc);
		//etharp_output(struct netif *netif, struct pbuf *q, const ip4_addr_t *ipaddr)
//		const struct eth_addr *dest;
//		err = ethernet_output(inp, p, (struct eth_addr*)(inp->hwaddr), &current_dest, ETHTYPE_IP);
//		ethernet_output(netif, p, ethsrc_addr, ethdst_addr, ETHTYPE_ARP);
//		err = etharp_query(inp, &current_dest, p);
	if (err != ERR_OK)
	{
		BRIDGE_DEBUG_FORWARD(("\r\nERROR OUT FORWARD: %d\r\n",err));
		BRIDGE_DEBUG_FORWARD
		((		
			"ERR_OK         = 0\r\n\
			ERR_MEM        = -1\r\n\
			ERR_BUF        = -2\r\n\
			ERR_TIMEOUT    = -3\r\n\
			ERR_RTE        = -4\r\n\
			ERR_INPROGRESS = -5\r\n\
			ERR_VAL        = -6\r\n\
			ERR_WOULDBLOCK = -7\r\n\
			ERR_USE        = -8\r\n\
			ERR_ALREADY    = -9\r\n\
			ERR_ISCONN     = -10\r\n\
			ERR_CONN       = -11\r\n\
			ERR_IF         = -12\r\n\
			ERR_ABRT       = -13\r\n\
			ERR_RST        = -14\r\n\
			ERR_CLSD       = -15\r\n\
			ERR_ARG        = -16\r\n\
		"));
	}
	else
	{
		BRIDGE_DEBUG_FORWARD(("\r\nFORWARD: NO ERROR OUT - OK\r\n"));		
	}
return err;
}
/**
 * Forwards an IP packet. It finds an appropriate route for the
 * packet, decrements the TTL value of the packet, adjusts the
 * checksum and outputs the packet on the appropriate interface.
 *
 * @param p the received IP packet (p->payload points to IP header)
 * @param inp the netif on which this packet was received
 * @return ERR_OK if the packet was processed (could return ERR_* if it wasn't
 *         processed, but currently always returns ERR_OK)
 */
static err_t Change_IP_Rebuild_Packet(struct pbuf *p, ip_addr_t *ip, uint16_t src_dest){
	struct pbuf *buff = NULL;
	struct ip_hdr *iphdr;
	struct tcp_hdr *tcphdr;
	u16_t ip_hlen = IP_HLEN;
	
	
	//IP4 HEADER!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	iphdr = (struct ip_hdr *)p->payload;
	if(src_dest==BRIDGE_CHANGE_IP_SRC){
		iphdr->src.addr = ip->addr;
		ip_data.current_iphdr_src.addr = ip->addr;
	}
	else {
		iphdr->dest.addr = ip->addr;
		ip_data.current_iphdr_dest.addr = ip->addr;
	}
		ip_hlen = IPH_HL(iphdr)*4;
		iphdr->_chksum = 0;
		IPH_CHKSUM_SET(iphdr, inet_chksum(iphdr, ip_hlen));
		int len = lwip_ntohs(IPH_LEN(iphdr)) - ip_hlen;
	
	if(IPH_PROTO(iphdr)==IP_PROTO_ICMP)
	{			
	}
	else if(IPH_PROTO(iphdr)==IP_PROTO_TCP)
	{
		tcphdr = (struct tcp_hdr *)((uint8_t *)p->payload+ip_hlen);
		buff = pbuf_alloc(PBUF_RAW, len, PBUF_RAM);
		if(buff==NULL)while(1) return ERR_BUF;

		tcphdr->chksum = 0;		

		memcpy( (uint8_t*)buff->payload, (uint8_t*)tcphdr, len);
    ((struct tcp_hdr *)(buff->payload))->chksum = ip_chksum_pseudo(buff, IP_PROTO_TCP,
      buff->tot_len, (ip_addr_t *)&iphdr->src, (ip_addr_t *)&iphdr->dest);		
	  memcpy((uint8_t*)tcphdr,(uint8_t*)buff->payload, len);
		
	if(buff!=NULL){pbuf_free(buff);}
	//printf("\r\n+++++++++FORWARD+++++++++++++\r\n");
	//my_tcp_debug_print(p);	
	}
	//TCP HEADER!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//tcp_debug_print(tcphdr);
	//ip4_debug_print(p);
	
	return ERR_OK;
}


/** Pass received raw characters to PPPoS to be decoded through lwIP TCPIP thread.
 *
 * @param ppp PPP descriptor index, returned by pppos_create()
 * @param s received data
 * @param l length of received data
 */
err_t PPPoS_Forward_ETH_From_TCPIP(ppp_pcb *ppp, struct pbuf *p)
{
  //err_t err;
	struct netif *inp = ppp_netif(ppp);
  LWIP_DEBUGF(TCPIP_DEBUG, ("MY! tcpip_inpkt: PACKET %p/%p\n", (void *)p, (void *)inp));
  LOCK_TCPIP_CORE();
  PPPoS_Input_For_Forward(ppp, (u8_t*)p->payload, p->len);
  UNLOCK_TCPIP_CORE();

  return ERR_OK;
}
/** Pass received raw characters to PPPoS to be decoded.
 *
 * @param ppp PPP descriptor index, returned by pppos_create()
 * @param s received data
 * @param l length of received data
 */
/* PPP's Asynchronous-Control-Character-Map.  The mask array is used
 * to select the specific bit for a character. */
#define ESCAPE_P(accm, c) ((accm)[(c) >> 3] & 1 << (c & 0x07))
/*
 * Values for FCS calculations.
 */
#define PPP_INITFCS     0xffff  /* Initial FCS value */
#define PPP_GOODFCS     0xf0b8  /* Good final FCS value */

static void PPPoS_Input_For_Forward(ppp_pcb *ppp, u8_t *s, int l)
{
  pppos_pcb *pppos = (pppos_pcb *)ppp->link_ctx_cb;
  struct pbuf *next_pbuf;
  u8_t cur_char;
  u8_t escaped;

  if (!pppos->open) {
		BRIDGE_DEBUG_FORWARD(("FORWARD: PPPoS not open!!!\r\n"));
    return;
  }
	
  PPPDEBUG(LOG_DEBUG, ("MY! pppos_input[%d]: got %d bytes\n", ppp->netif->num, l));
  while (l-- > 0) {
    cur_char = *s++;

    escaped = ESCAPE_P(pppos->in_accm, cur_char);
    /* Handle special characters. */
    if (escaped) {
      /* Check for escape sequences. */
      /* XXX Note that this does not handle an escaped 0x5d character which
       * would appear as an escape character.  Since this is an ASCII ']'
       * and there is no reason that I know of to escape it, I won't complicate
       * the code to handle this case. GLL */
      if (cur_char == PPP_ESCAPE) {
        pppos->in_escaped = 1;
      /* Check for the flag character. */
      } else if (cur_char == PPP_FLAG) {
        /* If this is just an extra flag character, ignore it. */
        if (pppos->in_state <= PDADDRESS) {
          /* ignore it */;
        /* If we haven't received the packet header, drop what has come in. */
        } else if (pppos->in_state < PDDATA) {
          PPPDEBUG(LOG_WARNING,
                   ("MY! pppos_input[%d]: Dropping incomplete packet %d\n",
                    ppp->netif->num, pppos->in_state));
          LINK_STATS_INC(link.lenerr);
          //pppos_input_drop(pppos);
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!				
					while(1){}
        /* If the fcs is invalid, drop the packet. */
        } else if (pppos->in_fcs != PPP_GOODFCS) {
          PPPDEBUG(LOG_INFO,
                   ("MY! pppos_input[%d]: Dropping bad fcs 0x%"X16_F" proto=0x%"X16_F"\n",
                    ppp->netif->num, pppos->in_fcs, pppos->in_protocol));
          /* Note: If you get lots of these, check for UART frame errors or try different baud rate */
          LINK_STATS_INC(link.chkerr);
          //pppos_input_drop(pppos);
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!				
					while(1){}
        /* Otherwise it's a good packet so pass it on. */
        } else {
          struct pbuf *inp;
          /* Trim off the checksum. */
          if(pppos->in_tail->len > 2) {
            pppos->in_tail->len -= 2;

            pppos->in_tail->tot_len = pppos->in_tail->len;
            if (pppos->in_tail != pppos->in_head) {
              pbuf_cat(pppos->in_head, pppos->in_tail);
            }
          } else {
            pppos->in_tail->tot_len = pppos->in_tail->len;
            if (pppos->in_tail != pppos->in_head) {
              pbuf_cat(pppos->in_head, pppos->in_tail);
            }

            pbuf_realloc(pppos->in_head, pppos->in_head->tot_len - 2);
          }

          /* Dispatch the packet thereby consuming it. */
          inp = pppos->in_head;
          /* Packet consumed, release our references. */
          pppos->in_head = NULL;
          pppos->in_tail = NULL;
//#if IP_FORWARD || LWIP_IPV6_FORWARD
          /* hide the room for Ethernet forwarding header */
          pbuf_header(inp, -(s16_t)(PBUF_LINK_ENCAPSULATION_HLEN + PBUF_LINK_HLEN));
//#endif /* IP_FORWARD || LWIP_IPV6_FORWARD */
#if PPP_INPROC_IRQ_SAFE
          if(tcpip_callback_with_block(pppos_input_callback, inp, 0) != ERR_OK) {
            PPPDEBUG(LOG_ERR, ("MY! pppos_input[%d]: tcpip_callback() failed, dropping packet\n", ppp->netif->num));
            pbuf_free(inp);
            LINK_STATS_INC(link.drop);
            MIB2_STATS_NETIF_INC(ppp->netif, ifindiscards);
          }
#else /* PPP_INPROC_IRQ_SAFE */
          PPP_Input_For_Forward(ppp, inp);
#endif /* PPP_INPROC_IRQ_SAFE */
        }

        /* Prepare for a new packet. */
        pppos->in_fcs = PPP_INITFCS;
        pppos->in_state = PDADDRESS;
        pppos->in_escaped = 0;
      /* Other characters are usually control characters that may have
       * been inserted by the physical layer so here we just drop them. */
      } else {
        PPPDEBUG(LOG_WARNING,
                 ("MY! pppos_input[%d]: Dropping ACCM char <%d>\n", ppp->netif->num, cur_char));
      }
    /* Process other characters. */
    } else {
      /* Unencode escaped characters. */
      if (pppos->in_escaped) {
        pppos->in_escaped = 0;
        cur_char ^= PPP_TRANS;
      }

      /* Process character relative to current state. */
      switch(pppos->in_state) {
        case PDIDLE:                    /* Idle state - waiting. */
          /* Drop the character if it's not 0xff
           * we would have processed a flag character above. */
          if (cur_char != PPP_ALLSTATIONS) {
            break;
          }
          /* no break */
          /* Fall through */

        case PDSTART:                   /* Process start flag. */
          /* Prepare for a new packet. */
          pppos->in_fcs = PPP_INITFCS;
          /* no break */
          /* Fall through */

        case PDADDRESS:                 /* Process address field. */
          if (cur_char == PPP_ALLSTATIONS) {
            pppos->in_state = PDCONTROL;
            break;
          }
          /* no break */

          /* Else assume compressed address and control fields so
           * fall through to get the protocol... */
        case PDCONTROL:                 /* Process control field. */
          /* If we don't get a valid control code, restart. */
          if (cur_char == PPP_UI) {
            pppos->in_state = PDPROTOCOL1;
            break;
          }
          /* no break */

#if 0
          else {
            PPPDEBUG(LOG_WARNING,
                     ("MY! pppos_input[%d]: Invalid control <%d>\n", ppp->netif->num, cur_char));
            pppos->in_state = PDSTART;
          }
#endif
        case PDPROTOCOL1:               /* Process protocol field 1. */
          /* If the lower bit is set, this is the end of the protocol
           * field. */
          if (cur_char & 1) {
            pppos->in_protocol = cur_char;
            pppos->in_state = PDDATA;
          } else {
            pppos->in_protocol = (u16_t)cur_char << 8;
            pppos->in_state = PDPROTOCOL2;
          }
          break;
        case PDPROTOCOL2:               /* Process protocol field 2. */
          pppos->in_protocol |= cur_char;
          pppos->in_state = PDDATA;
          break;
        case PDDATA:                    /* Process data byte. */
          /* Make space to receive processed data. */
          if (pppos->in_tail == NULL || pppos->in_tail->len == PBUF_POOL_BUFSIZE) {
            u16_t pbuf_alloc_len;
            if (pppos->in_tail != NULL) {
              pppos->in_tail->tot_len = pppos->in_tail->len;
              if (pppos->in_tail != pppos->in_head) {
                pbuf_cat(pppos->in_head, pppos->in_tail);
                /* give up the in_tail reference now */
                pppos->in_tail = NULL;
              }
            }
            /* If we haven't started a packet, we need a packet header. */
            pbuf_alloc_len = 0;
//#if IP_FORWARD || LWIP_IPV6_FORWARD
            /* If IP forwarding is enabled we are reserving PBUF_LINK_ENCAPSULATION_HLEN
             * + PBUF_LINK_HLEN bytes so the packet is being allocated with enough header
             * space to be forwarded (to Ethernet for example).
             */
            if (pppos->in_head == NULL) {
              pbuf_alloc_len = PBUF_LINK_ENCAPSULATION_HLEN + PBUF_LINK_HLEN;
            }
//#endif /* IP_FORWARD || LWIP_IPV6_FORWARD */
            next_pbuf = pbuf_alloc(PBUF_RAW, pbuf_alloc_len, PBUF_POOL);
            if (next_pbuf == NULL) {
              /* No free buffers.  Drop the input packet and let the
               * higher layers deal with it.  Continue processing
               * the received pbuf chain in case a new packet starts. */
              PPPDEBUG(LOG_ERR, ("MY! pppos_input[%d]: NO FREE PBUFS!\n", ppp->netif->num));
              LINK_STATS_INC(link.memerr);
							//pppos_input_drop(pppos);
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!				
							while(1){}
              pppos->in_state = PDSTART;  /* Wait for flag sequence. */
              break;
            }
            if (pppos->in_head == NULL) {
              u8_t *payload = ((u8_t*)next_pbuf->payload) + pbuf_alloc_len;
#if PPP_INPROC_IRQ_SAFE
              ((struct pppos_input_header*)payload)->ppp = ppp;
              payload += sizeof(struct pppos_input_header);
              next_pbuf->len += sizeof(struct pppos_input_header);
#endif /* PPP_INPROC_IRQ_SAFE */
              next_pbuf->len += sizeof(pppos->in_protocol);
              *(payload++) = pppos->in_protocol >> 8;
              *(payload) = pppos->in_protocol & 0xFF;
              pppos->in_head = next_pbuf;
            }
            pppos->in_tail = next_pbuf;
          }
          /* Load character into buffer. */
          ((u8_t*)pppos->in_tail->payload)[pppos->in_tail->len++] = cur_char;
          break;
        default:
          break;
      }

      /* update the frame check sequence number. */
      pppos->in_fcs = PPP_FCS(pppos->in_fcs, cur_char);
    }
  } /* while (l-- > 0), all bytes processed */
}

/*
 * Pass the processed input packet to the appropriate handler.
 * This function and all handlers run in the context of the tcpip_thread
 */
static void PPP_Input_For_Forward(ppp_pcb *pcb, struct pbuf *pb) {
  u16_t protocol;
#if PPP_DEBUG && PPP_PROTOCOLNAME
    const char *pname;
#endif /* PPP_DEBUG && PPP_PROTOCOLNAME */

  magic_randomize();

  if (pb->len < 2) {
    PPPDEBUG(LOG_ERR, ("MY! ppp_input[%d]: packet too short\n", pcb->netif->num));
    goto drop;
  }
  protocol = (((u8_t *)pb->payload)[0] << 8) | ((u8_t*)pb->payload)[1];

#if PRINTPKT_SUPPORT
  ppp_dump_packet(pcb, "rcvd", (unsigned char *)pb->payload, pb->len);
#endif /* PRINTPKT_SUPPORT */

  pbuf_header(pb, -(s16_t)sizeof(protocol));
  /*
   * Toss all non-LCP packets unless LCP is OPEN.
   */
  if (protocol != PPP_LCP && pcb->lcp_fsm.state != PPP_FSM_OPENED) {
    ppp_dbglog("MY! Discarded non-LCP packet when LCP not open");
    goto drop;
  }

  /*
   * Until we get past the authentication phase, toss all packets
   * except LCP, LQR and authentication packets.
   */
  if (pcb->phase <= PPP_PHASE_AUTHENTICATE
   && !(protocol == PPP_LCP
#if LQR_SUPPORT
   || protocol == PPP_LQR
#endif /* LQR_SUPPORT */
#if PAP_SUPPORT
   || protocol == PPP_PAP
#endif /* PAP_SUPPORT */
#if CHAP_SUPPORT
   || protocol == PPP_CHAP
#endif /* CHAP_SUPPORT */
#if EAP_SUPPORT
   || protocol == PPP_EAP
#endif /* EAP_SUPPORT */
   )) {
    ppp_dbglog("MY! discarding proto 0x%x in phase %d", protocol, pcb->phase);
    goto drop;
  }

#if CCP_SUPPORT
#if MPPE_SUPPORT
  /*
   * MPPE is required and unencrypted data has arrived (this
   * should never happen!). We should probably drop the link if
   * the protocol is in the range of what should be encrypted.
   * At the least, we drop this packet.
   */
  if (pcb->settings.require_mppe && protocol != PPP_COMP && protocol < 0x8000) {
    PPPDEBUG(LOG_ERR, ("MY! ppp_input[%d]: MPPE required, received unencrypted data!\n", pcb->netif->num));
    goto drop;
  }
#endif /* MPPE_SUPPORT */

  if (protocol == PPP_COMP) {
    u8_t *pl;

    switch (pcb->ccp_receive_method) {
#if MPPE_SUPPORT
    case CI_MPPE:
      if (mppe_decompress(pcb, &pcb->mppe_decomp, &pb) != ERR_OK) {
        goto drop;
      }
      break;
#endif /* MPPE_SUPPORT */
    default:
      PPPDEBUG(LOG_ERR, ("MY! ppp_input[%d]: bad CCP receive method\n", pcb->netif->num));
      goto drop; /* Cannot really happen, we only negotiate what we are able to do */
    }

    /* Assume no PFC */
    if (pb->len < 2) {
      goto drop;
    }

    /* Extract and hide protocol (do PFC decompression if necessary) */
    pl = (u8_t*)pb->payload;
    if (pl[0] & 0x01) {
      protocol = pl[0];
      pbuf_header(pb, -(s16_t)1);
    } else {
      protocol = (pl[0] << 8) | pl[1];
      pbuf_header(pb, -(s16_t)2);
    }
  }
#endif /* CCP_SUPPORT */

  switch(protocol) {

#if PPP_IPV4_SUPPORT
    case PPP_IP:            /* Internet Protocol */
      PPPDEBUG(LOG_INFO, ("MY! ppp_input[%d]: ip in pbuf len=%d\n", pcb->netif->num, pb->tot_len));
      //ip4_input(pb, pcb->netif);	
			//if((Check_TCPIP_Packet_For_Forward(pb, netif) != 0)) //проверяем не пересылать ли пакет
			//{
				//если да, то пересылаем
				Forward_ETH_From_TCPIP_Packet(pb, &gnetif);
			//}
			pbuf_free(pb);
      return;
#endif /* PPP_IPV4_SUPPORT */

#if PPP_IPV6_SUPPORT
    case PPP_IPV6:          /* Internet Protocol Version 6 */
      PPPDEBUG(LOG_INFO, ("MY! ppp_input[%d]: ip6 in pbuf len=%d\n", pcb->netif->num, pb->tot_len));
      ip6_input(pb, pcb->netif);
      return;
#endif /* PPP_IPV6_SUPPORT */

#if VJ_SUPPORT
    case PPP_VJC_COMP:      /* VJ compressed TCP */
      /*
       * Clip off the VJ header and prepend the rebuilt TCP/IP header and
       * pass the result to IP.
       */
      PPPDEBUG(LOG_INFO, ("MY! ppp_input[%d]: vj_comp in pbuf len=%d\n", pcb->netif->num, pb->tot_len));
      if (pcb->vj_enabled && vj_uncompress_tcp(&pb, &pcb->vj_comp) >= 0) {
        ip4_input(pb, pcb->netif);
        return;
      }
      /* Something's wrong so drop it. */
      PPPDEBUG(LOG_WARNING, ("MY! ppp_input[%d]: Dropping VJ compressed\n", pcb->netif->num));
      break;

    case PPP_VJC_UNCOMP:    /* VJ uncompressed TCP */
      /*
       * Process the TCP/IP header for VJ header compression and then pass
       * the packet to IP.
       */
      PPPDEBUG(LOG_INFO, ("MY! ppp_input[%d]: vj_un in pbuf len=%d\n", pcb->netif->num, pb->tot_len));
      if (pcb->vj_enabled && vj_uncompress_uncomp(pb, &pcb->vj_comp) >= 0) {
        ip4_input(pb, pcb->netif);
        return;
      }
      /* Something's wrong so drop it. */
      PPPDEBUG(LOG_WARNING, ("MY! ppp_input[%d]: Dropping VJ uncompressed\n", pcb->netif->num));
      break;
#endif /* VJ_SUPPORT */

    default: {
      int i;
      const struct protent *protp;

      /*
       * Upcall the proper protocol input routine.
       */
      for (i = 0; (protp = protocols[i]) != NULL; ++i) {
        if (protp->protocol == protocol) {
          pb = ppp_singlebuf(pb);
          (*protp->input)(pcb, (u8_t*)pb->payload, pb->len);
          goto out;
        }
#if 0   /* UNUSED
         *
         * This is actually a (hacked?) way for the Linux kernel to pass a data
         * packet to pppd. pppd in normal condition only do signaling
         * (LCP, PAP, CHAP, IPCP, ...) and does not handle any data packet at all.
         *
         * We don't even need this interface, which is only there because of PPP
         * interface limitation between Linux kernel and pppd. For MPPE, which uses
         * CCP to negotiate although it is not really a (de)compressor, we added
         * ccp_resetrequest() in CCP and MPPE input data flow is calling either
         * ccp_resetrequest() or lcp_close() if the issue is, respectively, non-fatal
         * or fatal, this is what ccp_datainput() really do.
         */
        if (protocol == (protp->protocol & ~0x8000)
          && protp->datainput != NULL) {
          (*protp->datainput)(pcb, pb->payload, pb->len);
          goto out;
        }
#endif /* UNUSED */
      }

#if PPP_DEBUG
#if PPP_PROTOCOLNAME
      pname = protocol_name(protocol);
      if (pname != NULL) {
        ppp_warn("MY! Unsupported protocol '%s' (0x%x) received", pname, protocol);
      } else
#endif /* PPP_PROTOCOLNAME */
        ppp_warn("MY! Unsupported protocol 0x%x received", protocol);
#endif /* PPP_DEBUG */
        pbuf_header(pb, (s16_t)sizeof(protocol));
        lcp_sprotrej(pcb, (u8_t*)pb->payload, pb->len);
      }
      break;
  }

drop:
  LINK_STATS_INC(link.drop);
out:
  pbuf_free(pb);
}

