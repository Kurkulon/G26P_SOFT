#pragma O3
#pragma Otime

#ifdef WIN32
#include "COM_DEF.h"
#endif
//#include <stdio.h>
//#include <conio.h>

#include "ComPort.h"
#include "time.h"
//#include "hardware.h"

#ifdef _DEBUG_
//	static const bool _debug = true;
#else
//	static const bool _debug = false;
#endif

extern dword millisecondsCount;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef CPU_SAME53	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	ComPort::ComBase	ComPort::_bases[2] = { 
		{false, HW::USART0,	 HW::PIOA, 1<<4 }, 
		{false, HW::USART1,	 HW::PIOA, 1<<27 }
	};

#elif defined(CPU_XMC48)	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	#define __SCTR (PDL(1) | TRM(1) | FLE(7) | WLE(7))

	#define __CCR (MODE(2))

	#define __BRG_ASYN	(DCTQ(15))
	#define __BRG_SYNC	(DCTQ(0)|CLKSEL(3))

	//#define __DX0CR (DSEL(0) | INSW(0) | DFEN(0) | DSEN(0) | DPOL(0) | SFSEL(0) | CM(0) | DXS(0))
	//#define __DX1CR (DSEL(0) | INSW(0) | DFEN(0) | DSEN(0) | DPOL(0) | SFSEL(0) | CM(0) | DXS(0))
	//#define __DX2CR (DSEL(0) | INSW(0) | DFEN(0) | DSEN(0) | DPOL(0) | SFSEL(0) | CM(0) | DXS(0))
	//#define __DX3CR (DSEL(0) | INSW(0) | DFEN(0) | DSEN(0) | DPOL(0) | SFSEL(0) | CM(0) | DXS(0))

	#define __PCR_ASYN	(SMD(1) | SP(9) | RSTEN(1) | TSTEN(1))
	#define __PCR_SYNC	(SMD(0) | SP(0) | RSTEN(1) | TSTEN(1))

//	#define __FDR (STEP(0x3FF) | DM(1))

	#define __TCSR (TDEN(1)|TDSSM(1))

	ComPort::ComBase	ComPort::_bases[2] = { 
		{false, 1, HW::USIC0_CH0,	 HW::P0, (1<<11), PID_USIC0, HW::DMA0, 0 }, 
		{false, 2, HW::USIC1_CH0,	 HW::P0, (1<<12), PID_USIC1, HW::DMA0, 1 }
	};

	static u32 parityMask[3] = { PM(0), PM(3), PM(2) };

