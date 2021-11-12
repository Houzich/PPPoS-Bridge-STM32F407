/**
  ******************************************************************************
  * @file    LwIP/LwIP_HTTP_Server_Socket_RTOS/Src/httpserver-socket.c
  * @author  MCD Application Team
  * @brief   Basic http server implementation using LwIP socket API   
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics International N.V. 
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
#include <string.h>
#include <stdio.h>
#include "cmsis_os.h"
#include "lwip/opt.h"
#include "lwip/api.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "lwip/apps/fs.h"
#include "lwip/dns.h"

#include "HTTP_Server.h"
#include "Bridge_Data.h"
#include "Application_PPPoS.h"
#include "PPPoS_Interface.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
//static const char *dns_request = "GET /DNS Servers";
    /**
  * @brief serve tcp connection  
  * @param conn: connection socket 
  * @retval None
  */

static void HTTP_Server_Serve(int conn)
{
  int buflen = 2048;
  int ret;
  unsigned char recv_buffer[2048];
  /* Read in the request */
  ret = read(conn, recv_buffer, buflen); 
  if(ret < 0) return;

  /* Check if request to get ST.gif */
	if (strncmp((char *)recv_buffer, "CONNECT", sizeof("CONNECT")-1) == 0)
  {
    //CONNECT "vmi.velcom.by","mvi","mvi"
    char *pch[6];
		char *tmp_pcr;
    int i = 0;
    tmp_pcr = pch[0] = strchr((char *)recv_buffer, '"');
    if (pch[0] == NULL) return;
      while ((tmp_pcr != NULL) && (i < 6))
      {
        tmp_pcr = pch[++i] = strchr(tmp_pcr + 1, '"');
      }
      if (i != 6) return;
			for(int i = 0; i<100; i++){
			temp_internet_apn[i] = 0;
			temp_internet_user[i] = 0;
			temp_internet_password[i] = 0;
			}
      memcpy(temp_internet_apn, pch[0]+1, (uint32_t)(pch[1]-pch[0])-1);
			memcpy(temp_internet_user, pch[2]+1, (uint32_t)(pch[3]-pch[2])-1);
			memcpy(temp_internet_password, pch[4]+1, (uint32_t)(pch[5]-pch[4])-1);
			
			write(conn, (const unsigned char *)"\r\nSTART GSM CONNECT!!!!!\r\n\r\n", strlen("\r\nSTART GSM CONNECT!!!!!\r\n\r\n"));
			
			fl_start_gsm_connect = 1;
			while(fl_end_gsm_connect == 0){
			write(conn, (const unsigned char *)"WAIT....\r\n", strlen("WAIT....\r\n"));
				osDelay(1000);
			}				
			tmp_pcr = "\r\n OK. GSM CONNECTED\r\n";
			write(conn, tmp_pcr, strlen(tmp_pcr));
			char string[200];
			
			sprintf(string, "   IPADDR    = %s\r\n", ipaddr_ntoa(&modem_pppos.netif->ip_addr));
			write(conn, string, strlen(string));
			sprintf(string, "   GATEWAY   = %s\r\n", ipaddr_ntoa(&modem_pppos.netif->gw));
			write(conn, string, strlen(string));
			sprintf(string, "   NETMASK   = %s\r\n", ipaddr_ntoa(&modem_pppos.netif->netmask));
			write(conn, string, strlen(string));

			ip_addr_t *ns;
      ns = (ip_addr_t *)dns_getserver(0);
      sprintf(string, "   DNS1        = %s\r\n", ipaddr_ntoa(ns));
			write(conn, string, strlen(string));
      ns = (ip_addr_t *)dns_getserver(1);
      sprintf(string, "   DNS2        = %s\r\n", ipaddr_ntoa(ns));
			write(conn, string, strlen(string));
			
			tmp_pcr = "\r\n SEND REQ on 81.177.141.229\r\n";
			write(conn, tmp_pcr, strlen(tmp_pcr));
			tmp_pcr = string_site_req;
			write(conn, tmp_pcr, strlen(tmp_pcr));
			
			while(fl_send_forward_packet == 0){
			write(conn, (const unsigned char *)"WAIT....\r\n", strlen("WAIT....\r\n"));
				osDelay(1000);
			}
			tmp_pcr = "\r\n RESPONSE: \r\n";
			write(conn, tmp_pcr, strlen(tmp_pcr));
			write(conn, for_test_rx_data, strlen(for_test_rx_data));
			
  }
  else
  {
    /* Load 404 page */
    write(conn, (const unsigned char *)"\r\nLoad 404 page\r\n\r\n", strlen("\r\nLoad 404 page\r\n\r\n"));
    BRIDGE_DEBUG_ETHERNET(" Load 404 page ");
  }
  /* Close connection socket */
  close(conn);
}

/**
  * @brief  http server thread 
  * @param arg: pointer on argument(not used here) 
  * @retval None
  */
static void HTTP_Server_Socket_Thread(void const * argument)
{
  int sock, size, newconn;
  struct sockaddr_in address, remotehost;

 /* create a TCP socket */
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
  {
		for( ;; )
		{
			/* Delete the Init Thread */ 
			osThreadTerminate(NULL);
		}
  }
  /* bind to port 80 at any interface */
  address.sin_family = AF_INET;
  address.sin_port = htons(80);
  address.sin_addr.s_addr = INADDR_ANY;

  if (bind(sock, (struct sockaddr *)&address, sizeof (address)) < 0)
  {
		for( ;; )
		{
			/* Delete the Init Thread */ 
			osThreadTerminate(NULL);
		}
  }
  
  /* listen for incoming connections (TCP listen backlog = 5) */
  listen(sock, 5);
  
  size = sizeof(remotehost);
  
  while (1) 
  {
    newconn = accept(sock, (struct sockaddr *)&remotehost, (socklen_t *)&size);
    HTTP_Server_Serve(newconn);
		osDelay(1);
  }
}

/**
  * @brief  Initialize the HTTP server (start its thread) 
  * @param  none
  * @retval None
  */
void HTTP_Server_Socket_Init(void)
{
  osThreadDef(HTTPTask, HTTP_Server_Socket_Thread, HTTP_SERVER_THREAD_PRIO, 0, HTTP_SERVER_THREAD_STACKSIZE);
  osThreadCreate(osThread(HTTPTask), NULL);
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
