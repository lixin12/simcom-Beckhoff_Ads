#ifndef _Siemens_S7_H
#define _Siemens_S7_H

typedef enum{S7_I=0x81,S7_Q=0x82,S7_M=0x83,S7_DB=0x84,S7_T=0x1F}S7_TYPE;
typedef enum{_U16=1,_U32=2,_FLOAT32=3,_INT16=4,_INT32=5}S7_DATATYPE;
typedef enum{S7_SMART=0,S7_200=1,S7_300=2,S7_400=3,S7_1200=4,S7_1500=5}SIEMENS_DEVICESTYPE;

typedef struct
{
	S7_TYPE         type;                    //func code
	int             high_addr;				 //high start addr
	int             low_addr;                //low start addr 
	S7_DATATYPE     S7_dataType;             //data type u16/u32/float32
	float           ax;                      //scale
	float           b;                       //read length
}S7_ADDRU16U32;

typedef struct
{
	S7_TYPE        type;                     //func code
	int            high_addr;				 //high start addr
	int            low_addr;                 //low start addr 
	int            bits;                     //与操作，取其中哪几位
}S7_ADDR_BIT;

typedef struct
{
	S7_TYPE        type;                          //func code
	int            high_addr;				 //high start addr
	int            low_addr;                //low start addr 
	int            length;//¶ÁÈ¡×ÖÊý                          //read length
	int            highLowExchange;                           //0:不交换，1交换   每2Byte为单位是否交换位置
}S7_ADDR_STRING;

void app_S7_Logic(void);
int S7_Init(void);
void app_TestMQ(void);
void app_ConvertToQueue(void);
unsigned char bsp_GetOnLine(void);
char* bsp_GetDistMac(void);
unsigned char app_GetDistMac(void);

char* S7_GetResult(unsigned char i);

void S7_ClearResult(void);
int Get_SendDataCNT(void);
unsigned char S7_AnalyzeData(unsigned char* data,unsigned char index);



double Get_S7_ax(int i);
float Get_S7_b(int i);//采集个数
unsigned int Get_S7_Addr_Bit_Bits(int i);
int Get_HighLow_Exchange(int i);
void S7_CMD_Init(void);

unsigned char GetS7_U16_CNT(void);
unsigned char GetS7_U32_CNT(void);
unsigned char GetS7_FLOAT32_CNT(void);
unsigned char GetS7_BIT_CNT(void);
unsigned char GetS7_STR_CNT(void);
unsigned int S7_String_Read_Len(int j);

char* Get_S7_CMD_STR(int i);
char Get_S7_CMD_FUNCode(int i);

unsigned char GetDeviceID(void);
int TC_GetResultCnt(void);


#endif
