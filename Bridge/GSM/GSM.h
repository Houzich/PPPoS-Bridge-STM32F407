#ifndef __GSM_H
#define __GSM_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "SIM800_USART.h"



/**
  * @brief  Definitions for error constants.
  */
typedef enum {
  GSM_ERROR_OK = 0x00U,          /** No error, everything OK. */
  GSM_ERROR_INIT_FAIL = 0x01U,   /** 	*/
	GSM_ERROR_INIT_MEMORY= 0x02U,  /** 	*/
} GSM_ErrorTypeDef;

//#define GSM_ERR_IS_FATAL(e) ((e) <= GSM_ErrorTypeDef.ERR_ABRT)

/**
  * @brief  Definitions for state constants.
  */
typedef enum {
  GSM_STATE_RESET = 0x00U,        /*!<                         */
  GSM_STATE_BUSY_RESET = 0x01U,   /*!< Reset process is ongoing                           */
	GSM_STATE_AFTER_RESET = 0x02U,   /*!< Reset process is ongoing                           */
//  GSM_STATE_PERIPH_READY = 0x02U, /*!< Peripheral Initialized and ready for use           */
  GSM_STATE_READY = 0x03U,        /*!< GSM Initialized               											*/
  GSM_STATE_BUSY_INIT = 0x04U,    /*!< Initialization process is ongoing                  */
  GSM_STATE_ERROR = 0x06U,        /*!< GSM error state                                    */
//  GSM_STATE_ABORT = 0x07U,        /*!< GSM abort is ongoing                               */
//  GSM_STATE_DISCONNECTED = 0x08U, /*!< 	                               */
//  GSM_STATE_CONNECTED = 0x09U,    /*!< 	                               */
//  GSM_STATE_IDLE = 0x19U,         /*!< 	                               */
} GSM_StateTypeDef;

/** 
  * @brief  command  
  */
typedef struct
{
  char 		*command;
  char 		*response_as_ok;
  uint16_t timeout_before_response;
  uint16_t timeout_between_packet;
  uint16_t delay_after_response;
} GSM_CommandTypeDef;

/** 
  * @brief  command + flag skip  
  */
typedef struct
{
  const GSM_CommandTypeDef 		*at;
  bool  						 			   skip;
} GSM_HandleCommandTypeDef;		

/** 
  * @brief  GSM session 
  */
typedef const struct {
    GSM_HandleCommandTypeDef *(*coommands)[]; //указатель на массив указателей на структуры GSM_HandleCommandTypeDef
    uint32_t cmds_num;
} GSM_SessionTypeDef;

/** 
  * @brief  GSM handle Structure definition  
  */
typedef struct __GSM_HandleTypeDef
{
  __IO GSM_StateTypeDef state;   								/*!< GSM state            					        */
  __IO GSM_ErrorTypeDef error; 	 								/*!< GSM Error code                         */
  UART_HandleTypeDef *usart;     								/*!< GSM USART Handle parameters            */
  uint8_t *tx_buff;           	 								/*!< Pointer to GSM Tx transfer Buffer      */
  uint16_t tx_buff_size;         								/*!< GSM Tx Transfer size                   */
  uint8_t *rx_buff;           	 								/*!< Pointer to GSM Rx transfer Buffer      */
  uint16_t rx_buff_size;         								/*!< GSM Rx Transfer size                   */
  const char *internet_user;     								/*!<                      									*/
  const char *internet_password; 								/*!<                      									*/
  const char *internet_apn;      								/*!<                      									*/
  const struct link_sim800_callbacks *link_cb;	/*!< Links callbacks functions USART        */
	GSM_SessionTypeDef *curr_session;							/*!< Current session												*/
	GSM_HandleCommandTypeDef *curr_command;				/*!< Current command												*/
} GSM_HandleTypeDef;

extern GSM_HandleTypeDef hgsm;

#define GSM_OK_Str "OK"


void GSM_Init(void const *argument);
void GSM_Config_Reset_State(GSM_HandleTypeDef *gsm, UART_HandleTypeDef *huart);
void GSM_Hardware_Reset_And_Start(GSM_HandleTypeDef *gsm, UART_HandleTypeDef *huart);
#endif /* __GSM_H */