#endif	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ComPort::ConnectAsyn(byte port, dword speed, byte parity, byte stopBits)
{
	if (_connected || port > 3 || _bases[port].used)
	{
		return false;
	};

	_portNum = port;

	ComBase &cb = _bases[port];

	_SU = cb.HU;
	_pm = cb.pm;
	_pinRTS = cb.pinRTS;

	#ifdef CPU_SAME53	

		_BaudRateRegister = BoudToPresc(speed);

		_ModeRegister = 0xC0;

		switch (parity)
		{
			case 0:		// нет четности
				_ModeRegister |= 0x800;
				break;

			case 1:
				_ModeRegister |= 0x200;
				break;

			case 2:
				_ModeRegister |= 0x000;
				break;
		};

		//_SU->CR = 3;
		//_SU->MR = _ModeRegister;
		//_SU->BRGR = _BaudRateRegister;

	#elif defined(CPU_XMC48)

		_dma = cb.dma;
		_dmaChMask = 1<<cb.dmaCh;
		_chdma = &(_dma->CH[cb.dmaCh]);
		_dlr = cb.dmaCh;

		_BaudRateRegister = BoudToPresc(speed) | DM(1);

		_ModeRegister = __CCR | ((parity < 3) ? parityMask[parity] : parityMask[0]);

		HW::Peripheral_Enable(cb.usic_pid);

		_SU->KSCFG = MODEN|BPMODEN|BPNOM|NOMCFG(0);

		_SU->FDR = _BaudRateRegister;
		_SU->BRG = __BRG_ASYN;
		_SU->PCR_ASCMode = __PCR_ASYN | ((stopBits == 2) ? STPB(1) : 0);;
		_SU->SCTR = __SCTR;
		_SU->DX0CR = DSEL(cb.dsel);//__DX0CR;

		_SU->TCSR = __TCSR;
		_SU->PSCR = ~0;

		_SU->CCR = _ModeRegister;

	#endif

	_status485 = READ_END;

	return _connected = cb.used = true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ComPort::ConnectSync(byte port, dword speed, byte parity, byte stopBits)
{
	if (_connected || port > 3 || _bases[port].used)
	{
		return false;
	};

	_portNum = port;

	ComBase &cb = _bases[port];

	_SU = cb.HU;
	_pm = cb.pm;
	_pinRTS = cb.pinRTS;

	#ifdef CPU_SAME53	


	#elif defined(CPU_XMC48)

		_dma = cb.dma;
		_dmaChMask = 1<<cb.dmaCh;
		_chdma = &(_dma->CH[cb.dmaCh]);
		_dlr = cb.dmaCh;

		//_BaudRateRegister = BoudToPresc(speed) | DM(1);

		_ModeRegister = __CCR | ((parity < 3) ? parityMask[parity] : parityMask[0]);

		HW::Peripheral_Enable(cb.usic_pid);

		_SU->KSCFG = MODEN|BPMODEN|BPNOM|NOMCFG(0);

	//	_SU->FDR			= _BaudRateRegister;
		_SU->BRG			= __BRG_SYNC;
		_SU->PCR_ASCMode	= __PCR_SYNC | ((stopBits == 2) ? STPB(1) : 0);
		_SU->SCTR			= __SCTR;
		_SU->DX0CR			= DSEL(cb.dsel);//__DX0CR;
		_SU->DX1CR			= DPOL(1);
		_SU->TCSR			= __TCSR;
		_SU->PSCR			= ~0;
		_SU->CCR			= _ModeRegister;

	#endif

	_status485 = READ_END;

	return _connected = cb.used = true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ComPort::Disconnect()
{
	if (!_connected) return false;

	DisableReceive();
	DisableTransmit();

	_status485 = READ_END;

	_connected = _bases[_portNum].used = false;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

word ComPort::BoudToPresc(dword speed)
{
	if (speed == 0) return 0;

	word presc;

	#ifdef CPU_SAME53	

		presc = (((MCK+8)/16) + speed/2) / speed;

		return presc;

	#elif defined(CPU_XMC48)

		presc = ((MCK + speed/2) / speed + 8) / 16;

		if (presc > 1024) presc = 1024;

		return 1024 - presc;

	#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ComPort::TransmitByte(byte v)
{
	#ifdef CPU_SAME53	

		//_SU->CR = 0xA0;	// Disable transmit and receive

		//_pm->SODR = _maskRTS;

		//_SU->CR = 0x40;

		//_SU->THR = v;

		//while ((_SU->CSR & 0x200) == 0);

		//_pm->CODR = _maskRTS;

		//_SU->CR = 0xA0;	// Disable transmit and receive

	#elif defined(CPU_XMC48)

		_pm->SET(_pinRTS);

		_SU->CCR = _ModeRegister|TBIEN;
		_SU->INPR = 0;

		_SU->TBUF[0] = v;

		while ((_SU->PSR & TSIF) == 0);

		_pm->CLR(_pinRTS);

		_SU->CCR = _ModeRegister;	// Disable transmit and receive

	#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ComPort::EnableTransmit(void* src, word count)
{
	#ifdef CPU_SAME53	
		//_SU->CR = 0xA0;	// Disable transmit and receive

		//_pm->SODR = _maskRTS;

		//_SU->PDC.TPR = src;
		//_SU->PDC.TCR = count;

		//_SU->PDC.PTCR = 0x100;

		//_startTransmitTime = GetRTT();
		//_SU->CR = 0x40;
	#elif defined(CPU_XMC48)

		if (count == 0) return;

	//	count -= 1;

		__disable_irq();

		HW::DLR->LNEN |= (1<<_dlr);

		_pm->SET(_pinRTS);

		_dma->DMACFGREG = 1;

		if (count > 0xFFF)
		{
			byte *p = (byte*)src;

			u32 i = 0;

			LLI *lli = &_lli[0];

			for ( ; i < ArraySize(_lli); i++)
			{
				lli = _lli+i;

				lli->SAR = src;
				lli->CTLL = DINC(2)|SINC(0)|TT_FC(1)|DEST_MSIZE(0)|SRC_MSIZE(0);
				lli->DAR = &_SU->TBUF[0];

				if (count > 0xFFF)
				{
					lli->LLP = _lli+i+1;
					lli->CTLH = 0xFFF;
					lli->CTLL |= LLP_DST_EN;
					count -= 0xFFF;
					p += 0xFFF;
				}
				else
				{
					lli->LLP = 0;
					lli->CTLH = count;
					count = 0;

					break;
				};
			};

			lli->LLP = 0;

			_dma->DMACFGREG = 1;

			_chdma->CTLH = _lli[0].CTLH;
			_chdma->CTLL = _lli[0].CTLL;
			_chdma->LLP = (u32)&_lli[0];
		}
		else
		{
			_chdma->CTLL = DINC(2)|SINC(0)|TT_FC(1)|DEST_MSIZE(0)|SRC_MSIZE(0);
			_chdma->CTLH = BLOCK_TS(count);
		};

		_chdma->SAR = (u32)src;
		_chdma->DAR = (u32)&_SU->TBUF[0];
		_chdma->CFGL = HS_SEL_SRC;
		_chdma->CFGH = PROTCTL(1)|DEST_PER(_dlr);

		_dma->CHENREG = _dmaChMask|(_dmaChMask<<8);

		_SU->PSCR = ~0;
		_SU->CCR = _ModeRegister|TBIEN;
		_SU->INPR = 0;

		if ((_SU->PSR & TBIF) == 0)
		{
			_SU->FMR = USIC_CH_FMR_SIO0_Msk;
		};

		__enable_irq();

	#endif

	_status485 = WRITEING;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ComPort::DisableTransmit()
{
	#ifdef CPU_SAME53	

		//_SU->CR = 0x80;
		//_SU->PDC.PTCR = 0x200;
		//_pm->CODR = _maskRTS;

	#elif defined(CPU_XMC48)

	//	_dma->CLEARBLOCK = _dmaChMask;
		_dma->CHENREG = _dmaChMask<<8;
		_SU->CCR = _ModeRegister;
		_pm->CLR(_pinRTS);
		HW::DLR->LNEN &= ~(1<<_dlr);
	//	_SU->KSCFG = BPNOM|NOMCFG(3);

	#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ComPort::EnableReceive(void* dst, word count)
{
	volatile u32 t;

	#ifdef CPU_SAME53	

		//_SU->CR = 0x1A0;	// Disable transmit and receive, reset status

		//_pm->CODR = _maskRTS;

		//_SU->PDC.RPR = dst;
		//_SU->PDC.RCR = count;

		//_SU->PDC.PTCR = 1;

		//_startReceiveTime = GetRTT();
		//_SU->CR = 0x110;

	#elif defined(CPU_XMC48)

		__disable_irq();

		_pm->CLR(_pinRTS);

		_dma->DMACFGREG = 1;

		if (count > 0xFFF)
		{
			byte *p = (byte*)dst;

			u32 i = 0;

			LLI *lli = &_lli[0];

			for ( ; i < ArraySize(_lli); i++)
			{
				lli = _lli+i;

				lli->SAR = &_SU->RBUF;
				lli->CTLL = DINC(0)|SINC(2)|TT_FC(2)|DEST_MSIZE(0)|SRC_MSIZE(0);
				lli->DAR = p;

				if (count > 0xFFF)
				{
					lli->LLP = _lli+i+1;
					lli->CTLH = 0xFFF;
					lli->CTLL |= LLP_SRC_EN;
					count -= 0xFFF;
					p += 0xFFF;
				}
				else
				{
					lli->LLP = 0;
					lli->CTLH = count;
					count = 0;

					break;
				};
			};

			lli->LLP = 0;

			_dma->DMACFGREG = 1;

			_chdma->CTLH = _lli[0].CTLH;
			_chdma->CTLL = _lli[0].CTLL;
			_chdma->LLP = (u32)&_lli[0];
		}
		else
		{
			_chdma->CTLH = BLOCK_TS(count);
			_chdma->CTLL = DINC(0)|SINC(2)|TT_FC(2)|DEST_MSIZE(0)|SRC_MSIZE(0);
		};

		_chdma->SAR = (u32)&_SU->RBUF;
		_chdma->DAR = _startDmaCounter = _prevDmaCounter = (u32)dst;
		_chdma->CFGL = HS_SEL_DST;
		_chdma->CFGH = PROTCTL(1)|SRC_PER(_dlr);
		_dma->CHENREG = _dmaChMask|(_dmaChMask<<8);

		_startReceiveTime = GetRTT();

		t = _SU->RBUF;
		t = _SU->RBUF;
		_SU->PSCR = ~0;

		HW::DLR->LNEN |= (1<<_dlr);

		_SU->CCR = _ModeRegister|RIEN;
		_SU->INPR = 0;

		__enable_irq();

	#endif

	_status485 = WAIT_READ;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ComPort::DisableReceive()
{
	#ifdef CPU_SAME53	

		//_SU->CR = 0x120;
		//_SU->PDC.PTCR = 2;
		//_pm->CODR = _maskRTS;

	#elif defined(CPU_XMC48)

		_dma->CHENREG = _dmaChMask<<8;
		_SU->CCR = _ModeRegister;
		_pm->CLR(_pinRTS);
		HW::DLR->LNEN &= ~(1<<_dlr);
	//	_SU->KSCFG = BPNOM|NOMCFG(3);

	#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	#ifdef CPU_SAME53	
	#elif defined(CPU_XMC48)
	#endif

bool ComPort::Update()
{
//	static u32 stamp = 0;

	bool r = true;

	if (!_connected) 
	{
		_status485 = READ_END;
	};

//	stamp = GetRTT();

	switch (_status485)
	{
		case WRITEING: //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			
			if (IsTransmited())
			{
				_pWriteBuffer->transmited = true;
				_status485 = READ_END;

				DisableTransmit();

				r = false;
			};

			break;

		case WAIT_READ: //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		{
			u32 t = _chdma->DAR;

			u32 stamp = GetRTT();

			if (_prevDmaCounter == t)
			{
				if ((u16)(stamp - _startReceiveTime) >= _readTimeout)
				{
					DisableReceive();
					_pReadBuffer->len = t - _startDmaCounter;
					_pReadBuffer->recieved = _pReadBuffer->len > 0;
					_status485 = READ_END;
					r = false;
				};
			}
			else
			{
				_prevDmaCounter = t;
				_startReceiveTime = stamp;
				_readTimeout = _postReadTimeout;
			};
		};

		break;

		case READ_END: //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

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

	_readTimeout = preTimeout;
	_postReadTimeout = postTimeout;

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

