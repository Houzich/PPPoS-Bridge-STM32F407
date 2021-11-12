
#ifndef _SIM800_USART_H_
#define _SIM800_USART_H_
/*
 * The following struct gives the addresses of procedures to call
 * for a particular lower link level protocol.
 */
struct link_sim800_callbacks {
void 	(*transmit)(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);
void 	(*receive)(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);
int 	(*wait_receive_data)(UART_HandleTypeDef *huart, uint16_t size, int timeout_before_response, int timeout_between_packet);
int 	(*receive_timeout)(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t size, int timeout_before_response, int timeout_between_packet);
void 	(*rx_dma_reset)(UART_HandleTypeDef *huart, uint8_t *buff, uint32_t size);
int 	(*get_rx_count)(UART_HandleTypeDef *huart, uint16_t Size);
};
const struct link_sim800_callbacks* SIM800_Get_Link_Callbacks(void);

#endif /*_SIM800_USART_H_*/
