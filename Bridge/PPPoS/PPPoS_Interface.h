#ifndef __PPPOSIF_H__
#define __PPPOSIF_H__


#include "lwip/err.h"
#include "lwip/netif.h"
#include "netif/ppp/pppos.h"
#include "netif/ppp/ppp.h"
#include "netif/ppp/pppapi.h"
#include "netif/ppp/ppp_impl.h"
#include "Application_PPPoS.h"

/**
  * @brief  Definitions for error constants.
  */
typedef enum {
  PPPOS_ERROR_OK = 0x00U,          /** No error, everything OK. */
  PPPOS_ERROR_INIT_FAIL = 0x01U,   /** 	*/
	PPPOS_ERROR_INIT_MEMORY= 0x02U,  /** 	*/
	PPPOS_ERROR_CONNECTION_LOST= 0x03U,  /** 	*/
} PPPOS_ErrorTypeDef;

/**
  * @brief  PPPOS State structure definition
  */
typedef enum {
  PPPOS_STATE_FIRSTINIT = 0x00U,    /*!< Peripheral not Initialized                         */
  PPPOS_STATE_BUSY_RESET = 0x01U,   /*!< Reset process is ongoing                           */
  PPPOS_STATE_PERIPH_READY = 0x02U, /*!< Peripheral Initialized and ready for use           */
  PPPOS_STATE_READY = 0x03U,        /*!< PPPOS Initialized               											*/
  PPPOS_STATE_BUSY_INIT = 0x04U,    /*!< Initialization process is ongoing                  */
  PPPOS_STATE_ERROR = 0x06U,        /*!< PPPOS error state                                    */
  PPPOS_STATE_ABORT = 0x07U,        /*!< PPPOS abort is ongoing                               */
  PPPOS_STATE_DISCONNECTED = 0x08U, /*!< 	                               */
  PPPOS_STATE_CONNECTED = 0x09U,    /*!< 	                               */
  PPPOS_STATE_IDLE = 0x10U,         /*!< 	                               */
	PPPOS_STATE_RECONNECT = 0x11U,         /*!< 	                               */
} PPPOS_StateTypeDef;


/** 
  * @brief  PPPOS handle Structure definition  
  */
typedef struct __PPPOS_HandleTypeDef
{
  __IO PPPOS_StateTypeDef state;   							/*!< PPPOS state            					        */
  __IO PPPOS_ErrorTypeDef error; 	 							/*!< PPPOS Error code                         */
  UART_HandleTypeDef *usart;     								/*!< PPPOS USART Handle parameters            */
  uint8_t *tx_buff;           	 								/*!< Pointer to PPPOS Tx transfer Buffer      */
  uint16_t tx_buff_size;         								/*!< PPPOS Tx Transfer size                   */
  uint8_t *rx_buff;           	 								/*!< Pointer to PPPOS Rx transfer Buffer      */
  uint16_t rx_buff_size;         								/*!< PPPOS Rx Transfer size                   */
} PPPOS_HandleTypeDef;

/** 
  * @brief  Modem handle Structure definition  
  */ 
typedef struct __Modem_HandleTypeDef
{   
	PPPOS_HandleTypeDef									*pppos_handle;	
	GSM_HandleTypeDef										*gsm;
	struct netif												*netif;
	ppp_pcb 														*ppp;
	ip_addr_t  													pc_addr;
} Modem_HandleTypeDef;  

extern PPPOS_HandleTypeDef	hpppos;
extern Modem_HandleTypeDef modem_pppos;
extern struct netif pppos_netif;						// The PPP IP interface
//extern ppp_pcb *pppos;				// The PPP control block

void PPPoS_Input_Task(void const * argument);
err_t PPPoS_Interface_Init(Modem_HandleTypeDef *modem);
void PPPoS_Interface_Input(Modem_HandleTypeDef *modem);
void PPPOS_Input_Task_Start(Modem_HandleTypeDef *modem);

u32_t pppos_output_callback(ppp_pcb *pcb, u8_t *data, u32_t len, void *ctx);

PPPOS_StateTypeDef Modem_PPPoS_Status_Get(void);
void Modem_PPPoS_Status_Set(PPPOS_StateTypeDef stat);
PPPOS_ErrorTypeDef Modem_PPPoS_Error_Get(void);
void Modem_PPPoS_Error_Set(PPPOS_ErrorTypeDef stat);

void PPPOS_Test(void);
void PPPOS_LOG_Phase(ppp_pcb *pcb);
#endif
