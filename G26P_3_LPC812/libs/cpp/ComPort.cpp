#include <stdio.h>
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

ComPort::ComBase	ComPort::_bases[3] = { 
	{false, HW::USART0,	 1<<0,	0, 0, 0, 0, ReadHandler_0, WriteHandler_0, 3	}, 
	{false, HW::USART1,	 0,		0, 0, 0, 0, ReadHandler_1, WriteHandler_1, 4	},
	{false, HW::USART2,	 0,		0, 0, 0, 0, ReadHandler_2, WriteHandler_2, 5	}
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ComPort::Connect(byte port, dword speed, byte parity)
{
	if (_connected || port > 2 || _bases[port].used)
	{
		return false;
	};

	_portNum = port;

	ComBase &cb = _bases[port];

	_usart = cb.usart;
	_maskRTS = cb.maskRTS;

	_BaudRateRegister = BoudToPresc(speed);

	_ModeRegister = 0x04;

	switch (parity)
	{
		case 0:		// нет четности
			_ModeRegister |= 0<<4;
			break;

		case 1:
			_ModeRegister |= 3<<4;
			break;

		case 2:
			_ModeRegister |= 2<<4;
			break;
	};

	_usart->CFG = _ModeRegister;
	_usart->BRG = _BaudRateRegister;

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

	return (MCK/8+speed) / 2 / speed - 1;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ComPort::EnableTransmit(void* src, word count)
{
	_usart->CFG &= ~1;	// Disable transmit and receive

	HW::GPIO->SET0 = _maskRTS;

	ComBase &cb = _bases[_portNum];

	cb.wdata = (byte*)src;
	cb.wlen = count;

	VectorTableExt[cb.ivect] = cb.iw;

	CM0::NVIC->ISER[0] = 1<<cb.ivect;
	
	_usart->INTENSET = 4;

	_usart->CFG |= 1;

	_status485 = WRITEING;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ComPort::DisableTransmit()
{
	_usart->CFG &= ~1;	// Disable transmit and receive
	HW::GPIO->CLR0 = _maskRTS;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ComPort::EnableReceive(void* dst, word count)
{
	_usart->CFG &= ~1;	// Disable transmit and receive

	HW::GPIO->CLR0 = _maskRTS;

	ComBase &cb = _bases[_portNum];

	cb.rdata = (byte*)dst;
	cb.rlen = count;

	VectorTableExt[cb.ivect] = cb.ir;

	CM0::NVIC->ISER[0] = 1<<cb.ivect;

	_usart->INTENSET = 1;

	_usart->CFG |= 1;


	_status485 = WAIT_READ;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void ComPort::DisableReceive()
{
	_usart->CFG &= ~1;	// Disable transmit and receive
	HW::GPIO->CLR0 = _maskRTS;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__irq void ComPort::ReadHandler_0()
{
	ComBase &cb = _bases[0];
	
	*(cb.rdata++) = cb.usart->RXDATA;
	cb.rlen--;
	
	if (cb.rlen == 0)
	{
		cb.usart->INTENCLR = 1;
	};

	HW::MRT->Channel[0].INTVAL = cb.tm;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__irq void ComPort::ReadHandler_1()
{
	ComBase &cb = _bases[1];
	
	*(cb.rdata++) = cb.usart->RXDATA;
	cb.rlen--;
	
	if (cb.rlen == 0)
	{
		cb.usart->INTENCLR = 1;
	};

	HW::MRT->Channel[1].INTVAL = cb.tm;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__irq void ComPort::ReadHandler_2()
{
	ComBase &cb = _bases[2];
	
	*(cb.rdata++) = cb.usart->RXDATA;
	cb.rlen--;
	
	if (cb.rlen == 0)
	{
		cb.usart->INTENCLR = 1;
	};

	HW::MRT->Channel[2].INTVAL = cb.tm;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__irq void ComPort::WriteHandler_0()
{
	ComBase &cb = _bases[0];
	
	cb.usart->TXDATA = *(cb.wdata++);
	cb.wlen--;
	
	if (cb.wlen == 0)
	{
		cb.usart->INTENCLR = 4;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__irq void ComPort::WriteHandler_1()
{
	ComBase &cb = _bases[1];
	
	cb.usart->TXDATA = *(cb.wdata++);
	cb.wlen--;
	
	if (cb.wlen == 0)
	{
		cb.usart->INTENCLR = 4;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__irq void ComPort::WriteHandler_2()
{
	ComBase &cb = _bases[2];
	
	cb.usart->TXDATA = *(cb.wdata++);
	cb.wlen--;
	
	if (cb.wlen == 0)
	{
		cb.usart->INTENCLR = 4;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool ComPort::Update()
{
	bool r = true;

	if (!_connected) 
	{
		_status485 = READ_END;
	};

	ComBase &cb = _bases[_portNum];

	switch (_status485)
	{
		case WRITEING:

			if (_usart->STAT & 8)
			{
				_pWriteBuffer->transmited = true;
				_status485 = READ_END;

				DisableTransmit();

				r = false;
			};

			break;

		case WAIT_READ:

			if (HW::MRT->Channel[_portNum].STAT & 1)
			{
				DisableReceive();
				
				HW::MRT->Channel[_portNum].INTVAL = (1UL<<31);
				HW::MRT->Channel[_portNum].STAT = 1;

				_pReadBuffer->len = _pReadBuffer->maxLen - cb.rlen;
				_pReadBuffer->recieved = _pReadBuffer->len > 0;
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
//		cputs("ComPort::Read falure\n\r");	
		return false;
	};


	_pReadBuffer = readBuffer;
	_pReadBuffer->recieved = false;
	_pReadBuffer->len = 0;

	_bases[_portNum].tm = (1UL<<31)|postTimeout;

	HW::MRT->Channel[_portNum].CTRL = 2;
	HW::MRT->Channel[_portNum].INTVAL = (1UL<<31)|(preTimeout); 
	HW::MRT->Channel[_portNum].STAT = 1;

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


