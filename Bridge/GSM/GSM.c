
#include "cmsis_os.h"
#include <string.h>
#include <stdlib.h>

#include "Bridge_Data.h"
#include "GSM.h"
#include "SIM800_USART.h"

#ifdef BRIDGE_USE_GSM
static void GSM_Hardware_Reset(GSM_HandleTypeDef *gsm);
GSM_HandleTypeDef hgsm;
osThreadId gsmTaskHandle = NULL;

#define GSM_SET_STATE(stat)\
      gsm->state = stat;

#define GSM_SET_STATE_ERROR(err)\
      gsm->state = GSM_STATE_ERROR;\
      gsm->error = err;
/*1*/
static const GSM_CommandTypeDef GSMCommandAT =
    {
        .command = "AT\r",
        .response_as_ok = GSM_OK_Str,
        .timeout_before_response = 3000,
        .timeout_between_packet = 500,
        .delay_after_response = 100,
};
/*1*/
static const GSM_CommandTypeDef GSMCommandBod115200 =
    {
        .command = "AT+IPR=115200\r",
        .response_as_ok = GSM_OK_Str,
        .timeout_before_response = 3000,
        .timeout_between_packet = 500,
        .delay_after_response = 100,
};
/*1*/
static const GSM_CommandTypeDef GSMCommandReset =
    {
        .command = "ATZ\r",
        .response_as_ok = GSM_OK_Str,
        .timeout_before_response = 3000,
        .timeout_between_packet = 500,
        .delay_after_response = 100,
};
/*1*/
static const GSM_CommandTypeDef GSMCommandNoSMSInd =
    {
        .command = "AT+CNMI=0,0,0,0,0\r",
        .response_as_ok = GSM_OK_Str,
        .timeout_before_response = 3000,
        .timeout_between_packet = 100,
        .delay_after_response = 100,
};
/*1*/
static const GSM_CommandTypeDef GSMCommandRFOn =
    {
        .command = "AT+CFUN=1\r",
        .response_as_ok = GSM_OK_Str,
        .timeout_before_response = 3000,
        .timeout_between_packet = 500,
        .delay_after_response = 3000,
};
/*1*/
static const GSM_CommandTypeDef GSMCommandEchoOff =
    {
        .command = "ATE0\r",
        .response_as_ok = GSM_OK_Str,
        .timeout_before_response = 3000,
        .timeout_between_packet = 500,
        .delay_after_response = 4000,
};
/*1*/
static const GSM_CommandTypeDef GSMCommandCMGF1 =
    {
        .command = "AT+CMGF=1\r",
        .response_as_ok = GSM_OK_Str,
        .timeout_before_response = 3000,
        .timeout_between_packet = 500,
        .delay_after_response = 100,
};
/*1*/
static const GSM_CommandTypeDef GSMCommandPin =
    {
        .command = "AT+CPIN?\r",
        .response_as_ok = "CPIN: READY",
        .timeout_before_response = 3000,
        .timeout_between_packet = 500,
        .delay_after_response = 100,
};
/*1*/
static const GSM_CommandTypeDef GSMCommandReg =
    {
        .command = "AT+CREG?\r",
        .response_as_ok = "CREG: 0,1",
        .timeout_before_response = 3000,
        .timeout_between_packet = 500,
        .delay_after_response = 100,
};

//эту команду в оперативной памяти размещаем, чтобы можно было менять оператора
//поле command будет заполняться входе инициализации
static GSM_CommandTypeDef GSMCommandAPN =
    {
        .command = NULL,
        .response_as_ok = GSM_OK_Str,
        .timeout_before_response = 3000,
        .timeout_between_packet = 500,
        .delay_after_response = 100,
};
/*1*/
static const GSM_CommandTypeDef GSMCommandCGQMIN =
    {
        .command = "AT+CGQMIN=1,0,0,0,0,0\r",
        .response_as_ok = GSM_OK_Str,
        .timeout_before_response = 3000,
        .timeout_between_packet = 1000,
        .delay_after_response = 100,
};
/*1*/
static const GSM_CommandTypeDef GSMCommandCGQREQ =
    {
        .command = "AT+CGQREQ=1,2,4,3,6,31\r",
        .response_as_ok = GSM_OK_Str,
        .timeout_before_response = 3000,
        .timeout_between_packet = 1000,
        .delay_after_response = 100,
};
/*1*/
static const GSM_CommandTypeDef GSMCommandConnect =
    {
        .command = "ATDT*99***1#\r",
        .response_as_ok = "CONNECT",
        .timeout_before_response = 30000,
        .timeout_between_packet = 500,
        .delay_after_response = 100,
};

