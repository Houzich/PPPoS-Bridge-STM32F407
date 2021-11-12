
/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "stm32f4xx_hal.h"
#include "lwip/opt.h"
#include "lwip/timeouts.h"
#include "lwip/ip4.h"
#include "lwip/inet.h"
#include "lwip/dns.h"
#include "netif/ethernet.h"
#include "netif/etharp.h"

#include "Bridge_Data.h"
#include "GSM.h"
#include "Application_PPPoS.h"
#include "PPPos_Interface.h"
#include "Forward.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Network interface name */
#define IFNAME0 's'
#define IFNAME1 't'

struct netif pppos_netif;						// The PPP IP interface
//ppp_pcb *pppos = NULL;							// The PPP control block
Modem_HandleTypeDef modem_pppos;
PPPOS_HandleTypeDef	hpppos;	


static void pppos_low_level_init(Modem_HandleTypeDef *modem);
static struct pbuf * pppos_low_level_input(GSM_HandleTypeDef *gsm);
static void ppp_status_cb(ppp_pcb *pcb, int err_code, void *ctx);
//static err_t pppos_low_level_output(struct netif *netif, struct pbuf *p);

void PPPOS_Input_Task(void const * argument)
{
	Modem_HandleTypeDef *modem = (Modem_HandleTypeDef *)argument;
		for( ;; )
		{
			// === Check if disconnected ===
			if ((Modem_PPPoS_Status_Get() == PPPOS_STATE_DISCONNECTED)&&(Modem_PPPoS_Error_Get() == PPPOS_ERROR_CONNECTION_LOST)) {
				Modem_PPPoS_Status_Set(PPPOS_STATE_RECONNECT);
				Modem_PPPoS_Error_Set(PPPOS_ERROR_OK);
				PPPOS_LOG_Phase(modem->ppp);
				ppp_close(modem->ppp, 0);				
				while(Modem_PPPoS_Status_Get() != PPPOS_STATE_DISCONNECTED){
				PPPoS_Interface_Input(modem);
				osDelay(1);
				}
				PPPoS_Interface_Init(modem);
			}
			
			PPPoS_Interface_Input(modem);
			osDelay(1);
		}	
}
void PPPoS_Interface_Input(Modem_HandleTypeDef *modem)
{
	ppp_pcb *pcb = (ppp_pcb *) modem->ppp;
	GSM_HandleTypeDef *gsm = (GSM_HandleTypeDef *)modem->gsm;
	struct pbuf *p = NULL;
	err_t err;
	
	p = pppos_low_level_input(gsm);
	if (p != NULL)
	{
		if(fl_send_forward_packet>0)
		{
//			u16_t protocol = (((u8_t *)p->payload)[0] << 8) | ((u8_t*)p->payload)[1];
			u16_t protocol =  ((u8_t*)p->payload)[1];
			if(protocol == PPP_IP){
				PPPoS_Forward_ETH_From_TCPIP(pcb, p);
			}else{
				err = pppos_input_tcpip(pcb, (u8_t *)(p->payload), p->len);
			}
			err = ERR_OK;
		}
		else
		{
			err = pppos_input_tcpip(pcb, (u8_t *)(p->payload), p->len);	
		}						
//		if (err != ERR_OK)
//			while(1){BRIDGE_DEBUG_PPPOS("ERROR PPPOS INPUT");}
		if (p != NULL) pbuf_free(p);
	}
}

