
#include <string.h>
#include "stm32f4xx_hal.h"
#include "stdbool.h"
#include "cmsis_os.h"
#include "Bridge_Data.h"
#include "SIM800_USART.h"

#ifdef BRIDGE_USE_GSM
static void SIM800_Transmit(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);
static void SIM800_Receive(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);
static int 	SIM800_Wait_Receive_Data(UART_HandleTypeDef *huart, uint16_t size, int timeout_before_response, int timeout_between_packet);
static int 	SIM800_Recieve_Timeout(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t size, int timeout_before_response, int timeout_between_packet);
static void SIM800_Rx_DMA_Reset(UART_HandleTypeDef *huart, uint8_t *buff, uint32_t size);
static int 	SIM800_Get_Rx_Count(UART_HandleTypeDef *huart, uint16_t Size);

/* Callbacks structure for GSM core */
static const struct link_sim800_callbacks sim800_callbacks = {
	SIM800_Transmit,
	SIM800_Receive,
	SIM800_Wait_Receive_Data,
	SIM800_Recieve_Timeout,
	SIM800_Rx_DMA_Reset,
	SIM800_Get_Rx_Count
};

/*###############################################################*/
/*###############################################################* SIM800_Transmit -->*/
/*###############################################################*/
static void SIM800_Transmit(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
{
	HAL_StatusTypeDef status;

	while(huart->gState != HAL_UART_STATE_READY)
	{
		osDelay(1);
	}	
	
	status=HAL_UART_Transmit_DMA(huart, pData, Size);
	#ifdef BRIDGE_DEBUG_GSM
	if (status==HAL_ERROR)
	{
		BRIDGE_DEBUG_GSM(("\r\nUSART Transmit HAL_ERROR\r\n"));
	}
	else if (status==HAL_BUSY)
	{
		BRIDGE_DEBUG_GSM(("\r\nUSART Transmit HAL_BUSY\r\n"));
	}
	else if (status==HAL_TIMEOUT)
	{
		BRIDGE_DEBUG_GSM(("\r\nUSART Transmit HAL_TIMEOUT\r\n"));
	}
	#endif
}

/*###############################################################*/
/*###############################################################* SIM800_Receive -->*/
/*###############################################################*/
static void SIM800_Receive(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
{
	HAL_StatusTypeDef status;
	
	while(huart->RxState != HAL_UART_STATE_READY)
	{
		osDelay(1);
	}	
	
	status=HAL_UART_Receive_DMA(huart, pData, Size);
	#ifdef BRIDGE_DEBUG_GSM
	if (status==HAL_ERROR)
	{
		BRIDGE_DEBUG_GSM(("\r\nUSART Receive HAL_ERROR\r\n"));
	}
	else if (status==HAL_BUSY)
	{
		BRIDGE_DEBUG_GSM(("\r\nUSART Receive HAL_BUSY\r\n"));
	}
	#endif
}

/*###############################################################*/
/*###############################################################* SIM800_Wait_Receive_Data -->*/
/*###############################################################*/
static int SIM800_Wait_Receive_Data(UART_HandleTypeDef *huart, uint16_t size, int timeout_before_response, int timeout_between_packet)
{
		int len=0;
		int len_last=0;
		int tmt_pct=timeout_between_packet;
	
	while(len<2 && timeout_before_response>0) //len<2 - мало ли помеху какую споймали
		{			
			osDelay(1);
			len=size - huart->hdmarx->Instance->NDTR;
			timeout_before_response--;	
		}	
		if ((len==0)||(timeout_before_response==0))goto exit;

		while(tmt_pct>0)
		{			
			osDelay(1);
			len=size - huart->hdmarx->Instance->NDTR; 
			if(len==len_last)tmt_pct--;
			if(len>len_last)tmt_pct = timeout_between_packet;	
			if(len==size)goto exit;
			len_last=len;						
		}
exit:		
	huart->RxState = HAL_UART_STATE_READY;
	return len;
}

/*###############################################################*/
/*###############################################################* SIM800_Recieve_Timeout -->*/
/*###############################################################*/
static int SIM800_Recieve_Timeout(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t size, int timeout_before_response, int timeout_between_packet)
{
	SIM800_Receive(huart, pData, size);
	return SIM800_Wait_Receive_Data(huart, size, timeout_before_response, timeout_between_packet);
}

/*###############################################################*/
/*###############################################################* SIM800_Rx_DMA_Reset -->*/
/*###############################################################*/
static void SIM800_Rx_DMA_Reset(UART_HandleTypeDef *huart, uint8_t *buff, uint32_t size)
{
	__HAL_DMA_DISABLE(huart->hdmarx);/* Disable the peripheral */
    /* Clear all flags */
    __HAL_DMA_CLEAR_FLAG (huart->hdmarx, __HAL_DMA_GET_TC_FLAG_INDEX(huart->hdmarx));
    __HAL_DMA_CLEAR_FLAG (huart->hdmarx, __HAL_DMA_GET_HT_FLAG_INDEX(huart->hdmarx));
    __HAL_DMA_CLEAR_FLAG (huart->hdmarx, __HAL_DMA_GET_TE_FLAG_INDEX(huart->hdmarx));
    __HAL_DMA_CLEAR_FLAG (huart->hdmarx, __HAL_DMA_GET_DME_FLAG_INDEX(huart->hdmarx));
    __HAL_DMA_CLEAR_FLAG (huart->hdmarx, __HAL_DMA_GET_FE_FLAG_INDEX(huart->hdmarx));
	huart->hdmarx->Instance->NDTR = size; /* Configure DMA Stream data length */
  huart->hdmarx->Instance->M0AR = (uint32_t)buff;	/* Configure DMA Stream destination address */
	memset(buff, 0, size);
	__HAL_UART_CLEAR_OREFLAG(huart);
	__HAL_DMA_ENABLE(huart->hdmarx);/* Enable the Peripheral */	
}

/*###############################################################*/
/*###############################################################* SIM800_Get_Rx_Count -->*/
/*###############################################################*/
static int SIM800_Get_Rx_Count(UART_HandleTypeDef *huart, uint16_t Size)
{
	return Size - huart->hdmarx->Instance->NDTR;
}
/*###############################################################*/
/*###############################################################* link_sim800_callbacks -->*/
/*###############################################################*/
const struct link_sim800_callbacks* SIM800_Get_Link_Callbacks(void)
{
	return &sim800_callbacks;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){}
void HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart){}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
//	while(1){
//		printf("FULL PPPOS BUFFER");
//		osDelay(1);
//	}
}
void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart){}
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
		//Error_Handler();
}
void HAL_UART_AbortCpltCallback (UART_HandleTypeDef *huart){Error_Handler();}
void HAL_UART_AbortTransmitCpltCallback (UART_HandleTypeDef *huart){Error_Handler();}
void HAL_UART_AbortReceiveCpltCallback (UART_HandleTypeDef *huart){Error_Handler();}



#endif /* BRIDGE_USE_GSM */