//Содержат в себе ссылку на структуру команды во флэш и флаг skip
static GSM_HandleCommandTypeDef GSMHandleCommandAT = {&GSMCommandAT,0};
static GSM_HandleCommandTypeDef GSMHandleCommandBod115200 = {&GSMCommandBod115200,0};
static GSM_HandleCommandTypeDef GSMHandleCommandReset = {&GSMCommandReset,0};
static GSM_HandleCommandTypeDef GSMHandleCommandEchoOff = {&GSMCommandEchoOff,0};
static GSM_HandleCommandTypeDef GSMHandleCommandCMGF1 = {&GSMCommandCMGF1,0};
static GSM_HandleCommandTypeDef GSMHandleCommandRFOn = {&GSMCommandRFOn,0};
static GSM_HandleCommandTypeDef GSMHandleCommandNoSMSInd = {&GSMCommandNoSMSInd,0};
static GSM_HandleCommandTypeDef GSMHandleCommandPin = {&GSMCommandPin,0};
static GSM_HandleCommandTypeDef GSMHandleCommandReg = {&GSMCommandReg,0};
static GSM_HandleCommandTypeDef GSMHandleCommandAPN = {&GSMCommandAPN,0};
static GSM_HandleCommandTypeDef GSMHandleCommandCGQMIN = {&GSMCommandCGQMIN,0};
static GSM_HandleCommandTypeDef GSMHandleCommandCGQREQ = {&GSMCommandCGQREQ,0};
static GSM_HandleCommandTypeDef GSMHandleCommandConnect = {&GSMCommandConnect,0};

/*2*///Набор комманд сессии инициализации GSM для PPPOS 
const GSM_HandleCommandTypeDef *GSMInitCommandsForPPPoS[] =
{
          &GSMHandleCommandAT,
          &GSMHandleCommandBod115200,
          &GSMHandleCommandReset,
          &GSMHandleCommandEchoOff,
          &GSMHandleCommandCMGF1,
          &GSMHandleCommandRFOn,
          &GSMHandleCommandNoSMSInd,
          &GSMHandleCommandPin,
          &GSMHandleCommandReg,
          &GSMHandleCommandAPN,
          &GSMHandleCommandCGQMIN,
          &GSMHandleCommandCGQREQ,
          &GSMHandleCommandConnect,
};

/*2*///Ссылка на список команд и их количество
GSM_SessionTypeDef GSMInitSessionForPPPoS =
{
  .coommands = (GSM_HandleCommandTypeDef *(*)[])&GSMInitCommandsForPPPoS,
  .cmds_num =   (sizeof(GSMInitCommandsForPPPoS) / sizeof(GSM_HandleCommandTypeDef *))
};

/*###############################################################*/
/*###############################################################* GSM_Response_Debug -->*/
/*###############################################################*/
//Используется только при отладке, чтоб строку в одну линию выводить. Убирает символы переноса коретки и т.п.
static void GSM_Response_Debug(GSM_HandleTypeDef *gsm, char *info)
{
    char *cmd = gsm->curr_command->at->command;
    int cmd_size = strlen(cmd);
    uint8_t *p = malloc(cmd_size);
    if (p == NULL)
        return;
    memset(p, 0, cmd_size);

    for (int i = 0; i < cmd_size; i++)
    {
        if ((cmd[i] != 0x00) && ((cmd[i] < 0x20) || (cmd[i] > 0x7F)))
            p[i] = '.';
        else
            p[i] = cmd[i];
        if (p[i] == '\0')
            break;
    }
    printf("%s [%s]", info, p);
    free(p);
}

