

#include <string.h>
#include <stdbool.h>
#include "main.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"

#include "netif/ppp/pppos.h"
#include "netif/ppp/ppp.h"
#include "netif/ppp/pppapi.h"
#include "netif/ppp/ppp_impl.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "lwip/dhcp.h"
#include "lwip/api.h"
#include "lwip/netdb.h"

#include "Bridge_Data.h"
#include "GSM.h"
#include "Application_PPPoS.h"
#include "PPPoS_Interface.h"
#include "PPPoS_HTTP_Client.h"

//osThreadId PPPTaskHandle;
//extern UART_HandleTypeDef huart3;


//static uint8_t gsm_status = PPPOS_STATE_FIRSTINIT;
//const char *PPP_User = CONFIG_GSM_INTERNET_USER;
//const char *PPP_Pass = CONFIG_GSM_INTERNET_PASSWORD;





int GSM_PPPoS_Flag_Get(uint8_t *fl)
{
	xSemaphoreTake(PPPOS_Mutex, PPPOS_MUTEX_TIMEOUT);
	uint8_t gstat = *fl;
	xSemaphoreGive(PPPOS_Mutex);
	return gstat;
}

void GSM_PPPoS_Flag_Set(uint8_t *fl, uint8_t gstat)
{
	xSemaphoreTake(PPPOS_Mutex, PPPOS_MUTEX_TIMEOUT);
	*fl=gstat;
	xSemaphoreGive(PPPOS_Mutex);
}



/*
 * PPPoS TASK
 * Handles GSM initialization, disconnects and GSM modem responses
 */
//-----------------------------
void pppos_client_task(void const * argument)
{
	Modem_HandleTypeDef *modem = (Modem_HandleTypeDef *) argument;

	PPPoS_Interface_Init(modem);
//	//ждем когда модем синициализируется
//	while (modem->gsm->state != GSM_STATE_READY) {
//		if(modem->gsm->state == GSM_STATE_ERROR){				
//				goto exit_error;
//			}
//		osDelay(1);
//	}

//	if (Modem_PPPoS_Status_Get() == PPPOS_STATE_FIRSTINIT){
//		//там создается память и не отчищается, поэтому вызывать ее нужно один раз???
//		modem->ppp = pppapi_pppos_create(modem->netif, pppos_output_callback, ppp_status_cb, modem);
//		if (modem->ppp == NULL) {
//				BRIDGE_DEBUG_PPPOS(( "Error initializing PPPoS\r\n"));
//			while(1){}
//		}
//		PPPOS_Input_Task_Start(modem);
//	}
//		
//	ppp_set_default(modem->ppp);
//	ppp_set_usepeerdns(modem->ppp, 1); //запросить DNS
//	ppp_set_auth(modem->ppp, PPPAUTHTYPE_PAP, modem_pppos.gsm->internet_user, modem_pppos.gsm->internet_password);	
//	ppp_connect(modem->ppp, 0);	

	while (Modem_PPPoS_Status_Get() != PPPOS_STATE_CONNECTED) {	
	if(Modem_PPPoS_Status_Get() == PPPOS_STATE_DISCONNECTED){
		goto exit_error;
	}		
		osDelay(1);
	}
	
	fl_end_gsm_connect = 1;
	User_notification(modem->netif);
	netif_set_default(modem->netif);	
//	Change_IP(&ppposnetif, "10.95.198.2");
	PPPoS_HTTP_Client(SITE_TEST_IP_1);
//	GSM_PPPoS_Flag_Set(&fl_forward_set,1); //Start Read PPPOS buffer
	osDelay(1500);
	fl_send_forward_packet = 1;
	for( ;; )
  {
    /* Delete the Init Thread */ 
    osThreadTerminate(NULL);
  }
	
exit_error:
	//if (modem->ppp) ppp_free(modem->ppp);
	Modem_PPPoS_Status_Set(PPPOS_STATE_ERROR);
	BRIDGE_DEBUG_PPPOS(( "PPPoS TASK TERMINATED\r\n"));
	for( ;; )
  {
    /* Delete the Init Thread */ 
    osThreadTerminate(NULL);
  }
}

void PPPOS_Start(GSM_HandleTypeDef *gsm, UART_HandleTypeDef *huart)
{
	osMutexDef(PPPOS_MUTEX);  
  PPPOS_Mutex = osMutexCreate(osMutex(PPPOS_MUTEX));
	
	GSM_Config_Reset_State(gsm, huart);
	modem_pppos.gsm = gsm;
	modem_pppos.pppos_handle = &hpppos;
	
	modem_pppos.pppos_handle->rx_buff = gsm->rx_buff;
	modem_pppos.pppos_handle->rx_buff_size = gsm->rx_buff_size;	
	modem_pppos.pppos_handle->tx_buff = gsm->tx_buff;
	modem_pppos.pppos_handle->tx_buff_size = gsm->tx_buff_size;

	modem_pppos.gsm->usart = huart;
	modem_pppos.netif = &pppos_netif;
	//modem_pppos.ppp = pppos;
	modem_pppos.pc_addr = temp_pc_addr;

	
	Modem_PPPoS_Error_Set(PPPOS_ERROR_OK);
	Modem_PPPoS_Status_Set(PPPOS_STATE_FIRSTINIT);
	osThreadDef(PPPTask, &pppos_client_task, PPPOS_THREAD_PRIO, 0, PPPOS_STACK_SIZE);
	osThreadCreate(osThread(PPPTask), &modem_pppos);
}

/**
  * @brief  Notify the User about the nework interface config status 
  * @param  netif: the network interface
  * @retval None
  */
void User_notification(struct netif *netif) 
{
  if (netif_is_up(netif))
 {
#ifdef USE_DHCP
    /* Update DHCP state machine */
    DHCP_state = DHCP_START;
#else
		BRIDGE_DEBUG_PPPOS(("\r\nStatic IP address: %s\r\n", ip4addr_ntoa((const ip4_addr_t *)&netif->ip_addr)));
		BRIDGE_DEBUG_PPPOS(("              NETMASK: %s\r\n", ip4addr_ntoa((const ip4_addr_t *)&netif->netmask)));
		BRIDGE_DEBUG_PPPOS(("              GATEWAY: %s\r\n", ip4addr_ntoa((const ip4_addr_t *)&netif->gw)));
#endif /* USE_DHCP */
  }
  else
  {  
#ifdef USE_DHCP
    /* Update DHCP state machine */
    DHCP_state = DHCP_LINK_DOWN;
#endif  /* USE_DHCP */
    BRIDGE_DEBUG_PPPOS(("The network cable is not connected \n"));
  } 
}








