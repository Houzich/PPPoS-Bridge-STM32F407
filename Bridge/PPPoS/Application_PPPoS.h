/*
 *  Author: LoBo (loboris@gmail.com, loboris.github)
 *
*/


#ifndef _APPLICATION_PPPOS_H_
#define _APPLICATION_PPPOS_H_

#include <time.h>
#include "GSM.h"
#include "PPPoS_Interface.h"
#include "netif/ppp/ppp.h"


void PPPOS_Start(GSM_HandleTypeDef *gsm, UART_HandleTypeDef *huart);
void pppos_client_task(void const * argument);
void User_notification(struct netif *netif) ;
int GSM_PPPoS_Flag_Get(uint8_t *fl);
void GSM_PPPoS_Flag_Set(uint8_t *fl, uint8_t gstat);
void User_notification(struct netif *netif);

#endif /*_APPLICATION_PPPOS_H_ */
