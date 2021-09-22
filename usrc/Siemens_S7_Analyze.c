#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "type.h"
#include "log.h"
#include "Siemens_S7.h"
extern S7_ADDRU16U32 S7_AddrU16U32[];
extern S7_ADDR_BIT S7_Addr_Bit[];
extern S7_ADDR_STRING S7_Addr_String[];
static char result[MQTT_MSG_MAX_LEN-40] = {0};

char ABC_Digit[62]={"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"};

char* Lx_itoaEx(int value, char* str, int radix) {
    static char dig[] ="0123456789abcdefghijklmnopqrstuvwxyz";
    int n = 0, neg = 0;
    unsigned int v;
    char* p, *q;
    char c;
	zlg_debug("value is %d\r\n",value);
    if (radix == 10 && value < 0) {
        value = -value;
        neg = 1;
    }
    v = value;
    do {
        str[n++] = dig[v%radix];
        v /= radix;
    } while (v);
    if (neg)
        str[n++] = '-';
    str[n] = '\0';
    for (p = str, q = p + (n-1); p < q; ++p, --q)
        c = *p, *p = *q, *q = c;
    return str;
}

unsigned char Inquire_letter(unsigned char letter)
{
	if(letter>=48&&letter<=57)
		return 1;
	else if(letter>=65&&letter<=90)
		return 2;
	else if(letter>=97&&letter<=120)
		return 3;
	else 
		return 0;
}
char ASCII(unsigned char letter)
{
	char ABC;
	switch (Inquire_letter(letter))
	{
		case 1:
			ABC = ABC_Digit[letter-0x30];
			break;
		case 2:
			ABC = ABC_Digit[letter-0x37];
			break;
		case 3:
			ABC = ABC_Digit[letter-0x3D];
		default:
			return 0;
	}
	return ABC;
}

void resolve_lotname(unsigned char * HEXLotName,char * Lotname)
{
		char * p = Lotname;
        unsigned char ii = 0;
		for(ii = 0;ii<8;ii++)
		{
			if(ASCII(HEXLotName[ii])!=0)
			{
				*p=ASCII(HEXLotName[ii]);
				p++;
			}		
		}
}

char *lotnameRevData(char *hexlotname,unsigned char * data,unsigned char len)
{
	unsigned char hexname[8];
	memcpy(hexname,data,len);
	resolve_lotname(hexname,hexlotname);
	return hexlotname;
}

//将Hex字符转换成无符号整型
unsigned int hex2int(char c)
{
    if( c >= '0' && c <= '9')
    {
        return (unsigned int) (c -48);
    }
    else if( c >= 'A' && c <= 'F')
    {
        return (unsigned int) (c -55);
    }
    else if( c >= 'a' && c <= 'f')
    {
        return (unsigned int) (c - 87);
    }
    else
    {
        return 0;
    }
}
unsigned char* getRevDataCnt(unsigned char* data,unsigned int cnt)
{
	unsigned short ucount;
	ucount = (unsigned short)((hex2int(data[0])<<12) | (hex2int(data[1])<<8) | (hex2int(data[2])<<4) | (hex2int(data[3])<<0));
	data++;
	data++;
	data++;
	data++;
	printf("%s\r\n",data);
	if(ucount+18 == cnt)
	{
		return data;
	}
	else
	{
		printf("ucount:%d  cnt:%d \r\n",ucount,cnt);
		return 0;
	}
}

unsigned char* isOverRevData(unsigned char* data)
{
	unsigned char ret;
	if(*data++ == '0' && *data++ == '0' && *data++ == '0' && *data++ == '0')
		ret = 1;
	if (ret == 1) 
	return data;
	else
	return 0;
}


unsigned char * AnalyzeHalfwordtatusData(unsigned char* data,unsigned short *data16)
{
	
	*data16=(unsigned short)((hex2int(data[0])<<12) | (hex2int(data[1])<<8) | (hex2int(data[2])<<4) | (hex2int(data[3])<<0));
	data=data+4;
	return data;
}
unsigned char * AnalyzeRevData(unsigned char* data,unsigned int *data32)
{
	unsigned short tp1 = 0;
	unsigned short tp2 = 0;
	data = AnalyzeHalfwordtatusData(data,&tp1);
	data = AnalyzeHalfwordtatusData(data,&tp2);
	*data32=((unsigned int)tp1<<16 | (unsigned int)tp2);
	return data;
}

