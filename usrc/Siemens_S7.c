#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include <sys/wait.h> 
#include <sys/ioctl.h> 
#include <net/if.h> 
#include <net/if_arp.h> 
#include <arpa/inet.h> 

#include "type.h"
#include "log.h"
#include "Siemens_S7.h"

#define     T1           (20)    /*Connect timeout*/
extern u32 TIMTICK;

char uart_rev[200][50] = {0};
char* uart_result[200] = {0};
char sendDataReault[MQTT_MSG_MAX_LEN-40] = {0};
int uart_resultCnt = 0;
int uart_isSendData = 0;

int current_status = _INIT;

char serverIP[20] = {"10.45.222.80"};
u32 serverPort = 102; //   Port
SIEMENS_DEVICESTYPE Siemens_Type = S7_SMART;


char rev[1536];
int len;
int sock;
unsigned long sck_mode;

struct sockaddr_in addr;

static void print_S7Data(char* data,int flag,int len)
{
	u16 i = 0; 
    char buff[500] = {0};
	for(i=0 ; i<len ; i++)
	{
		sprintf(buff+i*3,"%02X ",data[i]);
	}
	if(flag)
	{
		zlg_debug("S7SendData:%s\r\n",buff);
	}
	else
	{
		zlg_debug("S7RecvData:%s\r\n",buff);
	}
}

int S7_Init(void)
{
	int i = 0;
	if(sock>0)
	{
		close(sock);
	}
	for(i=0 ; i<200 ; i++)
	{
		uart_result[i] = uart_rev[i];
	}
    current_status = _INIT;
	bzero(&addr,sizeof(addr));
	addr.sin_port = htons(serverPort);
	addr.sin_family = PF_INET;
	strcpy(serverIP,IOT_PBoxInfo.PBoxInfo_equipmentIP());
	addr.sin_addr.s_addr = inet_addr(serverIP);
	sock = socket (AF_INET, SOCK_STREAM , 0);
	if(sock<0)
	{
		zlg_debug("socket creat error\r\n");
		return sock;
	}

	return 0;
}

int S7NotConnect(void)
{
	int res;
	res = connect (sock, (struct sockaddr_in *)&addr, sizeof (addr));
	return res;
}

int S7Connected(void)
{
	int res = 0;
    char* sendData = NULL;
    char* datarev = NULL;
    unsigned short crc = 0;
	int i = 0;
	int cnt = Get_S7_U16_U32_FLOAT32CNT() + GetS7_BIT_CNT() + GetS7_STR_CNT();
	zlg_debug("cmd cnt = %d\r\n",cnt);
	for(i=0 ; i<cnt ; i++)
	{
		sendData = Get_S7_CMD_STR(i);//数据打包
		res = send (sock, (char*)(sendData), sendData[3], 0);
		if(res<0)
		{
			zlg_debug("Send Error : %d\r\n",res);
			return 2;
		}
		print_S7Data(sendData,1,sendData[3]);
		res = recv (sock, (char*)rev, sizeof(rev), 0);
		if(res>0)
		{
			datarev = rev;
			rev[res] = 0;
		}
		else
		{
			datarev = 0;
		}
		//datarev = IOT_Uart.uart_revDataTimeOut(baud,200);
		if(datarev!=0)
		{
			print_S7Data(datarev,0,res);
			//datarev+=2;
			if(0==S7_AnalyzeData(datarev,i))
			{}
			else
			{
				zlg_debug("analyze error! index = %d\r\n",i);
				return 1;
			}
		}
		else
		{
			zlg_debug("receive data error! index = %d\r\n",i);
			return 2;
		}
	}
	return 0;
}