/*###############################################################*/
/*###############################################################* Send_Cmd_Wait_Response -->*/
/*###############################################################*/
//высылаем команду, ждем ответа. 
static int Send_Cmd_Wait_Response(GSM_HandleTypeDef *gsm)
{
    const GSM_CommandTypeDef *command = gsm->curr_command->at;
    uint8_t *rx_buff = gsm->rx_buff;
    int len = 0;

    gsm->link_cb->rx_dma_reset(gsm->usart, rx_buff, gsm->rx_buff_size);
    gsm->link_cb->receive(gsm->usart, rx_buff, gsm->rx_buff_size);
    // ** Send command to GSM
    if (command->command != NULL)
    {

        BRIDGE_DEBUG_GSM(("\r\nAT COMMAND:%s\r\n", command->command));
			gsm->link_cb->transmit(gsm->usart, (uint8_t *)command->command, strlen(command->command));
    }
    // ** Wait for and check the response

    len = gsm->link_cb->wait_receive_data(gsm->usart, gsm->rx_buff_size, command->timeout_before_response, command->timeout_between_packet);
    if (len > 0)
    {
        for (int i = 0; i < len; i++)
            if ((rx_buff[i] < 0x20) || (rx_buff[i] >= 0x80))
                rx_buff[i] = 0x2e;

        // Check the response
        if (strstr((char *)rx_buff, command->response_as_ok) != NULL)
        {
            BRIDGE_DEBUG_GSM(("AT RESPONSE(%i): [%s]\r\n", len, rx_buff));
            return len;
        }
				else if (len == 1)
        {
            BRIDGE_DEBUG_GSM(("RESPONSE URC: %i\r\n", rx_buff[0]));
            return 1;
        }
        else
        {
            BRIDGE_DEBUG_GSM(("AT BAD RESPONSE(%i): [%s]\r\n", len, rx_buff));
            return 0;
        }
    }
    else
    {
        BRIDGE_DEBUG_GSM(("AT: TIMEOUT\r\n"));
        return 0;
    }
}

/*###############################################################*/
/*###############################################################* Enable_Init_Cmd -->*/
/*###############################################################*/
//для всех команд сбрасываем флаг skip
static void Enable_Init_Cmd(GSM_HandleTypeDef *gsm)
{
    GSM_SessionTypeDef *init = gsm->curr_session;
    for (int idx = 0; idx < init->cmds_num; idx++)
    {
      (*(init->coommands))[2]->skip = 0;
    }
}

/*###############################################################*/
/*###############################################################* Fill_APN_Command_Handle -->*/
/*###############################################################*/
//Заполняе структуру для APN команды
static char * Fill_APN_Command_Handle(GSM_HandleTypeDef *gsm)
{
    GSMCommandAPN.command = "AT+CGDCONT=1,\"IP\",\"%s\"\r"; //вставляем шаблон
    uint32_t str_size = strlen(gsm->internet_apn) + strlen(GSMCommandAPN.command)- sizeof("%s");
    char *p = malloc(str_size);
    if (p == NULL) {
      GSM_SET_STATE_ERROR(GSM_ERROR_INIT_MEMORY)
      return NULL;
    }
    sprintf(p, GSMCommandAPN.command, gsm->internet_apn);
    GSMCommandAPN.command = p;
return p;
}

