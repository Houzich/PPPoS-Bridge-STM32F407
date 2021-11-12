
#include "cmsis_os.h"

#include <string.h>

//#include "lwip/err.h"
//#include "lwip/sockets.h"
//#include "lwip/sys.h"
//#include "lwip/netdb.h"
//#include "lwip/dns.h"
//#include "lwip/opt.h"
//#include "lwip/arch.h"
//#include "lwip/api.h"
#include "lwip/sockets.h"

#include "Bridge_Data.h"
#include "PPPoS_HTTP_Client.h"


char *string_site_req =  
"GET /ip HTTP/1.1\r\n\
Accept: text/html, application/xhtml+xml, image/jxr, */*\r\n\
Referer: http://httpbin.org/\r\n\
Accept-Language: en-US,en;q=0.7,ru;q=0.3\r\n\
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.36 Edge/16.16299\r\n\
Accept-Encoding: gzip, deflate\r\n\
Host: httpbin.org\r\n\
Connection: Keep-Alive\r\n\
Cookie: _gauges_unique_month=1; _gauges_unique=1; _gauges_unique_year=1; _gauges_unique_hour=1; _gauges_unique_day=1\r\n\
\r\n";

//char *string_site_req =  
//"GET / HTTP/1.1\r\n\
//Host: 92.124.147.233\r\n\
//User-Agent: Mozilla/5.0 zgrab/0.x\r\n\
//Accept: /\r\n\
//Accept-Encoding: gzip\r\n\
//\r\n";

//char *string_site_req =  
//"POST /index.php?r=v1%2Ftest%2Fregister HTTP/1.1\r\n\
//Content-Length: 24\r\n\
//Host: api.ivend.pro\r\n\
//Content-Type: application/json\r\n\
//\r\n\
//{\"UID\":\"000008-111117\"}\r\n\
//\r\n";


int PPPoS_HTTP_Client(uint32_t site_ip) {
  int sock;
  char recv_buf[700];
  int res;

  struct sockaddr_in serv_addr;
  uint32_t bytes;

     lwip_socket_init();

     if ((sock = lwip_socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        BRIDGE_DEBUG_HTTP_PPPOS("Error creating socket\n\r");
        return -1;
     }
		 
     serv_addr.sin_len = sizeof(serv_addr);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_port = PP_HTONS(SITE_PORT);
     serv_addr.sin_addr.s_addr = site_ip; //inet_addr("54.152.208.69");

     int opt = 3000;
     res = lwip_setsockopt(sock , SOL_SOCKET, SO_RCVTIMEO, &opt,   sizeof(int));
     res = lwip_connect(sock, (struct sockaddr *)&serv_addr, sizeof (serv_addr));


     if (res < 0) {
        BRIDGE_DEBUG_HTTP_PPPOS("\r\nCould not connect!!!\r\n");
        lwip_close(sock);

     } else {
        BRIDGE_DEBUG_HTTP_PPPOS("\r\n\r\n SITE CONNECTED!!!\r\n\r\n");
        bytes = lwip_write(sock, string_site_req, strlen(string_site_req));
				 //bytes = lwip_send(sock, string_req, strlen(string_req),0);


     BRIDGE_DEBUG_HTTP_PPPOS("\r\nBYTES WROTE: %d\r\n",bytes);
		 BRIDGE_DEBUG_HTTP_PPPOS("\r\nSEND MESSAGE:\r\n%s\r\n\r\n",string_site_req);

      res = lwip_read(sock, recv_buf, 700);

			lwip_close(sock);
			 
      BRIDGE_DEBUG_HTTP_PPPOS("\r\n\r\nRead return code: %d\r\n",res);
			BRIDGE_DEBUG_HTTP_PPPOS("\r\nBuffer:\r\n%s\r\n",recv_buf);
			strcpy((char *)for_test_rx_data,(char *)recv_buf);
     }
  return 1;
}



