static struct pbuf * pppos_low_level_input(GSM_HandleTypeDef *gsm)
{
  struct pbuf *p = NULL;
			int firs_byte = -1;
			int cont_not_zero = 0;
			for (int i = 0; i < gsm->rx_buff_size; i++)
			{
				if (gsm->rx_buff[i] != 0)cont_not_zero++;
				if ((gsm->rx_buff[i] == 0x7E) && (firs_byte == -1))
					firs_byte = i;

				if ((gsm->rx_buff[i] == 0x7E) && (i != firs_byte) && (firs_byte!=-1))
				{
					int len = i - firs_byte + 1;
					if((len-2)<=0){
						BRIDGE_DEBUG_PPPOS(("\r\n\r\nERROR PPPOS BUFFER HANDLER!!!!!!\r\n\r\n"));
						for (int x = 0; x <= firs_byte; x++)gsm->rx_buff[x] = 0;
						return NULL;
					}							
					else
					{
						if(len>200){
							BRIDGE_DEBUG_PPPOS(("\r\n\r\nBIG PACKET: %d\r\n\r\n",len));}

						/* We allocate a pbuf chain of pbufs from the Lwip buffer pool */
						p = pbuf_alloc(PBUF_RAW, len, PBUF_RAM);
						
						if(p==NULL)while(1){BRIDGE_DEBUG_PPPOS(("Error PBUFF"));}
						
						memcpy( (uint8_t*)p->payload, (uint8_t*)&gsm->rx_buff[firs_byte], len);
						p->len=len;
						for (int x = 0; x <= i; x++)
							gsm->rx_buff[x] = 0;

						if ((i + 1) == gsm->link_cb->get_rx_count(gsm->usart,gsm->rx_buff_size))
						{
							gsm->link_cb->rx_dma_reset(gsm->usart, (uint8_t *)gsm->rx_buff, gsm->rx_buff_size);
						}
						return p;
					}
				}
			}
  return p;
}
/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t PPPoS_Interface_Init(Modem_HandleTypeDef *modem)
{
  LWIP_ASSERT("netif != NULL", (modem->netif != NULL));
  
#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "gsm";
#endif /* LWIP_NETIF_HOSTNAME */

  modem->netif->name[0] = 'p';
  modem->netif->name[1] = 'p';
//___________________________________GSM Init
		//if (Modem_PPPoS_Status_Get() == PPPOS_STATE_FIRSTINIT){
  /* initialize the hardware */
  pppos_low_level_init(modem);

		//ждем когда модем синициализируется
	while (modem->gsm->state != GSM_STATE_READY) {
		if(modem->gsm->state == GSM_STATE_ERROR){
				BRIDGE_DEBUG_PPPOS(( "Error initializing GSM\r\n"));
				return ERR_RST;		
			}
		osDelay(1);
	}
	
//}
		
//___________________________________PPPOS Init
	if (Modem_PPPoS_Status_Get() == PPPOS_STATE_FIRSTINIT){
		//там создается память и не отчищается, поэтому вызывать ее нужно один раз
		modem->ppp = pppapi_pppos_create(modem->netif, pppos_output_callback, ppp_status_cb, modem);
		if (modem->ppp == NULL) {
				BRIDGE_DEBUG_PPPOS(( "Error initializing PPPoS\r\n"));
			while(1){}
		}
		PPPOS_Input_Task_Start(modem);
	}
		
	ppp_set_default(modem->ppp);
	ppp_set_usepeerdns(modem->ppp, 1); //запросить DNS
	ppp_set_auth(modem->ppp, PPPAUTHTYPE_PAP, modem_pppos.gsm->internet_user, modem_pppos.gsm->internet_password);	
	ppp_connect(modem->ppp, 0);	
	
  return ERR_OK;
}

static void pppos_low_level_init(Modem_HandleTypeDef *modem)
{ 
	GSM_Hardware_Reset_And_Start(modem->gsm, modem->gsm->usart);
}

static err_t pppos_low_level_output(Modem_HandleTypeDef *modem, u8_t *data, u32_t len)
{
  err_t errval;
		GSM_HandleTypeDef *gsm = (GSM_HandleTypeDef *)modem->gsm;

//		if(gsm->usart->hdmatx->Instance->NDTR!=0){
//		while(gsm->usart->hdmatx->Instance->NDTR!=0){}
//			osDelay(1);
//		}
		memcpy( (uint8_t*)gsm->tx_buff, (uint8_t *)data, len);
		
		if (len > 0) {gsm->link_cb->transmit(gsm->usart, (uint8_t *)gsm->tx_buff, len);}
			
	errval = ERR_OK;
  return errval;
}

// === Handle sending data to GSM modem ===
u32_t pppos_output_callback(ppp_pcb *pcb, u8_t *data, u32_t len, void *ctx)
{		
		Modem_HandleTypeDef *modem = (Modem_HandleTypeDef *)ctx;
		if (len > 0) {pppos_low_level_output(modem, data, len);}
    return len;
}


