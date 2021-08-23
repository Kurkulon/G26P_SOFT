#ifndef COMPORT__31_01_2007__15_32
#define COMPORT__31_01_2007__15_32

#include "types.h"
#include "core.h"

#ifndef WIN32
#ifdef CPU_XMC48
	#include "COM_DEF.h"
#endif
#else
#ifndef BUSY
#define BUSY 0
#endif
#endif

#define COM_RS232 0
#define COM_RS485 1

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

	#ifdef CPU_XMC48

	struct LLI
	{
		volatile void*	SAR;
		volatile void*	DAR;
		LLI*	LLP;
		u32		CTLL;
		u32		CTLH;
		u32		DSTAT;
	};

	#endif


  protected:

	enum STATUS485 { WRITEING = 0, WAIT_READ = 1, READING = 2, READ_END = 3 };

//	union CUSART { void *vp; T_HW::S_UART *DU; T_HW::S_USART *SU; };

	#ifdef CPU_SAME53

		struct ComBase
		{
			bool used;
			T_HW::S_USART* const HU;
			T_HW::S_PORT* const pm;
			const dword pinRTS;
		};

		T_HW::S_PORT	*_pm;
		T_HW::S_USART 	*_SU;

	#elif defined(CPU_XMC48)

		struct ComBase
		{
			bool used;
			const byte dsel;
			T_HW::USIC_CH_Type* const HU;
			T_HW::PORT_Type* const pm;
			const dword pinRTS;
			const u32 usic_pid;
			T_HW::GPDMA_Type* const dma;
			const u32 dmaCh;
		};

		LLI					_lli[4];

		T_HW::PORT_Type		*_pm;
		T_HW::USIC_CH_Type 	*_SU;
		T_HW::GPDMA_Type	*_dma;
		T_HW::GPDMA_CH_Type	*_chdma;
		u32					_dmaChMask;
		u32					_dlr;

		bool IsTransmited() { return (_SU->PSR & BUSY) == 0 && !(_dma->CHENREG & _dmaChMask); }

	#endif


	static ComBase _bases[2];

	bool			_connected;
	byte			_status485;
	byte			_portNum;

	u32				_startDmaCounter;
	u32				_prevDmaCounter;

	ReadBuffer		*_pReadBuffer;
	WriteBuffer		*_pWriteBuffer;



	word			_BaudRateRegister;

	dword			_ModeRegister;
	dword			_pinRTS;
	dword			_startTransmitTime;
	dword			_startReceiveTime;
//	dword			_preReadTimeout;
	dword			_postReadTimeout;
	dword			_readTimeout;


	void 		EnableTransmit(void* src, word count);
	void 		DisableTransmit();
	void 		EnableReceive(void* dst, word count);
	void 		DisableReceive();

	//static		bool _InitCom(byte i, ComPort* cp);
	//static		bool _DeinitCom(byte i, ComPort* cp);

	static ComPort *_objCom1;
	static ComPort *_objCom2;
	static ComPort *_objCom3;

	word 		BoudToPresc(dword speed);


  public:
	  
	ComPort() : _connected(false), _status485(READ_END) {}


	bool		ConnectAsyn(byte port, dword speed, byte parity, byte stopBits);
	bool		ConnectSync(byte port, dword speed, byte parity, byte stopBits);
	bool		Disconnect();
	bool		Update();

	bool		Read(ComPort::ReadBuffer *readBuffer, dword preTimeout, dword postTimeout);
	bool		Write(ComPort::WriteBuffer *writeBuffer);

	void		TransmitByte(byte v);

	static __irq void _IntHandlerCom1();
	static __irq void _IntHandlerCom2();
	static __irq void _IntHandlerCom3();

};

#endif // COMPORT__31_01_2007__15_32
