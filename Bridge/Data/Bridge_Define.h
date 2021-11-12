
#ifndef __BRIDGE_DEFINE_H__
#define __BRIDGE_DEFINE_H__

#define inet_addr_my(a, b, c, d) (((uint32_t)a) | ((uint32_t)b << 8) | \
								  ((uint32_t)c << 16) | ((uint32_t)d << 24))

//#define PC_IP						inet_addr_my(192,168,1,222)
#define SITE_PORT 80
#define SITE_TEST_IP_1 inet_addr_my(54, 152, 208, 69)
#define SITE_TEST_IP_2 inet_addr_my(92, 124, 147, 233)
#define SITE_TEST_IP_3 inet_addr_my(81, 177, 141, 229)

#define BRIDGE_USE_ETHERNET 1
#define BRIDGE_USE_GSM 			1
#define BRIDGE_USE_PPPOS 		1
#define BRIDGE_USE_FORWARD 	1

#define BRIDGE_DEBUG_ETHERNET printf
#define BRIDGE_DEBUG_GSM(x) printf x
#define BRIDGE_DEBUG_PPPOS(x) printf x
#define BRIDGE_DEBUG_HTTP_PPPOS printf
#define BRIDGE_DEBUG_FORWARD(x) printf x
#define BRIDGE_DEBUG_FREERTOS(x) printf x

//HTTP Server
#define HTTP_SERVER_THREAD_STACKSIZE 	1024
#define HTTP_SERVER_THREAD_PRIO 			osPriorityAboveNormal
#define ETH_INPUT_STACK_SIZE 					1024
#define ETH_INPUT_THREAD_PRIO 				osPriorityHigh
//GSM Init
#define GSM_INIT_THREAD_STACKSIZE 512
#define GSM_INIT_THREAD_PRIO 			osPriorityHigh
//PPPoS
#define PPPOS_STACK_SIZE 				1024
#define PPPOS_THREAD_PRIO 			osPriorityAboveNormal
#define PPPOS_INPUT_STACK_SIZE 	1024
#define PPPOS_INPUT_THREAD_PRIO osPriorityHigh

#define ENC28_SEMAPHORE_TIMEOUT (osWaitForever)

//GSM CONFIG
#define CONFIG_GSM_INTERNET_APN "vmi.velcom.by"
#define CONFIG_GSM_INTERNET_USER "vmi"
#define CONFIG_GSM_INTERNET_PASSWORD "vmi"
#define BUF_TX_SIZE_GSM 1024
#define BUF_RX_SIZE_GSM 10240

#define PPPOS_MUTEX_TIMEOUT (osWaitForever)

//FORWARD
#define BRIDGE_CHANGE_IP_SRC		0
#define BRIDGE_CHANGE_IP_DEST		1


//#ifdef BRIDGE_USE_PPPOS
//#ifndef BRIDGE_USE_GSM
//#error "If you want to use PPPOS, BRIDGE_USE_GSM needs to be defined in your GlobalDefine.h"
//#endif

#endif /* __BRIDGE_DEFINE_H__ */