void Siemens_COTP(u8 type,int m_rack , int m_slot)
{
	int res =0;
	char* datarev = NULL;
	unsigned char p_send_smart[] = {0x03, 0x00, 0x00, 0x16,
									0x11, 0xe0, 0x00, 0x00,
									0x00, 0x48, //ID
									0x00,
									0xc1, 0x02, //Parameter code:src-tsap (0xc1)        length=2
									0x10, 0x00, //source TSAP=1000
									0xc2, 0x02, //Parameter code: dst-tsap (0xc2)       length=2
									0x03, 0x00, //DES  TSAP=0300
									0xc0, 0x01, // Parameter code: tpdu-size (0xc0)     length=1;
									0x0a};		//TUPD SIZE=1024

	//S7-200
	unsigned char p_send_200[] = {0x03, 0x00, 0x00, 0x16,
								  0x11, 0xe0, 0x00, 0x00,
								  0x00, 0x48, //ID
								  0x00,
								  0xc1, 0x02, //Parameter code:src-tsap (0xc1)	    length=2
								  0x4d, 0x57, //source TSAP= MW
								  0xc2, 0x02, //Parameter code: dst-tsap (0xc2)     length=2
								  0x4d, 0x57, //DES  TSAP= MW
								  0xc0, 0x01,			// Parameter code: tpdu-size (0xc0)    length=1
								  0x00}; //TUPD SIZE   0A=1024=2^10   85=32=2^5   67=128=2^7     21   m_rack * 2 + m_slot

	//S7-300 S7-400 S7-1200 S7-1500
	unsigned char p_send_1500[] = {0x03, 0x00, 0x00, 0x16,
								   0x11, 0xe0, 0x00, 0x00,
								   0x00, 0x48, //ID  48閿涳拷 01閿涳拷
								   0x00,
								   0xc0, 0x01,				   // Parameter code: tpdu-size (0xc0)    length=1
								   0x0a,					   //TPU SIZE=1024
								   0xc1, 0x02,				   //Parameter code:src-tsap (0xc1)	   length=2
								   0x01, 0x02,				   //SOURCE  TSAP= 01 02
								   0xc2, 0x02,				   //Parameter code: dst-tsap (0xc2)     length=2
								   0x01, 0x00}; //DES TSAP      m_rack * 2 + m_slot
	if (type == 0){
		res = send(sock,(unsigned char*)p_send_smart, p_send_smart[3], 0);
		print_S7Data(p_send_smart,1,p_send_smart[3]);
	}	
	else if (type == 1){
		p_send_200[21] = m_rack * 2 + m_slot;
		res = send(sock,(unsigned char*)p_send_200, p_send_200[3], 0);
		print_S7Data(p_send_200,1,p_send_200[3]);
	}		
	else{
		p_send_1500[21] = m_rack * 2 + m_slot;
		res = send(sock,(unsigned char*)p_send_1500, p_send_1500[3], 0); //S7-300 S7-400 S7-1200 S7-1500
		print_S7Data(p_send_1500,1,p_send_1500[3]);
	}		
	//res = send (sock, (char*)(sendData)+2, (sendData[0]<<8 | sendData[1]), 0);
	if(res<0)
	{
		zlg_debug("Send Error : %d\r\n",res);
		//return 2;
	}
	res = recv (sock, (char*)rev, sizeof(rev), 0);
	if(res>0)
	{
		datarev = rev;
		rev[res] = 0;
	}
	else
	{
		datarev = 0;
	}
	//datarev = IOT_Uart.uart_revDataTimeOut(baud,200);
	if(datarev!=0)
	{
		print_S7Data(datarev,0,res);
	}
	else
	{
		zlg_debug("receive data error in COTP");
		//return 2;
	}
}

static Siemens_S7_Init(void)
{
	int		m_edit_rank;
	int		m_edit_slot;
	switch (Siemens_Type)
	{
	case 0:
		m_edit_rank = 0;
		m_edit_slot = 0;
		break; //S7-SMART
	case 1:
		m_edit_rank = 0;
		m_edit_slot = 0;
		break; //S7-200
	case 2:
		m_edit_rank = 0;
		m_edit_slot = 2;
		break; //S7-300
	case 3:
		m_edit_rank = 0;
		m_edit_slot = 3;
		break; //S7-400
	case 4:
		m_edit_rank = 0;
		m_edit_slot = 0;
		break; //S7-1200
	case 5:
		m_edit_rank = 0;
		m_edit_slot = 1;
		break; //S7-1500
	default:
		break;
	}
	Siemens_COTP(Siemens_Type,m_edit_rank,m_edit_slot);
}

