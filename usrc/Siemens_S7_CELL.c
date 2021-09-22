#include "type.h"
#include "Siemens_S7.h"
//typedef enum{RCS=0x01,RIS=2,RHR=3,RIR=4,FSC=5,PSR=6,FMC=15,PMR=16}MODBUS_TCP_TYPE; func code
//_U16=1,_U32=2,_FLOAT32=3,_INT16=4,_INT32=5
unsigned char DeviceID = 1;
S7_ADDRU16U32 S7_AddrU16U32[]= {   //16位，32位数据定义
//func code,  high start addr,  low start addr,  analyze data type,  scale, read length
	{S7_DB,          100,                0,              _U16,          1,      2},
	{S7_DB,          600,                0,              _INT16,          1,      2},
	{S7_DB,          200,                0,              _U32,          1,      2},
	{S7_DB,          700,                0,              _INT32,          1,      2},
	{S7_DB,          300,                0,              _FLOAT32,      1,      2},
//	{S7_DB,          1,                402,              _U32,          1,      1},
//	{S7_DB,          1,                1208,              _FLOAT32,          1,      1},
//	{S7_DB,          1,                1204,              _FLOAT32,          1,      1},
//	{S7_DB,          1,                1200,              _FLOAT32,          1,      1},
};
S7_ADDR_BIT S7_Addr_Bit[] = 
{
//func code,  start addr,  low start addr,  bits(取其中某几位)读取长度由bits中第一位不为0的位置数决定
	{S7_DB,          400,        0,           0xffff},
//	{S7_M,          10,        0,           0x0001},
//	{S7_M,          10,        0,           0x0002},
};

S7_ADDR_STRING S7_Addr_String[] = 
{
//func code,   start addr,  low start addr,  read length, 是否交换顺序
	{S7_DB,          500,        0,               1,         0},
	{S7_DB,          500,        0,               2,         0},
	{S7_DB,          500,        0,               3,         0},
	{S7_DB,          500,        0,               4,         1},
//	{S7_I,           20,        0,               2,         0}
};

//一下函数禁止修改
int Get_S7_U16_U32_FLOAT32CNT(void)
{
	return sizeof(S7_AddrU16U32)/sizeof(S7_ADDRU16U32);
}
unsigned char GetS7_BIT_CNT(void)
{
	return sizeof(S7_Addr_Bit)/sizeof(S7_ADDR_BIT);
}

unsigned char GetS7_STR_CNT(void)
{
	return sizeof(S7_Addr_String)/sizeof(S7_ADDR_STRING);
}