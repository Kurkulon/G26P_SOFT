#include <stdio.h>
#include <conio.h>

#include "ComPort.h"

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ComPort::Connect(byte port, dword speed, byte parity)
{
    char buf[256];

    wsprintf(buf, "\\\\.\\COM%u", port);

    return Connect(buf, speed, parity);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ComPort::Disconnect()
{
	if (!_connected) return false;

	_status485 = READ_END;

	_connected = false;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void near ComPort::EnableTransmit(void far* src, word count)
{
	_startTransmitTime = GetTickCount();

	_ovlWrite.hEvent = 0;
	_ovlWrite.Internal = 0;
	_ovlWrite.InternalHigh = 0;
	_ovlWrite.Offset = 0;
	_ovlWrite.OffsetHigh = 0;
	_ovlWrite.Pointer = 0;

	WriteFile(_comHandle, src, count, &_writeBytes, &_ovlWrite);
    SetCommMask(_comHandle, EV_TXEMPTY);

	_status485 = WRITEING;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


void ComPort::EnableReceive(void far* dst, word count)
{
	_startReceiveTime = GetTickCount();

	_ovlRead.hEvent = 0;
	_ovlRead.Internal = 0;
	_ovlRead.InternalHigh = 0;
	_ovlRead.Offset = 0;
	_ovlRead.OffsetHigh = 0;
	_ovlRead.Pointer = 0;

	ReadFile(_comHandle, dst, count, &_readBytes, &_ovlRead);

	_status485 = READING;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ComPort::Update()
{
//	static word p = 0;

	dword stamp;

	bool r = true;

	if (!_connected) 
	{
		_status485 = READ_END;
	};

	stamp = GetTickCount();

	switch (_status485)
	{
		case WRITE_DELAY:
			
			if ((stamp - _startTransmitTime) > 2)
			{
				EnableTransmit(_pWriteBuffer->data, _pWriteBuffer->len);	
			};

			break;

		case WRITEING:

			if (HasOverlappedIoCompleted(&_ovlWrite))
			{
				_pWriteBuffer->transmited = true;
				_status485 = READ_END;

				r = false;
			};

			break;

		case READING:

			if (HasOverlappedIoCompleted(&_ovlRead))
			{
				bool c = GetOverlappedResult(_comHandle, &_ovlRead, &_readBytes, false) != 0;
				_pReadBuffer->len = (word)_readBytes;
				_pReadBuffer->recieved = _pReadBuffer->len > 0 && c;
				_status485 = READ_END;
				r = false;
			};

			break;

		case READ_END:

			r = false;

			break;
	};

	return r;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ComPort::Read(ComPort::ReadBuffer *readBuffer, dword preTimeout, dword postTimeout)
{
	if (_status485 != READ_END || readBuffer == 0)
	{
		return false;
	};

    _cto.ReadIntervalTimeout = postTimeout;
	_cto.ReadTotalTimeoutConstant = preTimeout;

    if (!SetCommTimeouts(_comHandle, &_cto))
    {
		return false;
	};

	_pReadBuffer = readBuffer;
	_pReadBuffer->recieved = false;
	_pReadBuffer->len = 0;

	EnableReceive(_pReadBuffer->data, _pReadBuffer->maxLen);

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ComPort::Write(ComPort::WriteBuffer *writeBuffer)
{
	if (_status485 != READ_END || writeBuffer == 0)
	{
		return false;
	};

	_pWriteBuffer = writeBuffer;
	_pWriteBuffer->transmited = false;

	EnableTransmit(_pWriteBuffer->data, _pWriteBuffer->len);

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int  ComPort::_portTableSize = 0;
dword ComPort::_portTable[256];

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ComPort::Connect(const char* comPort, dword bps, byte parity)
{
    if (_comHandle != INVALID_HANDLE_VALUE)
    {
        return false;
    };

	_comHandle = CreateFile(comPort, GENERIC_READ|GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);

    if (_comHandle == INVALID_HANDLE_VALUE)
    {
		return false;
    };

    DCB dcb;

    if (!GetCommState(_comHandle, &dcb))
    {
        Disconnect();
		return false;
    };

    dcb.DCBlength = sizeof(dcb);
    dcb.BaudRate = bps;
    dcb.ByteSize = 8;
    dcb.Parity = parity;
    dcb.fParity = (parity > 0);
    dcb.StopBits = ONESTOPBIT;
    dcb.fBinary = true;
    dcb.fDtrControl = DTR_CONTROL_DISABLE;
    dcb.fRtsControl = RTS_CONTROL_DISABLE;
	dcb.fOutxCtsFlow = false;
	dcb.fOutxDsrFlow = false;
    dcb.fDsrSensitivity = false;

	if (!SetCommState(_comHandle, &dcb))
    {
        Disconnect();
		return false;
    };

    _cto.ReadIntervalTimeout = 2;
	_cto.ReadTotalTimeoutMultiplier = 0;
	_cto.ReadTotalTimeoutConstant = -1;
	_cto.WriteTotalTimeoutConstant = 0;
	_cto.WriteTotalTimeoutMultiplier = 0;

    if (!SetCommTimeouts(_comHandle, &_cto))
    {
        Disconnect();
    	return false;
    };

	return _connected = true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ComPort::BuildPortTable()
{
    if (_portTableSize != 0)
    {
        return;
    };

    char buf[256];

    int k = 0;

	HANDLE h;

    for (u32 i = 1; i < 256; i++)
    {
        wsprintf(buf, "\\\\.\\COM%u", i);

		h = CreateFile(buf, GENERIC_READ|GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);

        if (h != INVALID_HANDLE_VALUE)
        {
            _portTable[k++] = i;

			CloseHandle(h);
        };

    };

    _portTableSize = k;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