/*###############################################################*/
/*###############################################################* GSM_Init -->*/
/*###############################################################*/
void GSM_Init(void const *argument)
{
	GSM_HandleTypeDef *gsm = (GSM_HandleTypeDef *)argument; //передаем сюда структуру GSM
/*3*/GSM_HandleCommandTypeDef *(*init_commands)[] = gsm->curr_session->coommands; //передаем список команд текущей сессии инициализации
	uint32_t init_cmds_num = gsm->curr_session->cmds_num; //количество команд в текущей сессии инициализации
	uint32_t cmd_count = 0; //счетчик комманд
	uint32_t fail_number = 0; //количество отказов или попыток выслать команду и получить положительный ответ
	uint32_t err = 0;
	
  GSM_SET_STATE(GSM_STATE_BUSY_INIT) //ставим статус "заняты инициализацией"
	GSM_Hardware_Reset(gsm); //сбрасываем ножкой
	char *cmd_apn = Fill_APN_Command_Handle(gsm); //заполнаяем структуру команды APN, может вернуть ошибку выделения памяти (мало ли, чего не бывает)
  if(cmd_apn==NULL) goto exit;
/*4*/Enable_Init_Cmd(gsm); //сбрасываем флаг skip на всех командах
  BRIDGE_DEBUG_GSM(("GSM initialization start\r\n"));

	// проходимся по всем командам
    while (cmd_count < init_cmds_num) 
    {
			GSM_HandleCommandTypeDef *cmd = (*(init_commands))[cmd_count]; //записывыем сюда структуру текущей команды, для читабельности
        gsm->curr_command = cmd;
			if (cmd->skip) //если уже посылали такую
        {
#ifdef BRIDGE_DEBUG_GSM
					GSM_Response_Debug(gsm, "Skip command:"); //используется для отладки
#endif
            cmd_count++;
            continue;
        }
				
				err = Send_Cmd_Wait_Response(gsm);
        if ((err == 0)||(err == 1))
        {
            // * No response or not as expected, start from first initialization command
            BRIDGE_DEBUG_GSM(("Wrong response, restarting...\r\n"));
            fail_number++;
            if (fail_number > 20){
              GSM_SET_STATE_ERROR(GSM_ERROR_INIT_FAIL)
              goto exit;
            }

            //Если не получили ответа на команду AT, еще раз сбрасываем аппаратно
						//а если URC то не сбрасываем
            if((cmd->at==&GSMCommandAT)&&(err == 0)) GSM_Hardware_Reset(gsm);
            cmd_count = 0;
            continue;
        }

        //Получили положительный ответ на команду Bod115200, меняем скорость усарта
        if(cmd->at==&GSMCommandBod115200)//USART_Change_BaudRate(gsm->usart, 115200);

        if (cmd->at->delay_after_response > 0) //если установили задержку после команды, чтоб модем успел прийти в себя (особенно актуально после команды сброса и еще нескольких) 
            osDelay(cmd->at->delay_after_response);
        cmd->skip = 1;
        // Next command
        cmd_count++;
    }

    BRIDGE_DEBUG_GSM(("GSM initialized.\r\n"));
		GSM_SET_STATE(GSM_STATE_READY)
exit:
    if(cmd_apn != NULL) free(cmd_apn);
    for (;;)
    {
        /* Delete the Init Thread */
        osThreadTerminate(NULL);
    }
}
/*###############################################################*/
/*###############################################################* GSM_Config_Reset_State -->*/
/*###############################################################*/
void GSM_Config_Reset_State(GSM_HandleTypeDef *gsm, UART_HandleTypeDef *huart)
{
    gsm->state = GSM_STATE_RESET;                    											/*!< GSM transfer state                     */
    gsm->error = GSM_ERROR_OK;                       											/*!< GSM Error code                         */
    gsm->usart = huart;                              											/*!< GSM USART Handle parameters            */
    gsm->tx_buff = Buff_GSM_Tx;                      											/*!< Pointer to GSM Tx transfer Buffer      */
    gsm->tx_buff_size = BUF_TX_SIZE_GSM;             											/*!< GSM Tx Transfer size                   */
    gsm->rx_buff = Buff_GSM_Rx;                      											/*!< Pointer to GSM Rx transfer Buffer      */
    gsm->rx_buff_size = BUF_RX_SIZE_GSM;             											/*!< GSM Rx Transfer size                   */
    gsm->internet_user = temp_internet_user;         											/*!< string user                     			  */
    gsm->internet_password = temp_internet_password; 											/*!< string password                        */
    gsm->internet_apn = temp_internet_apn;           											/*!< string apn                             */
    gsm->link_cb = SIM800_Get_Link_Callbacks();      											/*!< Links callbacks functions USART        */
    gsm->curr_session = &GSMInitSessionForPPPoS;              						/*!< Session for use PPP               			*/
    gsm->curr_command = (*(GSMInitSessionForPPPoS.coommands))[0];    			/*!<  First command           		          */

		GSM_HandleCommandTypeDef *(*init_commands)[] = gsm->curr_session->coommands; //передаем список команд текущей сессии инициализации
		uint32_t init_cmds_num = gsm->curr_session->cmds_num; //количество команд в текущей сессии инициализации
		for(int i=0; i<init_cmds_num; i++){
		(*(init_commands))[i]->skip = 0;
		}
		
		for(int i=0; i<BUF_RX_SIZE_GSM; i++){ //отчищаем приемный буфер
			gsm->rx_buff[i] = 0;
		}
}

