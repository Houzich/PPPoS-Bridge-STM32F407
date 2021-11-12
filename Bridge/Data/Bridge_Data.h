
#ifndef __GRIDGE_DATA_H__
#define __GRIDGE_DATA_H__
#include <stdint.h>
#include "cmsis_os.h"
#include "lwip/ip_addr.h"
#include "Bridge_Define.h"


#ifdef BRIDGE_USE_GSM
extern uint8_t Buff_GSM_Rx[BUF_RX_SIZE_GSM];
extern uint8_t Buff_GSM_Tx[BUF_TX_SIZE_GSM];
#endif

#ifdef BRIDGE_USE_PPPOS
extern QueueHandle_t PPPOS_Mutex;

//----Времяночка
extern uint8_t fl_start_read_ppp;
//----------------
#endif


extern uint32_t 		fl_send_forward_packet;
extern ip_addr_t  	temp_pc_addr;


//Тестовые
extern char temp_internet_user[100];
extern char temp_internet_password[100];
extern char temp_internet_apn[100];
extern uint32_t fl_start_gsm_connect;
extern uint32_t fl_end_gsm_connect;
extern char for_test_rx_data[2048];
extern char *string_site_req;
#endif /* __GRIDGE_DATA_H__ */
