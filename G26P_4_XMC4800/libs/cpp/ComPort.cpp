#pragma O3
#pragma Otime

//#include <stdio.h>
//#include <conio.h>

#include "ComPort.h"
#include "time.h"

#ifdef _DEBUG_
//	static const bool _debug = true;
#else
//	static const bool _debug = false;
#endif

#define __SCTR (PDL(1) | TRM(1) | FLE(7) | WLE(7))

#define __CCR (MODE(2))

#define __BRG (DCTQ(15))

#define __DX0CR (DSEL(0) | INSW(0) | DFEN(0) | DSEN(0) | DPOL(0) | SFSEL(0) | CM(0) | DXS(0))
#define __DX1CR (DSEL(0) | INSW(0) | DFEN(0) | DSEN(0) | DPOL(0) | SFSEL(0) | CM(0) | DXS(0))
#define __DX2CR (DSEL(0) | INSW(0) | DFEN(0) | DSEN(0) | DPOL(0) | SFSEL(0) | CM(0) | DXS(0))
#define __DX3CR (DSEL(0) | INSW(0) | DFEN(0) | DSEN(0) | DPOL(0) | SFSEL(0) | CM(0) | DXS(0))

#define __PCR (SMD(1) | SP(8) | RSTEN(1) | TSTEN(1))

#define __FDR (STEP(0x3FF) | DM(1))

#define __TCSR (TDEN(1)|TDSSM(1))