/*###############################################################*/
/*###############################################################* GSM_Hardware_Reset_And_Start -->*/
/*###############################################################*/
//Делаем аппаратный сброс, вызываем поток для инициализии модема
void GSM_Hardware_Reset_And_Start(GSM_HandleTypeDef *gsm, UART_HandleTypeDef *huart)
{
    BRIDGE_DEBUG_GSM(("\r\nMODUL Start\r\n"));
    GSM_Config_Reset_State(gsm, huart); //аппаратно сбрасываем

    osThreadDef(gsmTask, GSM_Init, GSM_INIT_THREAD_PRIO, 0, GSM_INIT_THREAD_STACKSIZE);
    gsmTaskHandle = osThreadCreate(osThread(gsmTask), gsm);
		if(gsmTaskHandle == NULL)
			{
				BRIDGE_DEBUG_GSM(("\r\nERROR CREAT TASK GSM_Init!!!\r\n"));
			}	
    while (gsm->state != GSM_STATE_READY) //ждем финала
    {
			if(gsm->state == GSM_STATE_ERROR){ //обрабатываем ошибки
          BRIDGE_DEBUG_GSM(("\r\nGSM INIT ERROR\r\n"));

          switch(gsm->error)
          {
            case  GSM_ERROR_INIT_FAIL:     GSM_Config_Reset_State(gsm, huart); break;
            default:                       GSM_Config_Reset_State(gsm, huart); break;
          }
        }
        osDelay(1);
    }
}
/*###############################################################*/
/*###############################################################* GSM_Hardware_Reset -->*/
/*###############################################################*/
static void GSM_Hardware_Reset(GSM_HandleTypeDef *gsm)
{
		BRIDGE_DEBUG_GSM(("MODUL RESET\r\n"));
		GSM_SET_STATE(GSM_STATE_BUSY_RESET)
    HAL_GPIO_WritePin(MOff_GPIO_Port, MOff_Pin, GPIO_PIN_SET);
    osDelay(500);
	  HAL_GPIO_WritePin(MOff_GPIO_Port, MOff_Pin, GPIO_PIN_RESET);
    osDelay(2000);
    HAL_GPIO_WritePin(MOff_GPIO_Port, MOff_Pin, GPIO_PIN_SET);
    osDelay(500);
	  HAL_GPIO_WritePin(MOff_GPIO_Port, MOff_Pin, GPIO_PIN_RESET);
    osDelay(2000);
    HAL_GPIO_WritePin(MOff_GPIO_Port, MOff_Pin, GPIO_PIN_SET);
    osDelay(2000);
		GSM_SET_STATE(GSM_STATE_AFTER_RESET)
}
/*###############################################################*/
/*###############################################################* USART_Change_BaudRate -->*/
/*###############################################################*/
void USART_Change_BaudRate(UART_HandleTypeDef *huart, uint32_t BaudRate)
{
  if (HAL_UART_DeInit(huart) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  huart->Init.BaudRate = BaudRate;

  if (HAL_UART_Init(huart) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
}


#endif /*BRIDGE_USE_GSM*/
