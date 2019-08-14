#include "emac.h"
#include "core.h"
#include "xtrap.h"
#include "flash.h"
#include "time.h"
#include "ComPort.h"
#include "twi.h"
#include "hardware.h"

#pragma diag_suppress 546,550,177

//#pragma O3
//#pragma Otime

byte buf[5000] = {0x55,0,0,0,0,0,0,0,0,0x55};

static ComPort com1;
//static TWI twi;

u32 fps = 0;
u32 f = 0;

ComPort::WriteBuffer wb = { .transmited = false, .len = 0, .data = buf };
ComPort::ReadBuffer rb = { .recieved = false, .len = 0, .maxLen = sizeof(buf), .data = buf };

static void CopyDataDMA(volatile void *src, volatile void *dst, u16 len);
static void Init_UART_DMA();
static void Send_UART_DMA();

static byte len = 1;

static u16 temp = 0;

u16 manRcvData[10];
u16 manTrmData[50];

u16 rcvBuf[10];

u32 readsFlash = 0;

static const u16 manReqWord = 0x3B00;
static const u16 manReqMask = 0xFF00;


static bool RequestMan(u16 *buf, u16 len, MTB* mtb);

static i16 temperature = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool ReqMan00(u16 *buf, u16 len, MTB* mtb)
{
	if (buf == 0 || len == 0 || len > 2 || mtb == 0) return false;

	manTrmData[0] = (manReqWord & manReqMask) | 0;
	manTrmData[1] = 0xEC00;
	manTrmData[2] = 0xEC00;

	mtb->data = manTrmData;
	mtb->len = 3;

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool ReqMan10(u16 *buf, u16 len, MTB* mtb)
{
	if (buf == 0 || len == 0 || len > 2 || mtb == 0) return false;

	manTrmData[0] = (manReqWord & manReqMask) | 0x10;

	mtb->data = manTrmData;
	mtb->len = 1;

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool ReqMan20(u16 *buf, u16 len, MTB* mtb)
{
	__packed struct Rsp {u16 rw; u16 device; u16 session; u32 rcvVec; u32 rejVec; u32 wrVec; u32 errVec; u16 wrAdr[3]; u16 temp; u16 status; RTC rtc; };

	if (buf == 0 || len == 0 || len > 2 || mtb == 0) return false;

	Rsp &rsp = *((Rsp*)&manTrmData);

	rsp.rw = (manReqWord & manReqMask) | 0x20;
	rsp.device = 0xEC00;  
	rsp.session = FLASH_Session_Get();	  
	rsp.rcvVec =  FLASH_Vectors_Recieved_Get();
	rsp.rejVec = FLASH_Vectors_Rejected_Get();
	rsp.wrVec = FLASH_Vectors_Saved_Get();
	rsp.errVec = FLASH_Vectors_Errors_Get();
	*((__packed u64*)rsp.wrAdr) = FLASH_Current_Adress_Get();
	rsp.temp = temp;
	rsp.status = FLASH_Status();

	GetTime(&rsp.rtc);

	mtb->data = manTrmData;
	mtb->len = 20;

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool ReqMan30(u16 *buf, u16 len, MTB* mtb)
{
	if (buf == 0 || len < 5 || len > 6 || mtb == 0) return false;

	manTrmData[0] = (manReqWord & manReqMask) | 0x30;

	SetTime(*(RTC*)(&buf[1]));

	mtb->data = manTrmData;
	mtb->len = 1;

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool ReqMan31(u16 *buf, u16 len, MTB* mtb)
{
	if (buf == 0 || len == 0 || len > 2 || mtb == 0) return false;

	manTrmData[0] = (manReqWord & manReqMask) | 0x31;

	mtb->data = manTrmData;
	mtb->len = 1;

	FLASH_WriteEnable();

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool ReqMan32(u16 *buf, u16 len, MTB* mtb)
{
	if (buf == 0 || len == 0 || len > 2 || mtb == 0) return false;

	manTrmData[0] = (manReqWord & manReqMask) | 0x32;

	mtb->data = manTrmData;
	mtb->len = 1;

	FLASH_WriteDisable();

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool ReqMan33(u16 *buf, u16 len, MTB* mtb)
{
	if (buf == 0 || len == 0 || len > 2 || mtb == 0) return false;

	manTrmData[0] = (manReqWord & manReqMask) | 0x33;

	mtb->data = manTrmData;
	mtb->len = 1;

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool ReqMan80(u16 *buf, u16 len, MTB* mtb)
{
	if (buf == 0 || len < 3 || len > 4 || mtb == 0) return false;

	manTrmData[0] = (manReqWord & manReqMask) | 0x80;

	mtb->data = manTrmData;
	mtb->len = 1;

	if (buf[1] == 2)
	{
		SetTrmBoudRate(buf[2]-1);
	};

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool ReqMan90(u16 *buf, u16 len, MTB* mtb)
{
	if (buf == 0 || len < 3 || len > 4 || mtb == 0) return false;

	manTrmData[0] = (manReqWord & manReqMask) | 0x90;

	mtb->data = manTrmData;
	mtb->len = 1;

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool ReqManF0(u16 *buf, u16 len, MTB* mtb)
{
	if (buf == 0 || len == 0 || len > 2 || mtb == 0) return false;

	SaveParams();

	manTrmData[0] = (manReqWord & manReqMask) | 0xF0;

	mtb->data = manTrmData;
	mtb->len = 1;

	return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestMan(u16 *buf, u16 len, MTB* mtb)
{
	if (buf == 0 || len == 0 || mtb == 0) return false;

	bool res = false;

	byte i = buf[0]&0xFF;

	switch (i)
	{
		case 0x00:	res = ReqMan00(buf, len, mtb); break;
		case 0x10:	res = ReqMan10(buf, len, mtb); break;
		case 0x20:	res = ReqMan20(buf, len, mtb); break;
		case 0x30:	res = ReqMan30(buf, len, mtb); break;
		case 0x31:	res = ReqMan31(buf, len, mtb); break;
		case 0x32:	res = ReqMan32(buf, len, mtb); break;
		case 0x33:	res = ReqMan33(buf, len, mtb); break;
		case 0x80:	res = ReqMan80(buf, len, mtb); break;
		case 0x90:	res = ReqMan90(buf, len, mtb); break;
		case 0xF0:	res = ReqManF0(buf, len, mtb); break;
	};

	return res;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateMan()
{
	static MTB mtb;
	static MRB mrb;

	static byte i = 0;

	static RTM16 tm;


//	u16 c;

	switch (i)
	{
		case 0:

			mrb.data = manRcvData;
			mrb.maxLen = 5;
			RcvManData(&mrb);

			i++;

			break;

		case 1:

			ManRcvUpdate();

			if (mrb.ready)
			{
				tm.Reset();

				if (mrb.OK && mrb.len > 0 && (manRcvData[0] & manReqMask) == manReqWord && RequestMan(manRcvData, mrb.len, &mtb))
				{
					i++;
				}
				else
				{
					i = 0;
				};
			}
			else if (mrb.len > 0)
			{
				//if ((manRcvData[0] & manReqMask) == manReqWord)
				//{
				//	byte i = manRcvData[0] & 0xFF;

				//	u16 l = 100;

				//	switch (i)
				//	{
				//		case 0x00:			
				//		case 0x10:			
				//		case 0x20:	l = 1;		break;

				//		case 0x30:	l = 5;		break;

				//		case 0x31:	
				//		case 0x32:	
				//		case 0x33:	l = 1;		break;

				//		case 0x80:	
				//		case 0x90:	l = 3;		break;

				//		case 0xF0:	l = 1;		break;
				//	};

				//	if (mrb.len >= l)
				//	{
				//		ManRcvStop();
				//	};
				//};
			};

			break;

		case 2:

			if (tm.Check(US2RT(100)))
			{
//				SetTrmBoudRate(3); /*mtb.data = tableCRC;*/ mtb.len = 5; SendMLT3(&mtb);
				SendManData(&mtb);

				i++;
			};

			break;

		case 3:

			if (mtb.ready)
			{
				i = 0;
			};

			break;

	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void CopyDataDMA(volatile void *src, volatile void *dst, u16 len)
{
	using namespace HW;

	register u32 t __asm("r0");

	HW::GPDMA1->DMACFGREG = 1;

	if ((HW::GPDMA1->CHENREG & (1<<3)) == 0)
	{
		HW::P5->BTGL(9);

		HW::GPDMA1_CH3->CTLL = DINC(0)|SINC(0)|TT_FC(0)|DEST_MSIZE(1)|SRC_MSIZE(1);
		HW::GPDMA1_CH3->CTLH = BLOCK_TS(len);

	//	t = DMAC->EBCISR;

		HW::GPDMA1_CH3->SAR = (u32)src;
		HW::GPDMA1_CH3->DAR = (u32)dst;
		HW::GPDMA1_CH3->CFGL = 0;
		HW::GPDMA1_CH3->CFGH = PROTCTL(1);

		//HW::GPDMA1->CLEARBLOCK = 1<<3;
		//HW::GPDMA1->CLEARDSTTRAN = 1<<3;
		//HW::GPDMA1->CLEARERR = 1<<3;
		//HW::GPDMA1->CLEARSRCTRAN = 1<<3;
		//HW::GPDMA1->CLEARTFR = 1<<3;

		HW::GPDMA1->CHENREG = 0x101<<3;
	};
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void Init_UART_DMA()
{
	T_HW::GPDMA_Type &dma = *HW::DMA0;
	T_HW::GPDMA_CH_Type &ch = HW::DMA0->CH[1];
	T_HW::USIC_CH_Type	&uart = *HW::USIC1_CH0; 

	HW::SCU_CLK->CGATCLR2 = SCU_CLK_CGATCLR2_DMA0_Msk;
	HW::SCU_RESET->PRCLR2 = SCU_RESET_PRCLR2_DMA0RS_Msk;

	dma.DMACFGREG = 1;

	ch.SAR = 0;

	ch.DAR = (u32)&HW::USIC1_CH0->TBUF[0];
	ch.LLP = 0;
	ch.CTLH = 0;
	ch.CTLL = TT_FC(1)|SRC_MSIZE(2)|DEST_MSIZE(0)|SINC(0)|DINC(2)|INT_EN;

	ch.CFGL = GPDMA0_CH_CFGL_HS_SEL_SRC_Msk;

	ch.SGR = 0;
	ch.DSR = 0;

	ch.CFGH = PROTCTL(1)|DEST_PER(1);
          
	//XMC_DMA_EnableRequestLine(dma, line, peripheral);
	HW::DLR->SRSEL0 = SRSEL0(10,11,0,0,0,0,0,0);
	HW::DLR->LNEN |= 3;

    ch.CFGL &= (uint32_t)~GPDMA0_CH_CFGL_HS_SEL_DST_Msk;

	HW::DMA0->CLEARTFR = 2;
	HW::DMA0->CLEARBLOCK = 2;
	HW::DMA0->CLEARSRCTRAN = 2;
	HW::DMA0->CLEARDSTTRAN = 2;
	HW::DMA0->CLEARERR = 2;


	HW::DMA0->MASKBLOCK = 0x101<<1;

//	XMC_DMA_CH_EnableEvent(XMC_DMA0, 2, XMC_DMA_CH_EVENT_BLOCK_TRANSFER_COMPLETE);

	/* Enable DMA event handling */
//	NVIC_SetPriority(GPDMA0_0_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 63, 0));
//	NVIC_EnableIRQ(GPDMA0_0_IRQn);


	HW::SCU_CLK->CGATCLR1 = SCU_CLK_CGATCLR1_USIC1_Msk;
	HW::SCU_RESET->PRCLR1 = SCU_RESET_PRCLR1_USIC1RS_Msk;

	uart.KSCFG = (USIC_CH_KSCFG_MODEN_Msk | USIC_CH_KSCFG_BPMODEN_Msk);

	while ((uart.KSCFG & USIC_CH_KSCFG_MODEN_Msk) == 0U)
	{
	/* Wait till the channel is enabled */
	}


	uart.FDR = STEP(970)|DM(1);

	uart.BRG = DCTQ(15)/*|PDIV(0x3A)*/;

	uart.PCR_ASCMode = SMD(1)|SP(9)|RSTEN(1)|TSTEN(1);

	uart.SCTR = PDL(1)|TRM(1)|FLE(7)|WLE(7);

	/* Enable transfer buffer */
	uart.TCSR = (0x1UL << USIC_CH_TCSR_TDEN_Pos) | USIC_CH_TCSR_TDSSM_Msk;

	/* Clear protocol status */
	uart.PSCR = 0xFFFFFFFFUL;

	/* Set parity settings */
	uart.CCR = 0;

	uart.DX0CR = (uint32_t)(uart.DX0CR & (~(USIC_CH_DX0CR_INSW_Msk|USIC_CH_DX0CR_DSEN_Msk)));

	uart.CCR |= TBIEN;
	uart.PCR_ASCMode |= 0;

	uart.INPR = 0;

	uart.CCR = uart.CCR & (~USIC_CH_CCR_MODE_Msk) | 2;

	uart.FMR = USIC_CH_FMR_SIO0_Msk;

	//ch.CTLH = len++;

	//ch.SAR = (u32)buf;

	//dma.CHENREG = (uint32_t)(0x101UL << 1);   

	//HW::DLR->SRSEL0 = SRSEL0(10,11,0,0,0,0,0,0);
	//HW::DLR->SRSEL1 = SRSEL1(0,0,0,0);
	//HW::DLR->LNEN |= 3;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void Send_UART_DMA()
{
	T_HW::GPDMA_Type &dma = *HW::DMA0;
	T_HW::GPDMA_CH_Type &ch = HW::DMA0->CH[1];
	T_HW::USIC_CH_Type	&uart = *HW::USIC1_CH0; 

	ch.DAR = (u32)&HW::USIC1_CH0->TBUF[0];
	ch.CTLL = TT_FC(1)|SRC_MSIZE(2)|DEST_MSIZE(0)|SINC(0)|DINC(2)|INT_EN;

	ch.CFGL = GPDMA0_CH_CFGL_HS_SEL_SRC_Msk;

	ch.CFGH = PROTCTL(1)|DEST_PER(1);
          
    ch.CFGL &= (uint32_t)~GPDMA0_CH_CFGL_HS_SEL_DST_Msk;


	HW::SCU_CLK->CGATCLR1 = SCU_CLK_CGATCLR1_USIC1_Msk;
	HW::SCU_RESET->PRCLR1 = SCU_RESET_PRCLR1_USIC1RS_Msk;

	uart.KSCFG = (USIC_CH_KSCFG_MODEN_Msk | USIC_CH_KSCFG_BPMODEN_Msk);

	while ((uart.KSCFG & USIC_CH_KSCFG_MODEN_Msk) == 0U)
	{
	/* Wait till the channel is enabled */
	}

	uart.FDR = STEP(970)|DM(1);

	uart.BRG = DCTQ(15)/*|PDIV(0x3A)*/;

	uart.PCR_ASCMode = SMD(1)|SP(9)|RSTEN(1)|TSTEN(1);

	uart.SCTR = PDL(1)|TRM(1)|FLE(7)|WLE(7);

	uart.TCSR = (0x1UL << USIC_CH_TCSR_TDEN_Pos) | USIC_CH_TCSR_TDSSM_Msk;

	uart.PSCR = 0xFFFFFFFFUL;

	uart.CCR = 0;

	uart.DX0CR = (uint32_t)(uart.DX0CR & (~(USIC_CH_DX0CR_INSW_Msk|USIC_CH_DX0CR_DSEN_Msk)));

	uart.CCR |= TBIEN;
	uart.PCR_ASCMode |= 0;

	uart.INPR = 0;

	uart.CCR = uart.CCR & (~USIC_CH_CCR_MODE_Msk) | 2;

//	uart.FMR = USIC_CH_FMR_SIO0_Msk;


	ch.CTLH = len++;

	ch.SAR = (u32)buf;

	dma.CHENREG = (uint32_t)(0x101UL << 1);   
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void Init_Timer()
{
	T_HW::CCU4_GLOBAL_Type *module = HW::CCU42;
	T_HW::CCU4_CC4_Type *slice = HW::CCU42_CC40;

	u32 gctrl;

//	module->GCTRL = 0;

	HW::SCU_CLK->CLKSET = SCU_CLK_CLKSET_CCUCEN_Msk;

	HW::SCU_CLK->CGATCLR0 = SCU_CLK_CGATCLR0_CCU42_Msk;

//	HW::SCU_RESET->PRSET0 = SCU_RESET_PRSET0_CCU40RS_Msk;
	HW::SCU_RESET->PRCLR0 = SCU_RESET_PRCLR0_CCU42RS_Msk;

//	module->GIDLC |= CCU4_GIDLC_SPRB_Msk;

	//gctrl = module->GCTRL;
	//gctrl &= ~CCU4_GCTRL_MSDE_Msk;
	//gctrl |= 0 << CCU4_GCTRL_MSDE_Pos;

	module->GCTRL = 0;

	module->GIDLC = CCU4_GIDLC_CS0I_Msk|CCU4_GIDLC_SPRB_Msk;

	/* Program the timer mode */
//	slice->TC = 0;
	/* Enable the timer concatenation */
//	slice->CMC = 0 << CCU4_CC4_CMC_TCE_Pos;
	/* Program initial prescaler divider value */
//	slice->PSC = 0;
	/* Program the dither compare value */
//	slice->DITS = 0;
	/* Program timer output passive level */
//	slice->PSL = 0;
	/* Program floating prescaler compare value */
//	slice->FPCS = 0;
	
	slice->PRS = 65500U;

	slice->CRS = 32000U;

	module->GCSS = CCU4_GCSS_S0SE_Msk;  

	//slice->CMC = 0;//(slice->CMC & ~CCU4_CC4_CMC_STRTS_Msk) | (0 << CCU4_CC4_CMC_STRTS_Pos);

	//slice->TC |= CCU4_CC4_TC_STRM_Msk;

	slice->TCSET = 1;

	//HW::SCU_GENERAL->CCUCON |= SCU_GENERAL_CCUCON_GSC42_Msk;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateTemp()
{
	static byte i = 0;

	static DSCTWI dsc, dsc2;
	static byte reg = 0;
	static u16 rbuf = 0;
	static byte buf[10];
	//static byte *romData = 0;
	//static u16 romAdr = 0;
	//static u16 revRomAdr = 0;
	//static u16 romWrLen = 0;
	//static u16 romRdLen = 0;
	//static u16 pageLen = 0;

	static TM32 tm;

//	HW::GPIO->SET0 = 1<<12;

	switch (i)
	{
		case 0:

			if (tm.Check(100))
			{
				buf[0] = 0x11;

				dsc.adr = 0x68;
				dsc.wdata = buf;
				dsc.wlen = 1;
				dsc.rdata = &rbuf;
				dsc.rlen = 2;
				dsc.wdata2 = 0;
				dsc.wlen2 = 0;

				//buf[0] = 0x0E;
				//buf[1] = 0x20;
				//buf[2] = 0xC8;

				//dsc.adr = 0x68;
				//dsc.wdata = buf;
				//dsc.wlen = 3;
				//dsc.rdata = 0;
				//dsc.rlen = 0;
				//dsc.wdata2 = 0;
				//dsc.wlen2 = 0;

				if (AddRequest_TWI(&dsc))
				{
					i++;
				};
			};

			break;

		case 1:

			if (dsc.ready)
			{
				temp = ((i16)ReverseWord(rbuf) + 128) / 256;

				i++;
			};

			break;

		case 2:

//			if (tm.Check(100))
			{
				buf[0] = 0x0E;
				buf[1] = 0x20;
				buf[2] = 0xC8;

				dsc2.adr = 0x68;
				dsc2.wdata = buf;
				dsc2.wlen = 3;
				dsc2.rdata = 0;
				dsc2.rlen = 0;
				dsc2.wdata2 = 0;
				dsc2.wlen2 = 0;

				if (AddRequest_TWI(&dsc2))
				{
					i++;
				};
			};

			break;

		case 3:

			if (dsc2.ready)
			{
				i = 0;
			};

			break;
	};

//	HW::GPIO->CLR0 = 1<<12;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateMisc()
{
	static byte i = 0;

	#define CALL(p) case (__LINE__-S): p; break;

	enum C { S = (__LINE__+3) };
	switch(i++)
	{
		CALL( UpdateMan();		);
		CALL( UpdateTemp();		);
		CALL( FLASH_Update();	);
		CALL( UpdateEMAC();	);
		CALL( UpdateTraps();	);
	};

	i = (i > (__LINE__-S-3)) ? 0 : i;

	#undef CALL
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void Update()
{
	static byte i = 0;

	#define CALL(p) case (__LINE__-S): p; break;

	enum C { S = (__LINE__+3) };
	switch(i++)
	{
		CALL( NAND_Idle();	);
		CALL( UpdateMisc();	);
	};

	i = (i > (__LINE__-S-3)) ? 0 : i;

	#undef CALL
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

int main()
{
	static TM32 rtm;
	//static MTB mtb;
	//static u16 manbuf[10];
	//static DSCTWI dsctwi;
	//static DSCTWI dsc;

	//__breakpoint(0);

	InitHardware();

	Init_TWI();

	InitEMAC();

	InitTraps();

	FLASH_Init();


	//buf[4999] = 0x55;

	//wb.len = 5000;

	while(1)
	{
		HW::P2->BSET(6);
		f++;

		Update();

		HW::P2->BCLR(6);

		if (rtm.Check(1000))
		{	
			fps = f; f = 0; 

			//mtb.data = manbuf;
			//mtb.len = 3;

			//manbuf[0] = 0xAA00;

			//SendManData(&mtb);
			//SendMLT3(&mtb);
		};
	};

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

