
#include "Bridge_Data.h"


/*<!-- ********************************************************** GSM -->*/
#ifdef BRIDGE_USE_GSM
uint8_t Buff_GSM_Rx[];
uint8_t Buff_GSM_Tx[];
#endif

#ifdef BRIDGE_USE_PPPOS
QueueHandle_t PPPOS_Mutex = NULL;
//----Времяночка
uint8_t fl_start_read_ppp=0;
//----------------
#endif

//Дляфорварда
uint32_t fl_send_forward_packet=0;

//Тестовые
char temp_internet_user[];
char temp_internet_password[];
char temp_internet_apn[];
uint32_t fl_start_gsm_connect=0;
uint32_t fl_end_gsm_connect=0;
char for_test_rx_data[];
ip_addr_t  	temp_pc_addr;

