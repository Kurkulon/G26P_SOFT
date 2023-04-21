#ifndef COMPORT__31_01_2007__15_32
#define COMPORT__31_01_2007__15_32

#include "types.h"


class ComPort
{
  public:

	struct ReadBuffer				
	{								
		bool	recieved;			
		word	len;				
		word	maxLen;				
		void*	data;				
	};								
									
	struct WriteBuffer				
	{								
		bool	transmited;			
		word	len;				
		void*	data;				
	};								

  protected:

	enum STATUS485 { WRITE_DELAY = 1, WRITEING = 2, READING = 3, READ_END = 4 };

	bool		_connected;


	byte		_status485;
	byte		_portNum;

	dword		_startTransmitTime;
	dword		_startReceiveTime;

	dword		_preReadTimeout;
	dword		_postReadTimeout;

	ReadBuffer*		_pReadBuffer;
	WriteBuffer*	_pWriteBuffer;

	dword		_readBytes;
	dword		_writeBytes;

	HANDLE		_comHandle;
	OVERLAPPED	_ovlRead;
	OVERLAPPED	_ovlWrite;
	COMMTIMEOUTS _cto;

	static int		_portTableSize;
	static dword	_portTable[256];


	void near	EnableTransmit(void far* src, word count);
	void near	EnableReceive(void far* dst, word count);



  public:

	ComPort() : _connected(false), _status485(READ_END), _comHandle(INVALID_HANDLE_VALUE) {}
	bool		Connect(const char* comPort, dword bps, byte parity);

	bool		Connect(byte port, dword speed, byte parity);
	bool		Disconnect();
	bool		Update();

	bool		Read(ComPort::ReadBuffer *readBuffer, dword preTimeout, dword postTimeout);
	bool		Write(ComPort::WriteBuffer *writeBuffer);

	void		BuildPortTable();

	const u32*	GetPortTable() { return _portTable; }
	u32			GetPortTableSize() { return _portTableSize; }



};

#endif // COMPORT__31_01_2007__15_32


