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
	byte 	adr;
	byte 	func;
	byte 	n; 
	byte 	chnl; 
	u32		count;
	byte 	time; 
	byte 	gain; 
	byte 	delay; 
	byte 	filtr;
	u16		data[500]; 
	word	crc; 
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

__packed struct ReqMem
{
	byte adr;
	byte func;
	
	__packed union
	{
		__packed struct  { word crc; } f1;  // ����� ����� ������
		__packed struct  { word crc; } f3;  
	};
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct RspMem
{
	byte	adr;
	byte	func;
	
	__packed union
	{
		__packed struct  { word crc; } f1;  // ����� ����� ������
		__packed struct  { word crc; } f2;  // ������ �������
		__packed struct  { word crc; } f3;  // 
		__packed struct  { word crc; } fFE;  // ������ CRC
		__packed struct  { word crc; } fFF;  // ������������ ������
	};
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct REQ
{
	bool	ready;
	bool	crcOK;
	typedef void tRsp(REQ*);
	
	REQ *next;

	tRsp*	CallBack;
	void*	ptr;

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
	REQ* _req;
	
	byte _state;


	ComPort *com;

	u32		count;

	bool _run;

public:

	RequestQuery(ComPort *p) : _first(0), _last(0), _run(true), _state(0), com(p), count(0) {}
	void Add(REQ* req);
	REQ* Get();
	bool Empty() { return _first == 0; }
	bool Idle() { return (_first == 0) && (_req == 0); }
	bool Stoped() { return _req == 0; }
	void Update();
	void Stop() { _run = false; }
	void Start() { _run = true; }
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct R02
{
	bool memNeedSend;
	ComPort::WriteBuffer	wb;
	ComPort::ReadBuffer		rb;
	REQ		q;
	Req02	req;
	Rsp02	rsp;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


struct RMEM
{
	RMEM* next;

	R02*	r02;

	ComPort::WriteBuffer	wb;
	ComPort::ReadBuffer		rb;
	REQ		q;
	ReqMem	req;
	RspMem	rsp;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++






#endif //REQ_H__09_10_2014__10_31