//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ComPort::ComBase	ComPort::_bases[2] = { 
	{false, HW::USIC0_CH0,	 HW::P0, 11, 0, SCU_CLK_CGATSTAT0_USIC0_Msk, HW::DMA0, 0 }, 
	{false, HW::USIC1_CH0,	 HW::P0, 12, 1, SCU_CLK_CGATSTAT1_USIC1_Msk, HW::DMA0, 1 }
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ComPort::Connect(byte port, dword speed, byte parity)
{
#ifndef WIN32

	if (_connected || port > 1 || _bases[port].used)
	{
		return false;
	};

	_portNum = port;

	ComBase &cb = _bases[port];

	_SU = cb.HU;
	_pm = cb.pm;
	_pinRTS = cb.pinRTS;
	_dma = cb.dma;
	_dmaChMask = 1<<cb.dmaCh;
	_chdma = &(_dma->CH[cb.dmaCh]);
	_dlr = cb.dmaCh;


	_FDR = BoudToPresc(speed) | DM(1);

	_ModeRegister = __CCR;

	switch (parity)
	{
		case 0:		// нет четности
			_ModeRegister = __CCR | PM(0);
			break;

		case 1:
			_ModeRegister = __CCR | PM(3);
			break;

		case 2:
			_ModeRegister = __CCR | PM(2);
			break;
	};

	((u32*)&HW::SCU_CLK->CGATCLR0)[cb.gateIndex*3] = cb.gateMask;
	((u32*)&HW::SCU_RESET->PRCLR0)[cb.gateIndex*3] = cb.gateMask;

	_SU->KSCFG = MODEN|BPMODEN|BPNOM|NOMCFG(3);

	_SU->FDR = _FDR;
	_SU->BRG = __BRG;
	_SU->PCR_ASCMode = __PCR;
	_SU->SCTR = __SCTR;
	_SU->DX0CR = __DX0CR;

	_SU->TCSR = __TCSR;
	_SU->PSCR = ~0;

	_SU->CCR = _ModeRegister;

	_status485 = READ_END;

	return _connected = cb.used = true;

#else

    char buf[256];

    wsprintf(buf, "COM%u", port);

    return Connect(buf, speed, parity);

#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ComPort::Disconnect()
{
	if (!_connected) return false;

	DisableReceive();
	DisableTransmit();

	_status485 = READ_END;

#ifndef WIN32
	_connected = _bases[_portNum].used = false;
#else
	_connected = false;
#endif

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifndef WIN32

word ComPort::BoudToPresc(dword speed)
{
	if (speed == 0) return 0;

	word presc;

	presc = (word)((MCK*0.0625) / speed + 0.5);

	if (presc > 1024) presc = 1024;

	return 1024 - presc;
}

#endif

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ComPort::TransmitByte(byte v)
{
#ifndef WIN32

	_SU->CCR = _ModeRegister;	// Disable transmit and receive

	_pm->BSET(_pinRTS);

	_SU->KSCFG = BPNOM|NOMCFG(0);

	_SU->CCR = _ModeRegister;

	_SU->PSCR = TBIF|TSIF;

	_SU->TBUF[0] = v;

	while ((_SU->PSR & TSIF) == 0);

	_SU->KSCFG = BPNOM|NOMCFG(3);

	while (_SU->PSR & TXIDLE);

	_pm->BCLR(_pinRTS);

	_SU->CCR = _ModeRegister;	// Disable transmit and receive

#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ComPort::EnableTransmit(void* src, word count)
{
#ifndef WIN32

	HW::P5->BTGL(9);

	_SU->CCR = _ModeRegister|TBIEN;	// Disable transmit and receive

	_pm->BSET(_pinRTS);

	_SU->KSCFG = BPNOM|NOMCFG(0);

	_SU->PSCR = TBIF|TSIF|TFF;

	_SU->INPR = 0;

	_dma->DMACFGREG = 1;

	_chdma->CTLL = DINC(2)|SINC(2)|TT_FC(1)|DEST_MSIZE(0)|SRC_MSIZE(0);
	_chdma->CTLH = BLOCK_TS(count);

	_chdma->SAR = (u32)src;
	_chdma->DAR = (u32)&_SU->TBUF[0];
	_chdma->CFGL = HS_SEL_SRC;
	_chdma->CFGH = PROTCTL(1)|DEST_PER(_dlr);

	_dma->CHENREG = _dmaChMask|(_dmaChMask<<8);

	_SU->FMR = USIC_CH_FMR_SIO0_Msk;


/*	_SU->CCR = 0;	// Disable transmit and receive

	_pm->BSET(_pinRTS);

	_SU->PSCR = TBIF|TSIF;

	_dma->DMACFGREG = 1;

	_chdma->SAR = (u32)src;
	_chdma->DAR = (u32)&_SU->TBUF[0];
	_chdma->LLP = 0;
	_chdma->CTLH = BLOCK_TS(count);
	_chdma->CTLL = TT_FC(1)|DINC(2)|SINC(0)|DEST_MSIZE(0)|SRC_MSIZE(8);
	_chdma->CFGH = PROTCTL(1)|SRC_PER(1)|DEST_PER(1);
	_chdma->CFGL = HS_SEL_SRC;

	_SU->INPR = 0;

	_SU->CCR = _ModeRegister|TBIEN;	// Disable transmit and receive

	_SU->KSCFG = BPNOM|NOMCFG(0);

	_SU->FMR = USIC_CH_FMR_SIO0_Msk;

	_dma->CHENREG = _dmaChMask|(_dmaChMask<<8);

	_startTransmitTime = GetRTT();*/

#else

	_ovlWrite.hEvent = 0;
	_ovlWrite.Internal = 0;
	_ovlWrite.InternalHigh = 0;
	_ovlWrite.Offset = 0;
	_ovlWrite.OffsetHigh = 0;
	_ovlWrite.Pointer = 0;

	WriteFile(_comHandle, src, count, &_writeBytes, &_ovlWrite);
    SetCommMask(_comHandle, EV_TXEMPTY);

#endif

	_status485 = WRITEING;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ComPort::DisableTransmit()
{
#ifndef WIN32
	
	_SU->CCR = _ModeRegister;
	_pm->BCLR(_pinRTS);
//	_SU->KSCFG = BPNOM|NOMCFG(3);

#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ComPort::EnableReceive(void* dst, word count)
{
#ifndef WIN32

	_SU->CCR = 0;	// Disable transmit and receive, reset status

	_pm->BCLR(_pinRTS);

	_chdma->SAR = (u32)&_SU->RBUF0;
	_chdma->DAR = (u32)dst;
	_chdma->CTLH = BLOCK_TS(count);
	_chdma->CTLL = TT_FC(2);
	_chdma->CFGH = PROTCTL(1);
	_chdma->CFGL = 0;
	_dma->CHENREG = _dmaChMask|(_dmaChMask<<8);


	_startReceiveTime = GetRTT();
	_SU->CCR = _ModeRegister;

#else

	_ovlRead.hEvent = 0;
	_ovlRead.Internal = 0;
	_ovlRead.InternalHigh = 0;
	_ovlRead.Offset = 0;
	_ovlRead.OffsetHigh = 0;
	_ovlRead.Pointer = 0;

	ReadFile(_comHandle, dst, count, &_readBytes, &_ovlRead);

#endif

	_status485 = WAIT_READ;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ComPort::DisableReceive()
{
#ifndef WIN32

	_SU->CCR = _ModeRegister;
	_pm->BCLR(_pinRTS);
//	_SU->KSCFG = BPNOM|NOMCFG(3);

#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ComPort::Update()
{
	static u32 stamp = 0;

	bool r = true;

	if (!_connected) 
	{
		_status485 = READ_END;
	};

	stamp = GetRTT();

	switch (_status485)
	{
		case WRITEING:

			if (IsTransmited())
			{
				_pWriteBuffer->transmited = true;
				_status485 = READ_END;

				DisableTransmit();
//				DisableReceive();

				r = false;
			};

			break;

//		case WAIT_READ:
//
//#ifndef WIN32
//
//			if ((_prevDmaCounter-_SU->PDC.RCR) == 0)
//			{
//				if ((stamp - _startReceiveTime) >= _preReadTimeout)
//				{
//					DisableReceive();
//					_pReadBuffer->len = _pReadBuffer->maxLen - _prevDmaCounter;
//					_pReadBuffer->recieved = _pReadBuffer->len > 0;
//					_status485 = READ_END;
//					r = false;
//				};
//			}
//			else
//			{
//				_prevDmaCounter = _SU->PDC.RCR;
//				_startReceiveTime = stamp;
//				_status485 = READING;
//			};
//
//#else
//			if (HasOverlappedIoCompleted(&_ovlRead))
//			{
//				bool c = GetOverlappedResult(_comHandle, &_ovlRead, &_readBytes, false);
//				_pReadBuffer->len = _readBytes;
//				_pReadBuffer->recieved = _pReadBuffer->len > 0 && c;
//				_status485 = READ_END;
//				r = false;
//			};
//#endif
//
//			break;
//
//#ifndef WIN32
//
//		case READING:
//
//			//if (_SU->CSR & 0xE4) 
//			//{
//			//	DisableReceive();
//			//	_status485 = READ_END;
//			//	return false;
//			//};
//
//			if ((_prevDmaCounter-_SU->PDC.RCR) == 0)
//			{
//				if ((stamp - _startReceiveTime) >= _postReadTimeout)
//				{
//					DisableReceive();
//					_pReadBuffer->len = _pReadBuffer->maxLen - _prevDmaCounter;
//					_pReadBuffer->recieved = _pReadBuffer->len > 0;
//					_status485 = READ_END;
//					r = false;
//				};
//			}
//			else
//			{
//				_prevDmaCounter = _SU->PDC.RCR;
//				_startReceiveTime = stamp;
//			};
//
//			break;

//#endif

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
//		cputs("ComPort::Read falure\n\r");	
		return false;
	};

#ifndef WIN32

	_preReadTimeout = preTimeout;
	_postReadTimeout = postTimeout;

#else
    _cto.ReadIntervalTimeout = postTimeout/32+1;
	_cto.ReadTotalTimeoutConstant = preTimeout/32+1;

    if (!SetCommTimeouts(_comHandle, &_cto))
    {
		return false;
	};

#endif

	_pReadBuffer = readBuffer;
	_pReadBuffer->recieved = false;
	_pReadBuffer->len = 0;

	_prevDmaCounter = _pReadBuffer->maxLen;

	u32 t = (u32)_pReadBuffer->data;

	if (t < 0x20000000 || t >= 0x20028000)
	{
		__breakpoint(0);
	};

	EnableReceive(_pReadBuffer->data, _pReadBuffer->maxLen);

//	cputs("ComPort::Read start\n\r");	

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

#ifdef WIN32

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int  ComPort::_portTableSize = 0;
dword ComPort::_portTable[16];

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

	printf("Connect to %s speed = %u\r\n", comPort, bps);

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

    COMMCONFIG cc;
    DWORD lcc = sizeof(cc);

    int k = 0;

    for (int i = 1; i < 17; i++)
    {
        wsprintf(buf, "COM%i", i);

        lcc = sizeof(cc);

        if (GetDefaultCommConfig(buf, &cc, &lcc))
        {
            _portTable[k++] = i;
        };

    };

    _portTableSize = k;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif
