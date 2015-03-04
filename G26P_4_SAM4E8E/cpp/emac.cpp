#include "core.h"
#include "emac.h"
#include "EMAC_DEF.h"


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//static void WritePHY(byte reg, u16 data)
//{
//	HW::GMAC->MAN = 0x50020000 | (PHYA<<23) | ((reg&0x1F)<<18) | data;
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//static u16 ReadPHY(byte reg, u16 data)
//{
//	HW::GMAC->MAN = 0x50020000 | (PHYA<<23) | ((reg&0x1F)<<18) | data;
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*----------------------------------------------------------------------------
 *      RL-ARM - TCPnet
 *----------------------------------------------------------------------------
 *      Name:    GMAC_SAM9G20.C
 *      Purpose: Driver for Atmel AT91SAM9G20 GMAC Ethernet Controller
 *      Rev.:    V4.60
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2012 KEIL - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/

//#include <Net_Config.h>

/* The following macro definitions may be used to select the speed
   of the physical link:

  _10MBIT_   - connect at 10 MBit only
  _100MBIT_  - connect at 100 MBit only

  By default an autonegotiation of the link speed is used. This may take 
  longer to connect, but it works for 10MBit and 100MBit physical links.     */

/* Net_Config.c */

static const T_HW::S_GMAC::ADR hwAdr = {0x123456789ABC};
static const u32 ipAdr = IP32(192, 168, 10, 2);

/* Local variables */

static byte RxBufIndex = 0;
static byte TxBufIndex = 0;

enum	StateEM { LINKING, CONNECTED };	

StateEM stateEMAC = LINKING;

static byte linkState = 0;

u32 reqArpCount = 0;
u32 reqIpCount = 0;


/* GMAC local IO buffers descriptors. */
Buf_Desc Rx_Desc[NUM_RX_BUF];
Buf_Desc Tx_Desc[NUM_TX_BUF];

/* GMAC local buffers must be 8-byte aligned. */
byte rx_buf[NUM_RX_BUF][ETH_RX_BUF_SIZE];
byte tx_buf[NUM_TX_BUF][ETH_TX_BUF_SIZE];

struct GSTAT
{
	u32		FR;			
	u32		BCFR;		
	u32		MFR;		
	u32		PFR;		
	u32		BFR64;		
	u32		TBFR127;	
	u32		TBFR255;	
	u32		TBFR511;	
	u32		TBFR1023;	
	u32		TBFR1518;	
	u32		TMXBFR;		
	u32		UFR;		
	u32		OFR;		
	u32		JR;			
	u32		FCSE;		
	u32		LFFE;		
	u32		RSE;		
	u32		AE;			
	u32		RRE;		
	u32		ROE;		
	u32		IHCE;		
	u32		TCE;		
	u32		UCE;		
};

GSTAT stat = {0};

/*----------------------------------------------------------------------------
 *      GMAC Ethernet Driver Functions
 *----------------------------------------------------------------------------
 *  Required functions for Ethernet driver module:
 *  a. Polling mode: - void init_ethernet ()
 *                   - void send_frame (OS_FRAME *frame)
 *                   - void poll_ethernet (void)
 *  b. Interrupt mode: - void init_ethernet ()
 *                     - void send_frame (OS_FRAME *frame)
 *                     - void int_enable_eth ()
 *                     - void int_disable_eth ()
 *                     - interrupt function 
 *---------------------------------------------------------------------------*/

/* Local Function Prototypes */

//static void interrupt_ethernet (void) __irq;
//static void fetch_packet (void);
static void rx_descr_init (void);
static void tx_descr_init (void);
static void WritePHY(byte PhyReg, u16 Value);
static u16  ReadPHY(byte PhyReg);
static void ReqWritePHY(byte PhyReg, u16 Value);
static void ReqReadPHY(byte PhyReg);