static void ppp_status_cb(ppp_pcb *pcb, int err_code, void *ctx) {
	struct netif *pppif = ppp_netif(pcb);
	LWIP_UNUSED_ARG(ctx);

	switch(err_code) {
		case PPPERR_NONE: {
			#ifdef BRIDGE_DEBUG_PPPOS
			BRIDGE_DEBUG_PPPOS(("PPPoS status_cb: Connected\r\n"));
			#if PPP_IPV4_SUPPORT
			BRIDGE_DEBUG_PPPOS(("   ipaddr    = %s\r\n", ipaddr_ntoa(&pppif->ip_addr)));
			BRIDGE_DEBUG_PPPOS(("   gateway   = %s\r\n", ipaddr_ntoa(&pppif->gw)));
			BRIDGE_DEBUG_PPPOS(("   netmask   = %s\r\n", ipaddr_ntoa(&pppif->netmask)));
			#endif
			#if LWIP_DNS
			ip_addr_t *ns;
      ns = (ip_addr_t *)dns_getserver(0);
      BRIDGE_DEBUG_PPPOS(("   dns1        = %s\n", ipaddr_ntoa(ns)));
      ns = (ip_addr_t *)dns_getserver(1);
      BRIDGE_DEBUG_PPPOS(("   dns2        = %s\n", ipaddr_ntoa(ns)));
			#endif /* LWIP_DNS */
			#if PPP_IPV6_SUPPORT
			printf("   ip6addr   = %s\r\n", ip6addr_ntoa(netif_ip6_addr(pppif, 0)));
			#endif
			#endif
			Modem_PPPoS_Error_Set(PPPOS_ERROR_OK);
			Modem_PPPoS_Status_Set(PPPOS_STATE_CONNECTED);
			break;
		}
		case PPPERR_PARAM: {
			BRIDGE_DEBUG_PPPOS(("\r\nPPPoS status_cb: Invalid parameter\r\n"));
			break;
		}
		case PPPERR_OPEN: {
			BRIDGE_DEBUG_PPPOS(("\r\nPPPoS status_cb: Unable to open PPP session\r\n"));
			break;
		}
		case PPPERR_DEVICE: {
			BRIDGE_DEBUG_PPPOS(("\r\nPPPoS status_cb: Invalid I/O device for PPP\r\n"));
			break;
		}
		case PPPERR_ALLOC: {
			BRIDGE_DEBUG_PPPOS(("\r\nPPPoS status_cb: Unable to allocate resources\r\n"));
			break;
		}
		case PPPERR_USER: {
			/* ppp_free(); -- can be called here */
			BRIDGE_DEBUG_PPPOS(("\r\nPPPoS status_cb: User interrupt (disconnected)\r\n"));
			Modem_PPPoS_Status_Set(PPPOS_STATE_DISCONNECTED);
			break;
		}
		case PPPERR_CONNECT: {
			BRIDGE_DEBUG_PPPOS(("\r\nPPPoS status_cb: Connection lost\r\n"));
			Modem_PPPoS_Error_Set(PPPOS_ERROR_CONNECTION_LOST);
			Modem_PPPoS_Status_Set(PPPOS_STATE_DISCONNECTED);
			break;
		}
		case PPPERR_AUTHFAIL: {
			BRIDGE_DEBUG_PPPOS(("\r\nPPPoS status_cb: Failed authentication challenge\r\n"));
			break;
		}
		case PPPERR_PROTOCOL: {
			BRIDGE_DEBUG_PPPOS(("\r\nPPPoS status_cb: Failed to meet protocol\r\n"));
			break;
		}
		case PPPERR_PEERDEAD: {
			BRIDGE_DEBUG_PPPOS(("\r\nPPPoS status_cb: Connection timeout\r\n"));
			break;
		}
		case PPPERR_IDLETIMEOUT: {
			BRIDGE_DEBUG_PPPOS(("\r\nPPPoS status_cb: Idle Timeout\r\n"));
			break;
		}
		case PPPERR_CONNECTTIME: {
			BRIDGE_DEBUG_PPPOS(("\r\nPPPoS status_cb: Max connect time reached\r\n"));
			break;
		}
		case PPPERR_LOOPBACK: {
			BRIDGE_DEBUG_PPPOS(("\r\nPPPoS status_cb: Loopback detected\r\n"));
			break;
		}
		default: {
			BRIDGE_DEBUG_PPPOS(("\r\nPPPoS status_cb: Unknown error code %d\r\n", err_code));
			break;
		}
	}
/*
 * This should be in the switch case, this is put outside of the switch
 * case for example readability.
 */

  if (err_code == PPPERR_NONE) {
    return;
  }

  /* ppp_close() was previously called, don't reconnect */
  if (err_code == PPPERR_USER) {
    /* ppp_free(); -- can be called here */
    return;
  }
  /*
   * Try to reconnect in 30 seconds, if you need a modem chatscript you have
   * to do a much better signaling here ;-)
   */
//	ppposif_init(&modem_pppos);
//	//ждем когда модем синициализируется
//	while (modem_pppos.gsm->state != GSM_STATE_READY) {
//		osDelay(1);
//	}
//	pcb->phase = PPP_PHASE_DEAD;
//	ppp_close(pcb, 0);
//  ppp_connect(pcb, 3);
  /* OR ppp_listen(pcb); */
}

//void Change_IP(struct netif *netif, char* addr) 
//{
//  ip_addr_t ipaddr;	
//	ipaddr.addr = inet_addr(addr);
//	netif_set_ipaddr(netif, &ipaddr);
//}

PPPOS_StateTypeDef Modem_PPPoS_Status_Get(void)
{
	xSemaphoreTake(PPPOS_Mutex, PPPOS_MUTEX_TIMEOUT);
	PPPOS_StateTypeDef stat = modem_pppos.pppos_handle->state;
	xSemaphoreGive(PPPOS_Mutex);
	return stat;
}

