#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "type.h"
#include "Siemens_S7.h"
#include "log.h"
int U16_CNT = 0 ;
int U32_CNT = 0;
int FLOAT_CNT = 0;
int Data_CNT = 0;
//typedef enum{R=0x9c,B=0xa0,MR=0x90,LR=0x92,CR=0x91,CM=0xa9,DM=0xa8,FM=0xaf,ZF=0xb0,W=0xb4,TN=0xc2,TS=0xc1,CN=0xc5,CS=0xc4}MC_TYPE;
extern unsigned char DeviceID ;
extern S7_ADDRU16U32 S7_AddrU16U32[];
extern S7_ADDR_BIT S7_Addr_Bit[];
extern S7_ADDR_STRING S7_Addr_String[];
extern int Get_S7_U16_U32_FLOAT32CNT(void);
void Init_U16_U32_FLOATCNT(void)
{
	int i = 0;
	for(i=0 ; i<Get_S7_U16_U32_FLOAT32CNT();i++)
	{
		if(S7_AddrU16U32[i].S7_dataType==_U16 || S7_AddrU16U32[i].S7_dataType==_INT16)
		{
			U16_CNT++;
			Data_CNT += S7_AddrU16U32[i].b*1;
		}
		else if(S7_AddrU16U32[i].S7_dataType==_U32 || S7_AddrU16U32[i].S7_dataType==_INT32)
		{
			U32_CNT++;
			Data_CNT += S7_AddrU16U32[i].b*1;
		}
		else if(S7_AddrU16U32[i].S7_dataType==_FLOAT32)
		{
			FLOAT_CNT++;
			Data_CNT += S7_AddrU16U32[i].b*1;
		}
	}
	printf("U16_CNT=%d;U32_CNT=%d;FLOAT_CNT=%d\r\n",U16_CNT,U32_CNT,FLOAT_CNT);
}
double Get_S7_ax(int i)
{
	return S7_AddrU16U32[i].ax;
}
float Get_S7_b(int i)
{
	return S7_AddrU16U32[i].b;
}

static unsigned int S7_Bit_Read_Len(int j)
{
	int i=0;
	for(i=0 ; i<16 ; i++)
	{
		if((S7_Addr_Bit[j].bits&(0x8000>>i))!=0)
		{
			//zlg_debug("num=%d\r\n",(15-i)/8+1);
			return ((15-i)/8+1);
		}
	}
	return 0;
}
unsigned int S7_String_Read_Len(int j)
{
	if(S7_Addr_String[j].length%2 != 0)
		return (S7_Addr_String[j].length/2+1);
	else
		return (S7_Addr_String[j].length/2);
}

unsigned int Get_S7_Addr_Bit_Bits(int i)
{
	return S7_Addr_Bit[i].bits;
}


int Get_HighLow_Exchange(int i)
{
	return S7_Addr_String[i].highLowExchange;
}
char changeToChar(char data)
{
    switch(data)
    {
        case 0:case 1:case 2:case 3:case 4:case 5:case 6:case 7:case 8:case 9:
            return (data + '0');
        case 10:
            return 'a';
        case 11:
            return 'b';
        case 12:
            return 'c';
        case 13:
            return 'd';
        case 14:
            return 'e';
        case 15:
            return 'f';
        default:
            return 0xff;
    }
}

u8 buf[50] = {
	0x03, 0x00, 0x00, 0x1f,
	0x02, 0xf0, 0x80, 0x32,
	0x01,
	0x00, 0x00, 0x00, 0x01,
	0x00, 0x0e,
	0x00, 0x00,
	0x04, 0x01,
	0x12, 0x0a,
	0x10,
	0x02,//Transport_size
	0x00, 0x00,	//2byte lenth  
	0x00, 0x00,		//闂嗚埖妞傞惃鍕殶閹
	0x00,//funcode
	0x00, 0x00, 0x00 //閺佺増宓佹担搴濈秴
};
u16 sysbyte = 0;
u8* create_S7_Type(u8 SlaveID,u8 FuncCode,int highstartAddr,int lowstartAddr,u16 readLth)
{
	union
	{
		unsigned char b[4];
		int v;
	} iv;	
	iv.v = readLth;
	buf[23] =  iv.b[1];
	buf[24] =  iv.b[0];
	buf[27] =  FuncCode;
	if(FuncCode == S7_I || FuncCode == S7_Q || FuncCode == S7_M)
	{
		iv.v = 0;
		buf[25] = iv.b[1];
		buf[26] = iv.b[0];
		iv.v = highstartAddr * 8 + lowstartAddr;		
		buf[29] = iv.b[1];
		buf[30] = iv.b[0];
	}
	else if(FuncCode == S7_DB)
	{
		iv.v = highstartAddr;
		buf[25] = iv.b[1]; //閺
		buf[26] = iv.b[0];
		iv.v = lowstartAddr * 8;
		buf[28] = iv.b[2];
		buf[29] = iv.b[1]; //閺
		buf[30] = iv.b[0];
	}
	else  //S7_T
	{
		buf[22] = 0x1F;
		iv.v = highstartAddr;
		buf[28] = iv.b[2];
		buf[29] = iv.b[1]; //閺
		buf[30] = iv.b[0];
	}
	return buf;
}

void S7_CMD_Init(void)
{
	Init_U16_U32_FLOATCNT();
}