void Simens_SET_COMMUNICATION(void)
{
	char* datarev = NULL;
	uint8_t res =0;
	unsigned char p_send_smart[] = {0x03, 0x00, 0x00, 0x19,
									0x02, 0xf0, 0x80, 0x32, 0x01,
									0x00, 0x00, 0x00, 0x01,
									0x00, 0x08,
									0x00, 0x00, 0xf0, 0x00, 0x00, 0x01, 0x00, 0x01,
									0x03, 0xc0};

	unsigned char p_send_200[] = {0x03, 0x00, 0x00, 0x19,
								  0x02, 0xf0, 0x80, 0x32, 0x01,
								  0x00, 0x00, 0x00, 0x01,
								  0x00, 0x08,
								  0x00, 0x00, 0xf0, 0x00, 0x00, 0x01, 0x00, 0x01,
								  0x03, 0xc0};
	//S7-300 S7-400 S7-1200 S7-1500
	unsigned char p_send_1500[] = {0x03, 0x00, 0x00, 0x19,
								   0x02, 0xf0, 0x80, 0x32, 0x01,
								   0x00, 0x00, 0x00, 0x01,
								   0x00, 0x08,
								   0x00, 0x00, 0xf0, 0x00, 0x00, 0x01, 0x00, 0x01,
								   0x01, 0xe0};
	//	 3  0  0 19  2 f0 80 32  1  0  0 cc c1  0  8  0  0 f0  0  0  1  0  1  5 a0
	unsigned char p_send_NC808[] = {0x03, 0x00, 0x00, 0x19,
									0x02, 0xf0, 0x80, 0x32, 0x01,
									0x00, 0x00, 0xcc, 0xc1, //ID
									0x00, 0x08,
									0x00, 0x00, 0xf0, 0x00, 0x00, 0x01, 0x00, 0x01,
									0x05, 0xa0};
	if (Siemens_Type == 6){
		res = send(sock,(unsigned char*)p_send_NC808, p_send_NC808[3], 0);
		print_S7Data(p_send_NC808,1,p_send_NC808[3]);
	}	
	else if (Siemens_Type == 0){
		res = send(sock,(unsigned char*)p_send_smart, p_send_smart[3], 0);
		print_S7Data(p_send_smart,1,p_send_smart[3]);
	}	
	else if (Siemens_Type == 1){
		res = send(sock,(unsigned char*)p_send_200, p_send_200[3], 0);
		print_S7Data(p_send_200,1,p_send_200[3]);
	}		
	else{
		res = send(sock,(unsigned char*)p_send_1500, p_send_1500[3], 0); //S7-300 S7-400 S7-1200 S7-1500
		print_S7Data(p_send_1500,1,p_send_1500[3]);
	}
	if(res<0)
	{
		zlg_debug("Send Error : %d\r\n",res);
		//return 2;
	}
	res = recv (sock, (char*)rev, sizeof(rev), 0);
	if(res>0)
	{
		datarev = rev;
		rev[res] = 0;
	}
	else
	{
		datarev = 0;
	}
	//datarev = IOT_Uart.uart_revDataTimeOut(baud,200);
	if(datarev!=0)
	{
		print_S7Data(datarev,0,res);
	}
	else
	{
		zlg_debug("receive data error in COMMUNICATION");
		//return 2;
	}
}

