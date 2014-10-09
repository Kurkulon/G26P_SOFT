#ifndef REQ_H__09_10_2014__10_31
#define REQ_H__09_10_2014__10_31

#include "ComPort.h"


struct Request
{
	byte adr;
	byte func;
	
	union
	{
		struct  { byte n; word crc; } f1;  // ����� ���������
		struct  { byte n; byte chnl; word crc; } f2;  // ������ �������
		struct  { byte dt[3]; byte ka[3]; word crc; } f3;  // ��������� ������� ������������� ������� � ������������ ��������
	};
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct Req01	// ����� ���������
{
	byte adr;
	byte func;
	byte n; 
	word crc;  
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct Rsp01	// ����� ���������
{
	byte adr;
	byte func;
	word crc;  
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct Req02	// ������ �������
{
	byte adr;
	byte func;
	byte n; 
	byte chnl; 
	word crc; 
};  

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct Rsp02	// ������ �������
{
	byte adr;
	byte func;
	byte n; 
	byte chnl; 
	u16 data[500]; 
	word crc; 
};  

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct  Req03	// ��������� ������� ������������� ������� � ������������ ��������
{ 
	byte adr;
	byte func;
	byte dt[3]; 
	byte ka[3]; 
	word crc; 
};  

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct  Rsp03	// ��������� ������� ������������� ������� � ������������ ��������
{ 
	byte adr;
	byte func;
	word crc; 
};  

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct Response
{
	byte adr;
	byte func;
	
	union
	{
		struct  { word crc; } f1;  // ����� ���������
		struct  { byte n; byte chnl; u16 data[500]; word crc; } f2;  // ������ �������
		struct  { word crc; } f3;  // ��������� ������� ������������� ������� � ������������ ��������
	};
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct REQ
{
	bool	crcOK;
	typedef void tRsp(REQ*);
	
	REQ *next;

	tRsp*	CallBack;

	ComPort::WriteBuffer *wb;
	ComPort::ReadBuffer *rb;

	u32		preTimeOut, postTimeOut;

	REQ() : next(0), wb(0), rb(0) {}
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class RequestQuery
{
	REQ* _first;
	REQ* _last;

	ComPort *com;

public:

	RequestQuery(ComPort *p) : _first(0), _last(0), com(p) {}
	void Add(REQ* req);
	REQ* Get();
	bool Empty() { return _first == 0; }
	void Update();
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++








#endif //REQ_H__09_10_2014__10_31