unsigned char GetS7_U16_CNT(void)
{
	return U16_CNT;
}
unsigned char GetS7_U32_CNT(void)
{
	return U32_CNT;
}
unsigned char GetS7_FLOAT32_CNT(void)
{
	return FLOAT_CNT;
}
extern unsigned char GetS7_BIT_CNT(void);

//S7_TCP_ADDR_STRING S7_TCP_ADDR_STRING
extern unsigned char GetS7_STR_CNT(void);


static char* Get_S7_U16_U32_FLOAT32CMDSTR(int i)
{
	if((S7_AddrU16U32[i].S7_dataType == _U16) || (S7_AddrU16U32[i].S7_dataType == _INT16))
	{
		if(S7_AddrU16U32[i].type == S7_T)
			return create_S7_Type(DeviceID,S7_AddrU16U32[i].type,S7_AddrU16U32[i].high_addr,S7_AddrU16U32[i].low_addr,(u16)(S7_AddrU16U32[i].b));
		else
			return create_S7_Type(DeviceID,S7_AddrU16U32[i].type,S7_AddrU16U32[i].high_addr,S7_AddrU16U32[i].low_addr,(u16)(2*S7_AddrU16U32[i].b));
	}
	else if((S7_AddrU16U32[i].S7_dataType == _U32) || (S7_AddrU16U32[i].S7_dataType == _INT32) ||  (S7_AddrU16U32[i].S7_dataType == _FLOAT32))
	{
		if(S7_AddrU16U32[i].type == S7_T)
			return create_S7_Type(DeviceID,S7_AddrU16U32[i].type,S7_AddrU16U32[i].high_addr,S7_AddrU16U32[i].low_addr,(u16)(2*S7_AddrU16U32[i].b));
		else
			return create_S7_Type(DeviceID,S7_AddrU16U32[i].type,S7_AddrU16U32[i].high_addr,S7_AddrU16U32[i].low_addr,(u16)(4*S7_AddrU16U32[i].b));
	}
	return "ERROR";
}
static char* Get_S7_bITCMDSTR(int i)
{
	if(i<GetS7_BIT_CNT())
	{
		if(S7_Addr_Bit[i].type == S7_T)
			return create_S7_Type(DeviceID,S7_Addr_Bit[i].type,S7_Addr_Bit[i].high_addr,S7_Addr_Bit[i].low_addr ,1);
		else
			return create_S7_Type(DeviceID,S7_Addr_Bit[i].type,S7_Addr_Bit[i].high_addr,S7_Addr_Bit[i].low_addr ,S7_Bit_Read_Len(i));
	}
	return "ERROR";
}
static char* Get_S7_STRCMDSTR(int i)
{
	if(i<GetS7_STR_CNT())
	{
		if(S7_Addr_String[i].type == S7_T)
		{
			zlg_debug("string read length=%d\r\n",S7_String_Read_Len(i));
			return create_S7_Type(DeviceID,S7_Addr_String[i].type,S7_Addr_String[i].high_addr,S7_Addr_String[i].low_addr,S7_String_Read_Len(i));
		}
		else
			return create_S7_Type(DeviceID,S7_Addr_String[i].type,S7_Addr_String[i].high_addr,S7_Addr_String[i].low_addr,S7_Addr_String[i].length);
	}
	return "ERROR";
}
char* Get_S7_CMD_STR(int i)
{
	if(i<Get_S7_U16_U32_FLOAT32CNT()+GetS7_BIT_CNT()+GetS7_STR_CNT())
	{
		if(i<Get_S7_U16_U32_FLOAT32CNT())
		{
			return Get_S7_U16_U32_FLOAT32CMDSTR(i);
		}
		else if(i<Get_S7_U16_U32_FLOAT32CNT()+GetS7_BIT_CNT())
		{
			return Get_S7_bITCMDSTR(i-Get_S7_U16_U32_FLOAT32CNT());
		} 
		else if(i<Get_S7_U16_U32_FLOAT32CNT()+GetS7_BIT_CNT()+GetS7_STR_CNT())
		{
			return Get_S7_STRCMDSTR(i-(Get_S7_U16_U32_FLOAT32CNT()+GetS7_BIT_CNT()));
		}
	}
	return "ERROR";
}
char Get_S7_CMD_FUNCode(int i)
{
	if(i<Get_S7_U16_U32_FLOAT32CNT()+GetS7_BIT_CNT()+GetS7_STR_CNT())
	{
		if(i<Get_S7_U16_U32_FLOAT32CNT())
		{
			return S7_AddrU16U32[i].type;
		}
		else if(i<Get_S7_U16_U32_FLOAT32CNT()+GetS7_BIT_CNT())
		{
			return S7_Addr_Bit[i-(Get_S7_U16_U32_FLOAT32CNT())].type;
		}
		else if(i<Get_S7_U16_U32_FLOAT32CNT()+GetS7_BIT_CNT()+GetS7_STR_CNT())
		{
			return S7_Addr_String[i-(Get_S7_U16_U32_FLOAT32CNT()+GetS7_BIT_CNT())].type;
		}
	}
	return 0;
}
unsigned char GetDeviceID(void)
{
	return DeviceID;
}
void SetDeviceID(unsigned char id)
{
	DeviceID = id;
}
int Get_SendDataCNT(void)
{
	return (Data_CNT + GetS7_BIT_CNT() + GetS7_STR_CNT());
}