void Modem_PPPoS_Status_Set(PPPOS_StateTypeDef stat)
{
	xSemaphoreTake(PPPOS_Mutex, PPPOS_MUTEX_TIMEOUT);
	modem_pppos.pppos_handle->state=stat;
	xSemaphoreGive(PPPOS_Mutex);
}

PPPOS_ErrorTypeDef Modem_PPPoS_Error_Get(void)
{
	xSemaphoreTake(PPPOS_Mutex, PPPOS_MUTEX_TIMEOUT);
	PPPOS_ErrorTypeDef err = modem_pppos.pppos_handle->error;
	xSemaphoreGive(PPPOS_Mutex);
	return err;
}

void Modem_PPPoS_Error_Set(PPPOS_ErrorTypeDef err)
{
	xSemaphoreTake(PPPOS_Mutex, PPPOS_MUTEX_TIMEOUT);
	modem_pppos.pppos_handle->error = err;
	xSemaphoreGive(PPPOS_Mutex);
}

/*
 * 
 * 
 */
//-----------------------------
void PPPOS_Input_Task_Start(Modem_HandleTypeDef *modem)
{
	modem->gsm->link_cb->rx_dma_reset(modem->gsm->usart, modem->gsm->rx_buff, modem->gsm->rx_buff_size);
	modem->gsm->link_cb->receive(modem->gsm->usart, modem->gsm->rx_buff, modem->gsm->rx_buff_size);
	osThreadDef(PPPInputTask, &PPPOS_Input_Task, PPPOS_INPUT_THREAD_PRIO, 0, PPPOS_INPUT_STACK_SIZE);
	osThreadCreate(osThread(PPPInputTask), modem);
}

void PPPOS_Test(void)
{
	//modem_pppos.ppp->phase = PPP_PHASE_DEAD;
	ppp_status_cb(modem_pppos.ppp,PPPERR_CONNECT,&modem_pppos);
}

void PPPOS_LOG_Phase(ppp_pcb *pcb)
{
	switch(pcb->phase){
		case PPP_PHASE_DEAD: 					BRIDGE_DEBUG_PPPOS(("PPPoS: pcb->phase == PPP_PHASE_DEAD\r\n")); break;
		case PPP_PHASE_MASTER: 				BRIDGE_DEBUG_PPPOS(("PPPoS: pcb->phase == PPP_PHASE_MASTER\r\n")); break;
		case PPP_PHASE_HOLDOFF: 			BRIDGE_DEBUG_PPPOS(("PPPoS: pcb->phase == PPP_PHASE_HOLDOFF\r\n")); break;
		case PPP_PHASE_INITIALIZE: 		BRIDGE_DEBUG_PPPOS(("PPPoS: pcb->phase == PPP_PHASE_INITIALIZE\r\n")); break;
		case PPP_PHASE_SERIALCONN: 		BRIDGE_DEBUG_PPPOS(("PPPoS: pcb->phase == PPP_PHASE_SERIALCONN\r\n")); break;
		case PPP_PHASE_DORMANT: 			BRIDGE_DEBUG_PPPOS(("PPPoS: pcb->phase == PPP_PHASE_DORMANT\r\n")); break;
		case PPP_PHASE_ESTABLISH: 		BRIDGE_DEBUG_PPPOS(("PPPoS: pcb->phase == PPP_PHASE_ESTABLISH\r\n")); break;
		case PPP_PHASE_AUTHENTICATE: 	BRIDGE_DEBUG_PPPOS(("PPPoS: pcb->phase == PPP_PHASE_AUTHENTICATE\r\n")); break;
		case PPP_PHASE_CALLBACK: 			BRIDGE_DEBUG_PPPOS(("PPPoS: pcb->phase == PPP_PHASE_CALLBACK\r\n")); break;
		case PPP_PHASE_NETWORK: 			BRIDGE_DEBUG_PPPOS(("PPPoS: pcb->phase == PPP_PHASE_NETWORK\r\n")); break;
		case PPP_PHASE_RUNNING: 			BRIDGE_DEBUG_PPPOS(("PPPoS: pcb->phase == PPP_PHASE_RUNNING\r\n")); break;
		case PPP_PHASE_TERMINATE: 		BRIDGE_DEBUG_PPPOS(("PPPoS: pcb->phase == PPP_PHASE_TERMINATE\r\n")); break;
		case PPP_PHASE_DISCONNECT: 		BRIDGE_DEBUG_PPPOS(("PPPoS: pcb->phase == PPP_PHASE_DISCONNECT\r\n")); break;
	}
	
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