static void S7_AnalyzeU16Data(char* data,unsigned char index,unsigned char func_code)
{
	char strval[50] = {0};
	int j = 0;
	short tmp = 0;
	//if((int)Get_S7_b(index)*2*8 == (u16)((u16)data[23]<<8|data[24]))//判断长度是否一致
	//{
		for(j=0 ; j<(int)Get_S7_b(index) ; j++)
		{
			if(func_code == S7_T)
			{
				tmp = data[28+j*5];
				tmp = tmp<<8;
				tmp |= data[29+j*5];
			}
			else
			{
				tmp = data[25+j*2];
				tmp = tmp<<8;
				tmp |= data[26+j*2];
			}         
			if(S7_AddrU16U32[index].S7_dataType==_U16)
			{
				sprintf(strval,"%.12g;",(float)(Get_S7_ax(index)*((unsigned short)tmp)));
			}
			else if(S7_AddrU16U32[index].S7_dataType==_INT16)
			{
				sprintf(strval,"%.12g;",(float)(Get_S7_ax(index)*(tmp)));
			}
			
			strval[strlen(strval)] = 0;
			strcat(result,strval);
		}
		zlg_debug("---%s\r\n",result);
	//}
	//else
	//{
	//	zlg_debug("S7_AnalyzeU16Data error\r\n,(int)Get_S7_b(index)*2=%d;data[8]=%d\r\n",(int)Get_S7_b(index)*2,data[8]);
	//}
}
typedef union _int {
  int i;
  char  bufI[4];
} IntUnion;
static void S7_AnalyzeU32Data(char* data,unsigned char index,unsigned char func_code)
{
	char strval[50] = {0};
	IntUnion iu;
	int j = 0;
	//if((int)Get_S7_b(index)*4*8 == (u16)((u16)data[23]<<8|data[24]))
	//{
		for(j=0 ; j<(int)Get_S7_b(index) ; j++)
		{
			if(func_code == S7_T)
			{
				iu.bufI[3] = data[28+j*10];
				iu.bufI[2] = data[29+j*10];
				iu.bufI[1] = data[33+j*10];
				iu.bufI[0] = data[34+j*10];
			}
			else
			{
				iu.bufI[3] = data[25+j*4];
				iu.bufI[2] = data[25+j*4+1];
				iu.bufI[1] = data[25+j*4+2];
				iu.bufI[0] = data[25+j*4+3];
			}	
			if(S7_AddrU16U32[index].S7_dataType==_U32)
			{
				sprintf(strval,"%.12g;",(float)((float)((unsigned int)iu.i*Get_S7_ax(index))));
			}
			else if(S7_AddrU16U32[index].S7_dataType==_INT32)
			{
				sprintf(strval,"%.12g;",(float)(iu.i*Get_S7_ax(index)));
			}
			strval[strlen(strval)] = 0;
			strcat(result,strval);
		}
		zlg_debug("---%s\r\n",result);
	//}
	//else
	//{
	//	zlg_debug("S7_AnalyzeU32Data error\r\n,(int)Get_S7_b(index)*2=%d;data[8]=%d\r\n",(int)Get_S7_b(index)*2,data[8]);
	//}
}
typedef union _float {
  float f;
  char  bufF[4];
} FloatUnion;
static void S7_AnalyzeFloat32Data(char* data,unsigned char index,unsigned char func_code)
{
	char strval[50] = {0};
	float tmp = 0;
	int j = 0;
	FloatUnion fu;
	//if((int)Get_S7_b(index)*4*8 == (u16)((u16)data[23]<<8|data[24]))
	//{
		for(j=0 ; j<Get_S7_b(index) ; j++)
		{
			if(func_code == S7_T)
			{
				fu.bufF[3] = data[28+j*10];
				fu.bufF[2] = data[29+j*10];
				fu.bufF[1] = data[33+j*10];
				fu.bufF[0] = data[34+j*10];
			}
			else
			{
				fu.bufF[3] = data[25+j*4];
				fu.bufF[2] = data[25+j*4+1];
				fu.bufF[1] = data[25+j*4+2];
				fu.bufF[0] = data[25+j*4+3];
			}				
			zlg_debug("fu.f=%.3f\r\n",fu.f);
			sprintf(strval,"%.12g;",(float)(fu.f*Get_S7_ax(index)));
			strval[strlen(strval)] = 0;
			strcat(result,strval);
		}
		zlg_debug("---%s\r\n",result);
	//}
	//else
	//{
	//	zlg_debug("S7_AnalyzeFloat32 error\r\n,(int)Get_S7_b(index)*2=%d;data[8]=%d\r\n",(int)Get_S7_b(index)*2,data[8]);
	//}
}

