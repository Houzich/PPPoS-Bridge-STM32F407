
/**
  ******************************************************************************
  * File Name          : lwipopts.h
  * Description        : This file overrides LwIP stack default configuration
  *                      done in opt.h file.
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
 
/* Define to prevent recursive inclusion --------------------------------------*/
#ifndef __LWIPOPTS__H__
#define __LWIPOPTS__H__

#include "stm32f4xx_hal.h"

/*-----------------------------------------------------------------------------*/
/* Current version of LwIP supported by CubeMx: 2.0.3 -*/
/*-----------------------------------------------------------------------------*/

/* Within 'USER CODE' section, code will be kept by default at each generation */
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

#ifdef __cplusplus
 extern "C" {
#endif

/* STM32CubeMX Specific Parameters (not defined in opt.h) ---------------------*/
/* Parameters set in STM32CubeMX LwIP Configuration GUI -*/
/*----- WITH_RTOS enabled (Since FREERTOS is set) -----*/
#define WITH_RTOS 1
/*----- CHECKSUM_BY_HARDWARE disabled -----*/
#define CHECKSUM_BY_HARDWARE 0
/*-----------------------------------------------------------------------------*/

/* LwIP Stack Parameters (modified compared to initialization value in opt.h) -*/
/* Parameters set in STM32CubeMX LwIP Configuration GUI -*/
/*----- Default Value for LWIP_DNS: 0 ---*/
#define LWIP_DNS 1
/*----- Default Value for MEMP_NUM_UDP_PCB: 4 ---*/
#define MEMP_NUM_UDP_PCB 6
/*----- Default Value for MEMP_NUM_TCP_PCB: 5 ---*/
#define MEMP_NUM_TCP_PCB 10
/*----- Value in opt.h for MEM_ALIGNMENT: 1 -----*/
#define MEM_ALIGNMENT 4
/*----- Default Value for MEM_SIZE: 1600 ---*/
#define MEM_SIZE 20480
/*----- Default Value for MEMP_NUM_PBUF: 16 ---*/
#define MEMP_NUM_PBUF 10
/*----- Default Value for MEMP_NUM_TCP_PCB_LISTEN: 8 ---*/
#define MEMP_NUM_TCP_PCB_LISTEN 5
/*----- Default Value for MEMP_NUM_TCP_SEG: 16 ---*/
#define MEMP_NUM_TCP_SEG 12
/*----- Default Value for PBUF_POOL_BUFSIZE: 592 ---*/
#define PBUF_POOL_BUFSIZE 1524
/*----- Value in opt.h for LWIP_ETHERNET: LWIP_ARP || PPPOE_SUPPORT -*/
#define LWIP_ETHERNET 1
/*----- Value in opt.h for LWIP_DNS_SECURE: (LWIP_DNS_SECURE_RAND_XID | LWIP_DNS_SECURE_NO_MULTIPLE_OUTSTANDING | LWIP_DNS_SECURE_RAND_SRC_PORT) -*/
#define LWIP_DNS_SECURE 7
/*----- Default Value for TCP_WND: 5840 ---*/
#define TCP_WND 2920
/*----- Default Value for TCP_QUEUE_OOSEQ: 1 ---*/
#define TCP_QUEUE_OOSEQ 0
/*----- Default Value for TCP_MSS: 536 ---*/
#define TCP_MSS 1460
/*----- Default Value for TCP_SND_BUF: 2920 ---*/
#define TCP_SND_BUF 5840
/*----- Default Value for TCP_SND_QUEUELEN: 17 ---*/
#define TCP_SND_QUEUELEN 8
/*----- Value in opt.h for TCP_SNDQUEUELOWAT: LWIP_MAX(TCP_SND_QUEUELEN)/2, 5) -*/
#define TCP_SNDQUEUELOWAT 5
/*----- Value in opt.h for TCPIP_THREAD_STACKSIZE: 0 -----*/
#define TCPIP_THREAD_STACKSIZE 1024
/*----- Value in opt.h for TCPIP_THREAD_PRIO: 1 -----*/
#define TCPIP_THREAD_PRIO 3
/*----- Value in opt.h for TCPIP_MBOX_SIZE: 0 -----*/
#define TCPIP_MBOX_SIZE 6
/*----- Value in opt.h for SLIPIF_THREAD_STACKSIZE: 0 -----*/
#define SLIPIF_THREAD_STACKSIZE 1024
/*----- Value in opt.h for SLIPIF_THREAD_PRIO: 1 -----*/
#define SLIPIF_THREAD_PRIO 3
/*----- Value in opt.h for DEFAULT_THREAD_STACKSIZE: 0 -----*/
#define DEFAULT_THREAD_STACKSIZE 1024
/*----- Value in opt.h for DEFAULT_THREAD_PRIO: 1 -----*/
#define DEFAULT_THREAD_PRIO 3
/*----- Value in opt.h for DEFAULT_UDP_RECVMBOX_SIZE: 0 -----*/
#define DEFAULT_UDP_RECVMBOX_SIZE 6
/*----- Value in opt.h for DEFAULT_TCP_RECVMBOX_SIZE: 0 -----*/
#define DEFAULT_TCP_RECVMBOX_SIZE 6
/*----- Value in opt.h for DEFAULT_ACCEPTMBOX_SIZE: 0 -----*/
#define DEFAULT_ACCEPTMBOX_SIZE 6
/*----- Value in opt.h for RECV_BUFSIZE_DEFAULT: INT_MAX -----*/
#define RECV_BUFSIZE_DEFAULT 2000000000
/*----- Default Value for PPP_SUPPORT: 0 ---*/
#define PPP_SUPPORT 1
/*----- Default Value for PAP_SUPPORT: 0 ---*/
#define PAP_SUPPORT 1
/*----- Value in opt.h for LWIP_STATS: 1 -----*/
#define LWIP_STATS 0
/*----- Default Value for LWIP_CHECKSUM_CTRL_PER_NETIF: 0 ---*/
#define LWIP_CHECKSUM_CTRL_PER_NETIF 1
/*----- Default Value for NETIF_DEBUG: LWIP_DBG_OFF ---*/
#define NETIF_DEBUG LWIP_DBG_ON
/*----- Default Value for API_LIB_DEBUG: LWIP_DBG_OFF ---*/
#define API_LIB_DEBUG LWIP_DBG_ON
/*----- Default Value for API_MSG_DEBUG: LWIP_DBG_OFF ---*/
#define API_MSG_DEBUG LWIP_DBG_ON
/*----- Default Value for SOCKETS_DEBUG: LWIP_DBG_OFF ---*/
#define SOCKETS_DEBUG LWIP_DBG_ON
/*----- Default Value for ICMP_DEBUG: LWIP_DBG_OFF ---*/
#define ICMP_DEBUG LWIP_DBG_ON
/*----- Default Value for IGMP_DEBUG: LWIP_DBG_OFF ---*/
#define IGMP_DEBUG LWIP_DBG_ON
/*----- Default Value for INET_DEBUG: LWIP_DBG_OFF ---*/
#define INET_DEBUG LWIP_DBG_ON
/*----- Default Value for IP_DEBUG: LWIP_DBG_OFF ---*/
#define IP_DEBUG LWIP_DBG_OFF
/*----- Default Value for IP_REASS_DEBUG: LWIP_DBG_OFF ---*/
#define IP_REASS_DEBUG LWIP_DBG_OFF
/*----- Default Value for TCP_INPUT_DEBUG: LWIP_DBG_OFF ---*/
#define TCP_INPUT_DEBUG LWIP_DBG_OFF
/*----- Default Value for TCP_FR_DEBUG: LWIP_DBG_OFF ---*/
#define TCP_FR_DEBUG LWIP_DBG_OFF
/*----- Default Value for TCP_RTO_DEBUG: LWIP_DBG_OFF ---*/
#define TCP_RTO_DEBUG LWIP_DBG_OFF
/*----- Default Value for TCP_CWND_DEBUG: LWIP_DBG_OFF ---*/
#define TCP_CWND_DEBUG LWIP_DBG_OFF
/*----- Default Value for TCP_WND_DEBUG: LWIP_DBG_OFF ---*/
#define TCP_WND_DEBUG LWIP_DBG_OFF
/*----- Default Value for TCP_OUTPUT_DEBUG: LWIP_DBG_OFF ---*/
#define TCP_OUTPUT_DEBUG LWIP_DBG_OFF
/*----- Default Value for TCP_RST_DEBUG: LWIP_DBG_OFF ---*/
#define TCP_RST_DEBUG LWIP_DBG_OFF
/*----- Default Value for TCP_QLEN_DEBUG: LWIP_DBG_OFF ---*/
#define TCP_QLEN_DEBUG LWIP_DBG_OFF
/*----- Default Value for UDP_DEBUG: LWIP_DBG_OFF ---*/
#define UDP_DEBUG LWIP_DBG_OFF
/*----- Default Value for TCPIP_DEBUG: LWIP_DBG_OFF ---*/
#define TCPIP_DEBUG LWIP_DBG_OFF
/*----- Default Value for DHCP_DEBUG: LWIP_DBG_OFF ---*/
#define DHCP_DEBUG LWIP_DBG_OFF
/*----- Default Value for AUTOIP_DEBUG: LWIP_DBG_OFF ---*/
#define AUTOIP_DEBUG LWIP_DBG_OFF
/*----- Default Value for DNS_DEBUG: LWIP_DBG_OFF ---*/
#define DNS_DEBUG LWIP_DBG_OFF
/*----- Default Value for PPP_DEBUG: LWIP_DBG_OFF ---*/
#define PPP_DEBUG LWIP_DBG_ON
/*-----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */
#define LWIP_DEBUG 											LWIP_DBG_ON
#define LWIP_DBG_TYPES_ON               LWIP_DBG_ON

#define MEM_ALIGNMENT 									4
#define MEMP_NUM_SYS_TIMEOUT 						10
#define PBUF_POOL_SIZE 									8
#define UDP_TTL 												255
#define DEFAULT_UDP_RECVMBOX_SIZE 			6

#define LWIP_TCPIP_CORE_LOCKING 				1
#define LWIP_TCPIP_CORE_LOCKING_INPUT		1 //--? под вопрсом остаются


//----------PPPoS
//#define PPP_SUPPORT 				1
#define PPPOS_SUPPORT 			1
#define PRINTPKT_SUPPORT 		1
#define PPP_IPV4_SUPPORT 		1
#define PAP_SUPPORT 				1
#define LWIP_PPP_API   			1 
#define PPP_PROTOCOLNAME 		1


/* USER CODE END 1 */

#ifdef __cplusplus
}
#endif
#endif /*__LWIPOPTS__H_H */

/************************* (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
