#ifndef MODBUS__09_02_2007__08_38
#define MODBUS__09_02_2007__08_38

#include "types.h"
#include "Vars.h"
#include "ComPort.h"

#ifdef WIN32

#include <winsock.h>

#endif


struct ReqState
{
	enum	State { WAIT, READ, WRITE, RESPONSE };	// 0 - формирование событий, 1 - выполнение событий запись
													// 2 - выполнение событий чтение, 3 - формирование ответа
	State	value;
	ReqState(State v) { value = v;}
	void operator=(State v) { value = v; }
	operator State() { return value; }
};


class Modbus
{
  protected:

	struct Chain	{ word start; word count; };

	struct VarEvent
	{
		word	start;
		word	count;
		byte*	data;
		Var*	var;
	};

	struct PDU
	{
		__packed byte	function;

		union
		{
			__packed byte b[254];
			__packed word w[127];
		} data;
	};

	struct Message
	{
		bool	ready;
		
		word	trnsId;
		word	prtclID;
		word	len;
		byte	address;
		PDU		pdu;
	};

	bool		connected;
	bool		listenOnlyMode;
	bool		resetListenOnlyMode;

	byte		mode;
	byte		portNum;
	byte		netAddress;
	byte		acsRights;

	word		numChains;
	word		maxChains;
	word		numEvents;
	word		maxEvents;
	word		diagnosticRegister;
	word		busMessageCount;
	word		busCommunicationErrorCount;
	word		busExeptionErrorCount;
	word		slaveMessageCount;
	word		slaveNoResponseCount;
	word		slaveCountNAK;
	word		slaveBusyCount;
	word		busCharacterOverrunCount;
	word		time35;

	dword		accessTimeout;
	dword		prevAccessTime;

	Vars*		vars;
	ReqState	state;	
	Chain		chains[16];

	VarEvent	varsEvent[125];

	Message		reqMessage, rspMessage;

	ComPort					comPort;
	ComPort::ReadBuffer		readBuffer;
	ComPort::WriteBuffer	writeBuffer;


	bool		CreateChains(Vars* vars);
	bool		CheckChain(word start, word count);
	void		CreateEvents(word start, word count, byte *data);

	void		CreateExeption(byte errorCode, byte exeptionCode, Message *response);
	ReqState	ModbusFunction_03(const Message &request, Message *response);
	ReqState	ModbusFunction_04(const Message &request, Message *response);
	ReqState	ModbusFunction_06(const Message &request, Message *response);
	ReqState	ModbusFunction_08(const Message &request, Message *response);
	ReqState	ModbusFunction_16(const Message &request, Message *response);
	ReqState	ModbusFunction(const Message &req, Message *rsp);
	void		Request();

#ifdef WIN32

	SOCKET		lstnSocket;
	SOCKET		accptSocket;

#endif

  public:

				Modbus();

	void		Init(Vars& v);
	bool		Connect(byte port, dword speed, byte parity, byte address, word delay);
	bool		Disconnect();
	
	void		AddModbusEvent(word s, word c, byte* d, Var* v);

	void		Update();

	dword		GetAccessTimeout() { return accessTimeout; }
	void		SetAccessTimeout(dword timeout) { prevAccessTime -= timeout; }
//	dword		GetValidBoudRate(dword speed) { return comPort.GetValidBoudRate(speed); }
	void		SetAccessRights(byte acs) { acsRights = acs; }
	byte		GetAccessRights() { return acsRights; }

#ifdef WIN32

	bool		ConnectTCP(const char *adr, word port);
	void		UpdateTCP();

#endif

};

#endif // MODBUS__09_02_2007__08_38