inline void EnableMDI() { HW::GMAC->NCR |= GMAC_MPE; }
inline void DisableMDI() { HW::GMAC->NCR &= ~GMAC_MPE; }
inline bool IsReadyPHY() { return HW::GMAC->NSR & GMAC_IDLE; }
inline u16 ResultPHY() { return HW::GMAC->MAN; }

inline void StartLink() { linkState = 0; }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static Buf_Desc* GetTxDesc()
{
	Buf_Desc *p = 0;
	Buf_Desc &td = Tx_Desc[TxBufIndex];

	if (td.stat & TD_TRANSMIT_OK)
	{
		p = &td;
		TxBufIndex = (td.stat & TD_TRANSMIT_WRAP) ? 0 : TxBufIndex + 1;
	};

	return p;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateStatistic()
{
	using namespace HW;

	stat.FR			+= 	GMAC->FR;			
	stat.BCFR		+= 	GMAC->BCFR;		
	stat.MFR		+= 	GMAC->MFR;		
	stat.PFR		+= 	GMAC->PFR;		
	stat.BFR64		+= 	GMAC->BFR64;	
	stat.TBFR127	+= 	GMAC->TBFR127;	
	stat.TBFR255 	+= 	GMAC->TBFR255;	
	stat.TBFR511 	+= 	GMAC->TBFR511;	
	stat.TBFR1023 	+= 	GMAC->TBFR1023;	
	stat.TBFR1518 	+= 	GMAC->TBFR1518;	
	stat.TMXBFR		+= 	GMAC->TMXBFR;		
	stat.UFR 		+= 	GMAC->UFR;		
	stat.OFR 		+= 	GMAC->OFR;		
	stat.JR			+= 	GMAC->JR;			
	stat.FCSE		+= 	GMAC->FCSE;		
	stat.LFFE		+= 	GMAC->LFFE;		
	stat.RSE		+= 	GMAC->RSE;		
	stat.AE			+= 	GMAC->AE;			
	stat.RRE		+= 	GMAC->RRE;		
	stat.ROE		+= 	GMAC->ROE;		
	stat.IHCE		+= 	GMAC->IHCE;		
	stat.TCE		+= 	GMAC->TCE;		
	stat.UCE		+= 	GMAC->UCE;		
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool TransmitPacket(Buf_Desc *buf, u16 len)	// Send a packet
{
	if (buf == 0)
	{
		return false;
	};

	buf->stat &= TD_TRANSMIT_OK|TD_TRANSMIT_WRAP;
	buf->stat |= TD_LAST_BUF | (len & TD_LENGTH_MASK)| (7<<20);
	buf->stat &= ~TD_TRANSMIT_OK;

	HW::GMAC->TSR = 0x17F;
	HW::GMAC->NCR |= GMAC_TXEN|GMAC_TSTART;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void RequestARP(EthHdr *eth, u32 stat)
{
	ArpHdr *pArp = (ArpHdr*)eth->data;

	if (ReverseWord(pArp->ar_op) == ARP_REQUEST) // ARP REPLY operation
	{     
		if (pArp->ar_tpa == ipAdr)
		{
			reqArpCount++;

			Buf_Desc *buf = GetTxDesc();

			if (buf == 0) return;

			EthHdr *tEth = (EthHdr*)buf->addr;
			ArpHdr *tArp = (ArpHdr*)tEth->data;

			tEth->et_dest[0] = eth->et_src[0];
			tEth->et_dest[1] = eth->et_src[1];
			tEth->et_dest[2] = eth->et_src[2];

			tEth->et_src[0]  = hwAdr.W[0];
			tEth->et_src[1]  = hwAdr.W[1];
			tEth->et_src[2]  = hwAdr.W[2];

			tEth->et_protlen = ReverseWord(PROT_ARP);

			tArp->ar_hrd = 0x100;	
			tArp->ar_pro = 8;	
			tArp->ar_hln = 6;	
			tArp->ar_pln = 4;	
			tArp->ar_op =  ReverseWord(ARP_REPLY);				

			tArp->ar_tha[0]   = pArp->ar_sha[0];
			tArp->ar_tha[1]   = pArp->ar_sha[1];
			tArp->ar_tha[2]   = pArp->ar_sha[2];

			tArp->ar_sha[0] = hwAdr.W[0];
			tArp->ar_sha[1] = hwAdr.W[1];
			tArp->ar_sha[2] = hwAdr.W[2];

			tArp->ar_tpa = pArp->ar_spa;
			tArp->ar_spa = ipAdr;

			TransmitPacket(buf, sizeof(EthHdr) - sizeof(EthHdr::data) + sizeof(ArpHdr));
		};	
		
	}			
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u16 IcmpChksum(u16 *p, u16 size)
{
	U32u sum;

	sum.d = 0;
	
	for(register u16 i = 0; i < size; i++)
	{
		sum.d += ReverseWord(*p++);
		sum.w[0] += sum.w[1];
		sum.w[1] = 0;
	};

	return ~sum;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void RequestICMP(EthHdr *eth, IPheader *iph, u32 stat)
{
	IcmpEchoHdr *icmph = (IcmpEchoHdr*)(&iph->udp_src);
	
	if(icmph->type == ICMP_ECHO_REQUEST)
	{
		Buf_Desc *buf = GetTxDesc();

		if (buf == 0) return;

		EthHdr *tEth = (EthHdr*)buf->addr;
		IPheader *tiph = (IPheader*)tEth->data;
		IcmpEchoHdr *ticmph = (IcmpEchoHdr*)(&tiph->udp_src);

		tEth->et_dest[0] = eth->et_src[0];
		tEth->et_dest[1] = eth->et_src[1];
		tEth->et_dest[2] = eth->et_src[2];

		tEth->et_src[0]  = hwAdr.W[0];
		tEth->et_src[1]  = hwAdr.W[1];
		tEth->et_src[2]  = hwAdr.W[2];

		tEth->et_protlen = ReverseWord(PROT_IP);

		tiph->ip_hl_v = 0x45;	
		tiph->ip_tos = 0;		
		tiph->ip_len = iph->ip_len;		
		tiph->ip_id = iph->ip_id;		
		tiph->ip_off = 0;		
		tiph->ip_ttl = 64;		
		tiph->ip_p = PROT_ICMP;		
		tiph->ip_sum = 0;		
		tiph->ip_src = ipAdr;		
		tiph->ip_dst = iph->ip_src;	

		tiph->ip_sum = ReverseWord(IcmpChksum((u16*)tiph, 10));		

		u16 icmp_len = (ReverseWord(tiph->ip_len) - 20);	// Checksum of the ICMP Message

		if (icmp_len & 1)
		{
			*((byte*)icmph + icmp_len) = 0;
			icmp_len ++;
		};

		icmp_len >>= 1;

		u16 *d = (u16*)ticmph;
		u16 *s = (u16*)icmph;
		u16 c = icmp_len;

		while (c > 0)
		{
			*d++ = *s++; c--;
		};

		ticmph->type = ICMP_ECHO_REPLY;
		ticmph->code = 0;
		ticmph->cksum = 0;

		ticmph->cksum = ReverseWord(IcmpChksum((u16*)ticmph, icmp_len));

		TransmitPacket(buf, ReverseWord(iph->ip_len) + sizeof(EthHdr) - 1);
	}
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void RequestUDP(EthHdr *eth, u32 stat)
{

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void RequestIP(EthHdr *eth, u32 stat)
{
	reqIpCount++;

	IPheader *iph = (IPheader*)(eth->data);	

	if (!(stat & RD_IP_CHECK))
	{
		return;
	};

	switch(iph->ip_p)
	{
		case PROT_ICMP:	

			RequestICMP(eth, iph, stat);

			break; 

		//case PROT_UDP:		

		//	char *adrRxTxBuf;
		//	char *adrOurBuf;
		//	int len;
		//						bool NewPacket = false;
		//	if(pIpHeader->ip_id != OurRxPacketIpHeader->ip_id) 
		//	{
		//		if(((ReverseWord(pIpHeader->ip_off))&0x1FFF)==0)
		//		{
  //  						if(pIpHeader->udp_dst != ReverseWord(OurUDPPort)) 
		//			{ break; }
		//		}
		//		NewPacket = true;		
		//		}
		//	if(!NewPacket)
		//	{
		//		adrRxTxBuf = (char *)(pIpHeader->ip_src);
		//		adrOurBuf = (char *)(OurRxPacketIpHeader->ip_src);
		//		len = 8;
		//			while(len)
		//		{
		//			if(*adrOurBuf != *adrRxTxBuf) NewPacket = true;
		//			adrRxTxBuf++;
		//			adrOurBuf++;
		//			len --;
		//		}
		//	}
		//	if(NewPacket)
		//	{
		//		UDPBuildLength = 0;
		//		FirstFragment = false;
		//		// Copy EthHeader and IPHeader	
		//		adrRxTxBuf = (char *)(pEth);
		//		adrOurBuf = (char *)(OurRxPacketEthHeader);
		//		len = ((pIpHeader->ip_hl_v&0x0F)*sizeof(unsigned int)) + 14;
		//		while(len)
		//		{
		//			*adrOurBuf = *adrRxTxBuf;
		//			adrRxTxBuf++;
		//			adrOurBuf++;
		//			len --;
		//		}
		//	}
		//	
		//	unsigned short offset;			
		//	offset = ReverseWord(pIpHeader->ip_off);
		//	bool IPFragmentation;
		//	bool IPFragmentLast;
		//	if(((offset)&0x4000)) IPFragmentation = false;//IP Fragmentation  = no
		//		else IPFragmentation = true;
		//	if(((offset)&0x2000)) IPFragmentLast = false; //IP Fragment last = no
		//		else  IPFragmentLast = true; //IP Fragment last = yes
		//	adrRxTxBuf = ((char *)(pIpHeader) + (pIpHeader->ip_hl_v&0x0F)*sizeof(unsigned int));
		//	adrOurBuf = (char *)(OurRxPacketIpHeader) + ((pIpHeader->ip_hl_v&0x0F)*sizeof(unsigned int));
		//	if(IPFragmentation) adrOurBuf += 8*(offset&0x1FFF);
		//	char *startAdr = (char *)(RxtdList[0].addr & 0xFFFFFFFC);
		//	char *endAdr = (char *)((RxtdList[NB_RX_BUFFERS-1].addr & 0xFFFFFFFC) + ETH_RX_BUFFER_SIZE);
		//	len =  (ReverseWord(pIpHeader->ip_len)) - ((pIpHeader->ip_hl_v&0x0F)*sizeof(unsigned int));
		//	UDPBuildLength+=len;
  //                  		char *endOurAdr = (char *)(&OurRxPacket[MAX_OUR_RX_PACKET_SIZE-sizeof(unsigned int)]);
		//	while(len > 0)
		//	{
		//		if(adrOurBuf >= endOurAdr) break;	// Buf is full
		//		if(adrRxTxBuf >= endAdr) adrRxTxBuf = startAdr;
		//								*adrOurBuf = *adrRxTxBuf;
		//		adrRxTxBuf++;
		//		adrOurBuf++;
		//		len --;
		//	}
		//	if(adrOurBuf >= endOurAdr) 
		//	{ 
		//		UDPNeedLength = 0xFFFFFFFF;
		//		UDPBuildLength = 0;
		//		break;
		//	}
		//	if((offset&0x1FFF) == 0)
		//	{
		//		FirstFragment = true;
		//		UDPNeedLength = ReverseWord(pIpHeader->udp_len);
		//	}

		//	if((!IPFragmentation)||(FirstFragment&&(UDPBuildLength == UDPNeedLength)))
		//	{
		//		UDPNeedLength = 0xFFFFFFFF;
		//		UDPBuildLength = 0;
		//		need_handle_data =true;
		//	}
		//	break; 
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void RecieveFrame()
{
	Buf_Desc &buf = Rx_Desc[RxBufIndex];

	if((buf.addr & OWNERSHIP_BIT) == 0)
	{
		return;
	};
	

//	UpdateStatistic();

	// Receive one packet		

	EthHdr *pEth = (EthHdr*)(buf.addr & ~3);

	switch (ReverseWord(pEth->et_protlen))
	{
		case PROT_ARP: // ARP Packet format

			RequestARP(pEth, buf.stat);
			break; 

		case PROT_IP:	// IP protocol frame

			RequestIP(pEth, buf.stat);
			break;
	};

	Rx_Desc[RxBufIndex].addr &= ~OWNERSHIP_BIT;
	RxBufIndex = (Rx_Desc[RxBufIndex].addr & 2) ? 0 : (RxBufIndex+1);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



/*--------------------------- init_ethernet ---------------------------------*/

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool InitEMAC()
{

	using namespace HW;
	
	/* Initialize the GMAC ethernet controller. */
	u32 id1,id2;

	// Enable Peripheral Clock for GMAC Peripherals

	PMC->PCER0 = PID::PIOD_M;
	PMC->PCER1 = PID::GMAC_M;	

								// 1  8 7  4 3  0 9  6 5  2 1  8 7  4 3  0								
	PIOD->PDR =		0x0003FFFF;	// 0000 0000 0000 0011 1111 1111 1111 1111
	PIOB->ABCDSR1 &= ~0x3FFFF;	// xxxx xxxx xxxx xx00 0000 0000 0000 0000		
	PIOB->ABCDSR2 &= ~0x3FFFF;	// xxxx xxxx xxxx xx00 0000 0000 0000 0000		
	
	PIOD->PER =	(1<<29)|(1<<4)|(1<<7);
	PIOD->OER = (1<<29)|(1<<4);

	PIOD->SODR = (1<<29)|(1<<7); // Disable Reset, enable auto-mdix
	PIOD->CODR = 1<<4; // Enable MII

	PIOD->PUDR = (1<<4);
	PIOD->PPDER = (1<<4);

	HW::GMAC->NCFGR = (3<<18); // MDC CLock MCK/48

	EnableMDI();
  
	/* Put the DM9161 in reset mode */
	WritePHY(PHY_REG_BMCR, BMCR_RESET);

  /* Wait for hardware reset to end. */
	for (u32 tout = 0; tout < 0x800000; tout++)
	{
		if (!(ReadPHY(PHY_REG_BMCR) & BMCR_RESET))
		{
			break; /* Reset complete */
		}
	};

	PIOD->PDR = (1<<4)|(1<<7);

	id1 = ReadPHY(PHY_REG_IDR1);
	id2 = ReadPHY(PHY_REG_IDR2);

	if (((id1 << 16) | (id2 & 0xfff0)) == DP83848C_ID)
	{
	};

	//u16 bmcr = ReadPHY(PHY_REG_BMCR);
	//u16 anar = ReadPHY(PHY_REG_ANAR);
	//u16 rbr = ReadPHY(PHY_REG_RBR);
	//u16 phycr = ReadPHY(PHY_REG_PHYCR);
	//u16 btscr = ReadPHY(PHY_REG_10BTSCR);


	WritePHY(PHY_REG_BMCR, BMCR_ANENABLE|BMCR_FULLDPLX);
	WritePHY(PHY_REG_RBR, 1);
	WritePHY(PHY_REG_PHYCR, 0x8021);
	WritePHY(PHY_REG_10BTSCR, 0x0804);




	// disable_MDI ();

	// Enable GMAC in MII mode.
	HW::GMAC->UR = GMAC_MII;

	// Transmit and Receive disable. 
	HW::GMAC->NCR &= ~(GMAC_RXEN | GMAC_TXEN);

	/* Initialize Tx and Rx DMA Descriptors */
	rx_descr_init ();
	tx_descr_init ();

	/* The sequence write GMAC_SA1L and write GMAC_SA1H must be respected. */
	HW::GMAC->SA[0].U = hwAdr.U;
//	HW::GMAC->SA[0].T = hwAdr.T;

	/* Enable receiving of all Multicast packets. */
	HW::GMAC->HRB  = 0xFFFFFFFF;
	HW::GMAC->HRT  = 0xFFFFFFFF;

	/* Clear receive and transmit status registers. */
	HW::GMAC->RSR  = (RSR_RXOVR | RSR_REC | RSR_BNA | RSR_HNO);
	HW::GMAC->TSR  = (TSR_HRESP | TSR_UND | TSR_TXCOMP| TSR_TFC | TSR_RLE| TSR_COL | TSR_UBR);

	/* Configure GMAC operation mode, enable Multicast. */
	HW::GMAC->NCFGR |= GMAC_LFERD | GMAC_RXCOEN;// | GMAC_CAF ;
	//HW::GMAC->NCR   |= (GMAC_TXEN  | GMAC_RXEN | GMAC_WESTAT);

	/* Configure the GMAC Interrupts. */
	HW::GMAC->IDR  = -1;

	/* Allow RCOMP, RXUBR and ROVR interrupts. */
//	HW::GMAC->IER  = GMAC_RCOMP | GMAC_ROVR | GMAC_RXUBR;

	StartLink();

	/* Enable GMAC interrupts. */
	//pAIC->AIC_IDCR   = (1 << AT91C_ID_EMAC);
	//pAIC->AIC_SVR[AT91C_ID_EMAC] = (u32)interrupt_ethernet;
	//pAIC->AIC_SMR[AT91C_ID_EMAC] = AT91C_AIC_PRIOR_HIGHEST;
	//pAIC->AIC_ICCR   = (1 << AT91C_ID_EMAC);
	//pAIC->AIC_SPU    = (u32)def_interrupt;

  return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool UpdateLink()
{
	bool result = false;

	switch(linkState)
	{
		case 0:		

			ReqReadPHY(PHY_REG_BMSR);

			linkState++;

			break;

		case 1:

			if (IsReadyPHY())
			{
				if (ResultPHY() & BMSR_LINKST)
				{
					linkState++;
				}
				else
				{
					linkState = 0;
				};
			};

			break;

		case 2:

			ReqWritePHY(PHY_REG_BMCR, BMCR_ANENABLE|BMCR_FULLDPLX);

			linkState++;

			break;

		case 3:

			if (IsReadyPHY())
			{
				ReqReadPHY(PHY_REG_PHYSTS);

				linkState++;
			};

			break;

		case 4:

			if (IsReadyPHY())
			{
				if (ResultPHY() & PHYSTS_LINKST)
				{
					HW::GMAC->NCFGR &= ~(GMAC_SPD|GMAC_FD);

					if ((ResultPHY() & PHYSTS_SPEEDST) == 0)	// Speed 100Mbit is enabled.
					{
						HW::GMAC->NCFGR |= GMAC_SPD;
					};

					if (ResultPHY() & PHYSTS_DUPLEXST)	//  Full duplex is enabled.
					{
						HW::GMAC->NCFGR |= GMAC_FD;
					};

					result = true;

					linkState++;
				}
				else
				{
					linkState = 3;
				};
			};

			break;
	};

	return result;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool CheckLink() // Если нет связи, то результат false
{
	static byte state = 0;

	bool result = true;

	switch (state)
	{
		case 0:		

			ReqReadPHY(PHY_REG_BMSR);

			state++;

			break;

		case 1:

			if (IsReadyPHY())
			{
				result = ((ResultPHY() & BMSR_LINKST) != 0);

				state = 0;
			};

			break;
	};

	return result;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void UpdateEMAC()
{
	switch(stateEMAC)
	{
		case LINKING:

			if (UpdateLink())
			{
				stateEMAC = CONNECTED;
				HW::GMAC->NCR |= GMAC_RXEN;
			};
			
			break;

		case CONNECTED:

			if (!CheckLink())
			{
				HW::GMAC->NCR &= ~GMAC_RXEN;
				StartLink();
				stateEMAC = LINKING;
			}
			else if (HW::GMAC->ISR & (ISR_RXUBR | ISR_ROVR))
			{
				HW::GMAC->NCR &= ~GMAC_RXEN;
				rx_descr_init ();
				HW::GMAC->RSR  = RSR_RXOVR | RSR_REC | RSR_BNA;
				HW::GMAC->NCR |= GMAC_RXEN;
			}
			else
			{
				RecieveFrame();
			};


			break;

	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*--------------------------- int_enable_eth --------------------------------*/

static void int_enable_eth (void)
{
//	CM3::NVIC->ISER[HW::PID::GMAC_I/32] = HW::PID::GMAC_M;
}


//*--------------------------- int_disable_eth -------------------------------*/

static void int_disable_eth (void)
{
//	CM3::NVIC->ICER[HW::PID::GMAC_I/32] = HW::PID::GMAC_M;
}


/*--------------------------- send_frame ------------------------------------*/

void send_frame (OS_FRAME *frame) {
  /* Send frame to GMAC ethernet controller */
  u32 *sp,*dp;
  u32 i;

  /* Packet Transmit in progress, wait until finished. */
  while (HW::GMAC->TSR & TSR_TXGO);

  sp = (u32 *)&frame->data[0];
  dp = (u32 *)(Tx_Desc[0].addr & ~3);
  /* Copy frame data to GMAC IO buffer. */
  for (i = (frame->length + 3) >> 2; i; i--) {
    *dp++ = *sp++;
  }
  Tx_Desc[0].stat = frame->length | TD_LAST_BUF | TD_TRANSMIT_WRAP;

  /* Start frame transmission. */
  HW::GMAC->NCR |= GMAC_TSTART;
}


/*--------------------------- fetch_packet ----------------------------------*/

//static void fetch_packet (void) {
//  /* Fetch a packet from DMA buffer and release buffer. */
//  OS_FRAME *frame;
//  u32 j,ei,si,RxLen;
//  u32 *sp,*dp;
//
//	for (ei = RxBufIndex; ; RxBufIndex = ei)
//	{
//	    /* Scan the receive buffers. */
//		for (si = ei; ; )
//		{
//			if (!(Rx_Desc[ei].addr & AT91C_OWNERSHIP_BIT))	// End of scan, unused buffers found. 
//			{
//				if (si != ei) // Found erroneus fragment, release it. 
//				{
//					ei = si;
//					goto rel;
//		        };
//
//				return;
//			};
//
//			// Search for EOF.
//			
//			if (Rx_Desc[ei].stat & RD_EOF)
//			{
//				break;
//			};
//
//      /* Check the SOF-SOF sequence */
//      if (Rx_Desc[ei].stat & RD_SOF) {
//        /* Found one, this is new start of frame. */
//        si = ei;
//      }
//      if (++ei == NUM_RX_BUF) ei = 0;
//
//      if (ei == RxBufIndex) {
//        /* Safety limit to prevent deadlock. */
//        ei = si;
//        goto rel;
//      }
//    }
//
//    /* Get frame length. */
//    RxLen = Rx_Desc[ei].stat & RD_LENGTH_MASK;
//    if (++ei == NUM_RX_BUF) ei = 0;
//
//    if (RxLen > ETH_MTU) {
//      /* Packet too big, ignore it and free buffer. */
//      goto rel;
//    }
//
//    /* Flag 0x80000000 to skip sys_error() call when out of memory. */
////    frame = alloc_mem (RxLen | 0x80000000);
//
//    /* if 'alloc_mem()' has failed, ignore this packet. */
//    if (frame != 0) {
//      /* Make sure that block is 4-byte aligned */
//      dp = (u32 *)&frame->data[0];
//      for ( ; si != ei; RxLen -= ETH_RX_BUF_SIZE) {
//        sp = (u32 *)(Rx_Desc[si].addr & ~0x3);
//        j = RxLen;
//        if (j > ETH_RX_BUF_SIZE) j = ETH_RX_BUF_SIZE;
//        for (j = (j + 3) >> 2; j; j--) {
//          *dp++ = *sp++;
//        }
//        if (++si == NUM_RX_BUF) si = 0;
//      }
////      put_in_queue (frame);
//    }
//
//    /* Release packet fragments from GMAC IO buffer. */
//rel:for (j = RxBufIndex; ; ) {
//      Rx_Desc[j].addr &= ~AT91C_OWNERSHIP_BIT;
//      if (++j == NUM_RX_BUF) j = 0;
//      if (j == ei) break;
//    }
//  }
//}
//

/*--------------------------- interrupt_ethernet ----------------------------*/

//static void interrupt_ethernet (void) __irq
//{
//  /* GMAC Ethernet Controller Interrupt function. */
//  register u32 int_stat;
//
//  int_stat = HW::GMAC->ISR;
//
//  if (int_stat & (ISR_RXUBR | ISR_ROVR))
//  {
//    /* Receive buffer overflow, all packets are invalid, because */
//    /* a descriptor status is overwritten by DMA engine. */
//    HW::GMAC->NCR &= ~GMAC_RXEN;
//    rx_descr_init ();
//    HW::GMAC->RSR  = RSR_RXOVR | RSR_REC | RSR_BNA;
//    HW::GMAC->NCR |= GMAC_RXEN;
//  }
//  else if (int_stat & ISR_RCOMP)
//  {
//    /* Valid frame has been received. */
//    if (HW::GMAC->RSR & RSR_REC)
//	{
//      fetch_packet ();
//      HW::GMAC->RSR = RSR_REC;
//    };
//  };
//  /* Acknowledge the interrupt. */
////  pAIC->AIC_EOICR = 0;
//}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void rx_descr_init (void)
{
	RxBufIndex = 0;

	for (u32 i = 0; i < NUM_RX_BUF; i++)
	{
		Rx_Desc[i].addr = (u32)&rx_buf[i];
		Rx_Desc[i].stat = 0;
	};

	Rx_Desc[NUM_RX_BUF-1].addr |= 0x02; // Set the WRAP bit at the end of the list descriptor.

	HW::GMAC->DCFGR = 0x00180004|GMAC_TXCOEN; // DMA Receive Buffer Size 1536 bytes

	HW::GMAC->RBQP = (u32)&Rx_Desc[0]; // Set Rx Queue pointer to descriptor list.
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void tx_descr_init (void)
{
	TxBufIndex = 0;

	for (u32 i = 0; i < NUM_TX_BUF; i++)
	{
		Tx_Desc[i].addr = (u32)&tx_buf[i];
		Tx_Desc[i].stat = TD_TRANSMIT_OK;
	};
  
	Tx_Desc[NUM_TX_BUF-1].stat |= TD_TRANSMIT_WRAP; // Set the WRAP bit at the end of the list descriptor. 

	HW::GMAC->TBQP = (u32)&Tx_Desc[0]; // Set Tx Queue pointer to descriptor list. 
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


static void WritePHY (byte PhyReg, u16 Value)
{
	HW::GMAC->MAN = ((0x01<<30) | (2 << 16) | (1 << 28) | (PHYA << 23) | (PhyReg << 18)) | Value;

	while (!IsReadyPHY());
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static u16 ReadPHY (byte PhyReg)
{
  HW::GMAC->MAN = (0x01<<30) | (2 << 16) | (2 << 28) | (PHYA << 23) | (PhyReg << 18);

  while (!IsReadyPHY());
  
  return ResultPHY();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void ReqWritePHY(byte PhyReg, u16 Value)
{
	HW::GMAC->MAN = ((0x01<<30) | (2 << 16) | (1 << 28) | (PHYA << 23) | (PhyReg << 18)) | Value;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void ReqReadPHY(byte PhyReg)
{
  HW::GMAC->MAN = (0x01<<30) | (2 << 16) | (2 << 28) | (PHYA << 23) | (PhyReg << 18);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++





