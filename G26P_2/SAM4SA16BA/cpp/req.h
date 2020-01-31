#ifndef REQ_H__09_10_2014__10_31
#define REQ_H__09_10_2014__10_31

#include "ComPort.h"


//struct Request
//{
//	byte adr;
//	byte func;
//	
//	union
//	{
//		struct  { byte n; word crc; } f1;  // старт оцифровки
//		struct  { byte n; byte chnl; word crc; } f2;  // чтение вектора
//		struct  { byte dt[3]; byte ka[3]; word crc; } f3;  // установка периода дискретизации вектора и коэффициента усиления
//	};
//};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct Req01	// старт оцифровки
{
	byte 	len;
	byte 	adr;
	byte 	func;
	byte 	n; 
	u16		vc;
	word 	crc;  
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct Rsp01	// старт оцифровки
{
	byte adr;
	byte func;
	word crc;  
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct Req02	// чтение вектора
{
	byte 	len;
	byte 	adr;
	byte 	func;
	byte 	n; 
	byte 	chnl; 
	word 	crc; 
};  

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct Rsp02	// чтение вектора
{
	u16 rw; 
	u32 cnt; 
	u16 gain; 
	u16 st; 
	u16 len; 
	u16 delay; 
	u16 data[1024*4]; 
	u16 crc;
};  

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct  Req03	// установка периода дискретизации вектора и коэффициента усиления
{ 
	byte 	len;
	byte 	adr;
	byte 	func;
	u16 	st[3]; 
	u16		sl[3]; 
	u16		sd[3];
	word	crc; 
};  

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct  Rsp03	// установка периода дискретизации вектора и коэффициента усиления
{ 
	byte adr;
	byte func;
	word crc; 
};  

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct  Req04	// установка коэффициента усиления
{ 
	byte 	len;
	byte 	adr;
	byte 	func;
	byte 	ka[3]; 
	word 	crc; 
};  

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct  Rsp04	// установка периода дискретизации вектора и коэффициента усиления
{ 
	byte	adr;
	byte	func;
	u16 	maxAmp[4];
	u16		power[4];
	word	crc; 
};  

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct  Req05	// запрос контрольной суммы и длины программы во флэш-памяти
{ 
	byte 	len;
	byte 	adr;
	byte 	func;
	word 	crc; 
};  

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct  Rsp05	// запрос контрольной суммы и длины программы во флэш-памяти
{ 
	byte 	adr;
	byte 	func;
	u16		flashLen; 
	u16		flashCRC;
	word 	crc; 
};  

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct  Req06	// запись страницы во флэш
{ 
	byte 	len;
	byte 	adr;
	byte 	func;
	u16		stAdr; 
	u16		count; 
	word	crc; 
	byte	data[258]; 
};  

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct  Rsp06	// запись страницы во флэш
{ 
	byte 	adr;
	byte 	func;
	u16		res; 
	word	crc; 
};  

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct  Req07	// перезагрузить блэкфин
{ 
	byte 	len;
	byte 	adr;
	byte 	func;
	word 	crc; 
};  

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct ReqTrm01	
{
	byte 	len;
	byte 	func;
	byte 	n; 
	word 	crc;  
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct ReqTrm02	
{
	byte 	len;
	byte 	f;
	word 	crc;  
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct RspTrm02	
{
	byte f; 
	u16 hv; 
	u16 crc;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct ReqTrm03	
{
	byte 	len;
	byte	f; 
	byte	fireCountM; 
	byte	fireCountXY; 
	u16		hv;
	word 	crc;  
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct RspTrm03	
{
	byte f; 
	u16 crc;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct ReqMem
{
	u16 rw; 
	u32 cnt; 
	u16 gain; 
	u16 st; 
	u16 len; 
	u16 delay; 
	u16 data[1024*4]; 
	u16 crc;

	//byte adr;
	//byte func;
	
	//__packed union
	//{
	//	__packed struct  { word crc; } f1;  // Старт новой сессии
	//	__packed struct  { word crc; } f3;  
	//};
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct RspMem
{
	u16 rw; 
	u16 crc; 

	//byte	adr;
	//byte	func;
	
	//__packed union
	//{
	//	__packed struct  { word crc; } f1;  // Старт новой сессии
	//	__packed struct  { word crc; } f2;  // Запись вектора
	//	__packed struct  { word crc; } f3;  // 
	//	__packed struct  { word crc; } fFE;  // Ошибка CRC
	//	__packed struct  { word crc; } fFF;  // Неправильный запрос
	//};
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct RspMan60
{
	u16 rw; 
	u32 cnt; 
	u16 maxAmp[96]; 
	u16 power[96];
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct REQ
{
	bool	ready;
	bool	crcOK;
	bool	checkCRC;
	bool	updateCRC;

	typedef void tRsp(REQ*);

	u16		tryCount;
	
	REQ *next;

	tRsp*	CallBack;
	void*	ptr;

	ComPort::WriteBuffer *wb;
	ComPort::ReadBuffer *rb;

	u32		preTimeOut, postTimeOut;

	REQ() : next(0), wb(0), rb(0), tryCount(0) {}
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class RequestQuery
{
	REQ* _first;
	REQ* _last;
	REQ* _req;
	
	byte _state;

	u16	_crc;
	u16 _crcLen;

	byte* _crcPtr;


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
	R02* next;

//	bool memNeedSend;
	ComPort::WriteBuffer	wb;
	ComPort::ReadBuffer		rb;
	REQ		q;
	Req02	req[2];
	Rsp02	rsp;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


struct RMEM
{
	RMEM* next;

//	R02*	r02;

	ComPort::WriteBuffer	wb;
	ComPort::ReadBuffer		rb;
	REQ		q;
	ReqMem	req;
	RspMem	rsp;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++






#endif //REQ_H__09_10_2014__10_31