unsigned int notConnectTimeOut = 0;
int SECS_TICK = 1;
void app_S7_Logic(void)
{
	u8 t = 0;
	int res = 0;
	char tmp[30] = {0};
	switch(current_status)
	{
		case _INIT:
			if(S7_Init()!=0)
			{
				current_status = _INIT;
			}
			else
			{
				current_status = _NotConnect;
			}
		break;
		case _NotConnect:
			if(S7NotConnect()!=0)
			{
				current_status = _NotConnect;
				notConnectTimeOut++;
                sleep(2);
				if(notConnectTimeOut > T1)
				{
					notConnectTimeOut = 0;
					current_status = _INIT;
				}
			}
			else
			{
				notConnectTimeOut = 0;
				Siemens_S7_Init();
				Simens_SET_COMMUNICATION();
				current_status = _Connected;
			}
		break;
		case _Connected:
            SECS_TICK++;
			if(!is_empty_MQCmd())
			{
				if(front_MQCmd().flag==1)
				{
					pop_MQCmd();
					if(0==S7Connected())
					{
						//发送数据 
						//RSTCP_SendData(TC_GetResult(0));
						strncpy(sendDataReault,S7_GetResult(0),MQTT_MSG_MAX_LEN-40);
						zlg_debug("analyze success:%s\r\n",sendDataReault);
						S7_ClearResult();
					}
					else
					{
						S7_ClearResult();
						current_status = _INIT;
					}
				}
			}
		break;
	}
}

unsigned char bsp_GetOnLine(void)
{
	return (current_status == _Connected);
}

void app_TestMQ(void)
{
	int i=0;
	if(current_status==_Connected)
	{
		MQCmd temp = {0};
		temp.flag = 1;
		push_MQCmd(temp);
	}
	else
	{
		//memset(sendDataReault,0,MQTT_MSG_MAX_LEN-40);
		memset(uart_rev,0,200*30);
		strncpy(sendDataReault,"-1;-1;-1;",MQTT_MSG_MAX_LEN-40);
		
	}
}


static int split(char *src,const char *separator,char **dest,int *num) {
     char *pNext;
     int count = 0;
     if (src == NULL || strlen(src) == 0) //?????????????0,???? 
        return 1;
     if (separator == NULL || strlen(separator) == 0) //??????????,???? 
        return 2;
     pNext = (char *)strtok(src,separator); //????(char *)????????(??????????????????)
     while(pNext != NULL) {
          strcpy(*dest++ , pNext);
          ++count;

         pNext = (char *)strtok(NULL,separator);  //????(char *)????????
    }  
    *num = count;
	return 0;
} 


typedef enum {BYTE=1,SHORT=2,INT=3,FLOAT=4,STRING=5} DataTypeAPP;
extern void DataSerialization(u8 dataType,int* pos,void* data,char* dataDiscripton);
void app_interface_binary_data(int* pos,MQData* data)
{
	int i = 0;
	int uart_resultCnt = 0;
	char buf[20] = {0};
	const char sep = ';';
	simcom_lbs_receive_info_t* templbs = IOT_GPS.IOT_GetAGPSInfoVal();
	int dataCnt = Get_SendDataCNT();
	split(data->data,&sep,uart_result,&uart_resultCnt);

    for(i=0 ; i< dataCnt; i++)
   	{
		sprintf(buf,"%d->",i);
		//printf("uart_result[%d] : %s\r\n",i,uart_result[i]);
		DataSerialization((u8)STRING,pos,(void*)(uart_result[i]),buf);
	}
	//获取经纬度值
	sprintf(buf,"%f",(float)(templbs->u32Lng)/1000000);
	DataSerialization((u8)STRING,pos,(void*)buf,"longitude");

	sprintf(buf,"%f",(float)(templbs->u32Lat)/1000000);
	DataSerialization((u8)STRING,pos,(void*)buf,"latitude");
	memset(uart_rev,0,200*30);
}

void app_ConvertToQueue(void)
{
	MQData temp = {0};
	strncpy(temp.data,sendDataReault,MQTT_MSG_MAX_LEN-40);
	//zlg_debug("sendDataReault=%s\r\n",sendDataReault);
	//zlg_debug("temp.data=%s\r\n",temp.data);
	IOT_NTP.get_date_time(temp.collTime);
	if(!is_full_MQData())
	{
		push_MQData(temp);
		//zlg_debug("push_MQData ok\r\n size_MQData:%d\r\n",size_MQData());
	}
	else
	{
		zlg_debug("is_full_MQData is true\r\n");
	}
}