static void S7AnalyzeBitData(char* data,unsigned char index,unsigned char func_code)
{
	char strval[50] = {0};
	if(func_code == S7_T)
	{
		Lx_itoaEx(((data[28]<<8 | data[29])&Get_S7_Addr_Bit_Bits(index)), strval , 2);
	}
	else
	{
		if((data[24]==8)||(data[24]==1))
			Lx_itoaEx((data[25]&Get_S7_Addr_Bit_Bits(index)), strval , 2);
		else if((data[24]==16)||(data[24]==2))
			Lx_itoaEx(((data[25]<<8 | data[26])&Get_S7_Addr_Bit_Bits(index)), strval , 2);
		else if((data[24]==24)||(data[24]==3))
			Lx_itoaEx(((data[25]<<16 | data[26]<<8 | data[27])&Get_S7_Addr_Bit_Bits(index)), strval , 2);
		else if((data[24]==32)||(data[24]==4))
			Lx_itoaEx(((data[25]<<24 | data[26]<<16 | data[27]<<8 | data[28])&Get_S7_Addr_Bit_Bits(index)), strval , 2);
	}
	strval[strlen(strval)] = ';';
	strval[strlen(strval)+1] = 0;
	strcat(result,strval);
	zlg_debug("---%s\r\n",result);
}

static void S7_AnalyzeStringData(char* data,unsigned char index,unsigned char func_code)
{
	char strval[50] = {0};
	char strbuff[50] = {0};
	int i = 0;
	int read_length = 0;
	if(Get_HighLow_Exchange(index)==1)
	{
		if(func_code == S7_T)
		{
			read_length = S7_String_Read_Len(index);
			for(i=0; i<read_length; i++)
			{
				strbuff[2*i+1] = data[28+i*5];
				strbuff[2*i] = data[29+i*5];
			}
			strncpy(strval,strbuff,S7_Addr_String[index].length);
		}
		else
		{
			for(i=0 ; i<data[24];i=i+2)
			{
				strval[i] = data[26+i];
				strval[i+1] = data[25+i];
			}
		}	
	}
	else if(Get_HighLow_Exchange(index)==0)
	{
		if(func_code == S7_T)
		{
			read_length = S7_String_Read_Len(index);
			for(i=0; i<read_length; i++)
			{
				strbuff[2*i] = data[28+i*5];
				strbuff[2*i+1] = data[29+i*5];
			}
			strncpy(strval,strbuff,S7_Addr_String[index].length);
		}
		else
		{
			for(i=0 ; i<data[24];i++)
			{
				strval[i] = data[25+i];
			}
		}	
	}
	strval[strlen(strval)] = ';';
	strval[strlen(strval)+1] = 0;
	strcat(result,strval);
	zlg_debug("---%s\r\n",result);
}

unsigned char S7_AnalyzeData(unsigned char* data,unsigned char index)
{
	if(data[21]==0xff && ((data[22]==0x04)||(data[22]==0x09)))
	{
		if(index<Get_S7_U16_U32_FLOAT32CNT())
		{
			if(S7_AddrU16U32[index].S7_dataType==_U16 || S7_AddrU16U32[index].S7_dataType==_INT16)
			{
				S7_AnalyzeU16Data(data,index,S7_AddrU16U32[index].type);
			}
			else if(S7_AddrU16U32[index].S7_dataType==_U32 || S7_AddrU16U32[index].S7_dataType==_INT32)
			{
				S7_AnalyzeU32Data(data,index,S7_AddrU16U32[index].type);
			}
			else if(S7_AddrU16U32[index].S7_dataType==_FLOAT32)
			{
				S7_AnalyzeFloat32Data(data,index,S7_AddrU16U32[index].type);
			}
			return 0;
		}
		else if(index<Get_S7_U16_U32_FLOAT32CNT()+GetS7_BIT_CNT())
		{
			S7AnalyzeBitData(data,index-Get_S7_U16_U32_FLOAT32CNT(),S7_Addr_Bit[index-Get_S7_U16_U32_FLOAT32CNT()].type);
			return 0;
		}
		else if(index<Get_S7_U16_U32_FLOAT32CNT()+GetS7_BIT_CNT()+GetS7_STR_CNT())
		{
			S7_AnalyzeStringData(data,index-(Get_S7_U16_U32_FLOAT32CNT()+GetS7_BIT_CNT()),S7_Addr_String[index-(Get_S7_U16_U32_FLOAT32CNT()+GetS7_BIT_CNT())].type);
			return 0;
		}
		return 1;
	}
	return 3;
}




char* S7_GetResult(unsigned char i)
{
	return result;
}
void S7_ClearResult(void)
{
	memset(result,0,sizeof(result));
}
int S7_GetResultCnt(void)
{
	return 0;
}

