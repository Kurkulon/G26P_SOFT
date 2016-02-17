#include "core.h"
#include "emac.h"
#include "EMAC_DEF.h"
#include "xtrap.h"

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/* Net_Config.c */

#define OUR_IP_ADDR   	IP32(192, 168, 3, 234)
#define DHCP_IP_ADDR   	IP32(192, 168, 3, 254)

static const MAC hwAdr = {0x12345678, 0x9ABC};
static const MAC hwBroadCast = {0xFFFFFFFF, 0xFFFF};
static const u32 ipAdr = OUR_IP_ADDR;//IP32(192, 168, 10, 1);
static const u32 ipMask = IP32(255, 255, 255, 0);

static const u16 udpInPort = SWAP16(66);
static const u16 udpOutPort = SWAP16(66);

static bool EmacIsConnected = false;

/* Local variables */

static byte RxBufIndex = 0;
static byte TxBufIndex = 0;
static byte TxFreeIndex = 0;

enum	StateEM { LINKING, CONNECTED };	

StateEM stateEMAC = LINKING;

static byte linkState = 0;

u32 reqArpCount = 0;
u32 reqIpCount = 0;
u32 reqUdpCount = 0;
u32 reqIcmpCount = 0;
u32 rxCount = 0;
u32 countBNA = 0;
u32 countREC = 0;
u32 countRXOVR = 0;
u32 countHNO = 0;



/* GMAC local IO buffers descriptors. */
Buf_Desc Rx_Desc[NUM_RX_BUF];
Buf_Desc Tx_Desc[NUM_TX_DSC];

/* GMAC local buffers must be 8-byte aligned. */
byte rx_buf[NUM_RX_BUF][ETH_RX_BUF_SIZE];
//byte tx_buf[NUM_TX_BUF][ETH_TX_BUF_SIZE];

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct SysEthBuf : public EthBuf
{
	byte data[((sizeof(EthDhcp) + 127) & ~127) - sizeof(EthBuf)];
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static SysEthBuf	sysTxBuf[8];
static byte			indSysTx = 0;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

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

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

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

static SysEthBuf* GetSysTxBuffer()
{
	SysEthBuf &p = sysTxBuf[indSysTx];

	if (p.len == 0)
	{
		indSysTx = (indSysTx + 1) & 7;
		return &p;
	}
	else
	{
		return 0;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static Buf_Desc* GetTxDesc()
{
	Buf_Desc *p = 0;
	Buf_Desc &td = Tx_Desc[TxBufIndex];

	if ((td.stat & TD_TRANSMIT_OK) && (td.stat & TD_LENGTH_MASK) == 0)
	{
		p = &td;
		TxBufIndex = (td.stat & TD_TRANSMIT_WRAP) ? 0 : TxBufIndex + 1;
	};

	return p;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void FreeTxDesc()
{
	Buf_Desc &td = Tx_Desc[TxFreeIndex];

	if ((td.stat & TD_TRANSMIT_OK) && (td.stat & TD_LENGTH_MASK) != 0)
	{
		td.stat &= TD_TRANSMIT_OK|TD_TRANSMIT_WRAP;

		*((u32*)(td.addr-4)) = 0;

		TxFreeIndex = (td.stat & TD_TRANSMIT_WRAP) ? 0 : TxFreeIndex + 1;
	};
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

static bool TransmitPacket(Buf_Desc *dsc, EthBuf *b)	// Send a packet
{
	if (dsc == 0 || b == 0 || b->len == 0)
	{
		return false;
	};

	dsc->addr = (u32)&b->eth;
	dsc->stat &= TD_TRANSMIT_OK|TD_TRANSMIT_WRAP;
	dsc->stat |= TD_LAST_BUF | (b->len & TD_LENGTH_MASK);
	dsc->stat &= ~TD_TRANSMIT_OK;

	HW::GMAC->TSR = 0x17F;
	HW::GMAC->NCR |= GMAC_TXEN|GMAC_TSTART;

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


bool EMAC_SendData(void *pData, u16 length)
{
	static u16 TxDataID = 0;
	
	const u16 BLOK_LEN = 1480;	// Split to max IPlen=1500
	const u16 MAX_TIME_TO_TRANSMIT_MS = 10;	// ms, do not write zero!

	//if(!EmacIsConnected) return false; 
	//if(!ComputerFind)  return false;
	//length += sizeof(AT91S_UDPHdr);
	//unsigned int blok = 0;
	//unsigned int sended_len = 0;
	//unsigned short checksum;
	//unsigned int i,j;
	bool ready = true;
 
	TxDataID ++;

	//AT91S_EthHdr *OurTxPacketEthHeader;
	//AT91S_IPheader *OurTxPacketIpHeader;
	//while(sended_len < length) 
	//{
 //       	OurTxPacketEthHeader = (AT91S_EthHdr *)((unsigned int)((unsigned int)TxPacket + TxBuffIndex*ETH_TX_BUFFER_SIZE)&0xFFFFFFFC);
	//	OurTxPacketIpHeader = (AT91S_IPheader *)((unsigned int)OurTxPacketEthHeader + 14);
	//	// EthHeader
	//	for(i=0;i<6;i++) 
	//	{ 
	//		OurTxPacketEthHeader->et_dest[i] = ComputerEmacAddr[i];
 //      		        OurTxPacketEthHeader->et_src[i] = OurEmacAddr[i];
	//	}
	//	OurTxPacketEthHeader->et_protlen = SWAP16(PROT_IP);
	//	// IpHeader
	//	for(i=0;i<4;i++) 
	//	{ 
	//		OurTxPacketIpHeader->ip_dst[i] = ComputerIpAddr[i];
	//		OurTxPacketIpHeader->ip_src[i] = OurIpAddr[i];
	//	}
	//	OurTxPacketIpHeader->ip_hl_v 	= OurRxPacketIpHeader->ip_hl_v;	// may be fix  = 0x45
	//	OurTxPacketIpHeader->ip_tos 	= OurRxPacketIpHeader->ip_tos;  // may be fix  = 0x00
	//	OurTxPacketIpHeader->ip_id 	= SWAP16(TxDataID);
	//	OurTxPacketIpHeader->ip_ttl	= 128;
	//	OurTxPacketIpHeader->ip_p	= PROT_UDP;
	//	unsigned short offset = 0x0000;
	//	if(length - sended_len > BLOK_LEN) offset |= 0x2000;
	//	offset |= (sended_len/8)&0x1FFF;
	//	OurTxPacketIpHeader->ip_off 	= SWAP16(offset);
	//	// UDP header
	//	char *data;
 //       	if(blok==0)	
	//	{
	//		OurTxPacketIpHeader->udp_src 	= SWAP16(OurUDPPort);
	//		OurTxPacketIpHeader->udp_dst 	= SWAP16(ComputerUDPPort);
	//		OurTxPacketIpHeader->udp_len 	= SWAP16(length);
	//		OurTxPacketIpHeader->udp_xsum	= 0;
	//		checksum = ~NetChksumAdd(NetChksumAdd(NetChksum((unsigned short *)(&(OurTxPacketIpHeader->udp_src)), sizeof(AT91S_UDPHdr)), 
	//						   PseudoChksum((unsigned char *)(OurTxPacketIpHeader->ip_src), (unsigned char *)(OurTxPacketIpHeader->ip_dst), SWAP16(OurTxPacketIpHeader->udp_len))),
	//					NetChksum((unsigned short *)(pData), length-sizeof(AT91S_UDPHdr)));
	//		OurTxPacketIpHeader->udp_xsum	= checksum;
	//		sended_len += sizeof(AT91S_UDPHdr);
	//		data = (char *)(&OurTxPacketIpHeader->udp_xsum) + sizeof(OurTxPacketIpHeader->udp_xsum);
	//		i = sizeof(AT91S_UDPHdr);
	//	}
	//	else
	//	{
	//		i = 0;
	//		data = (char *)(&OurTxPacketIpHeader->udp_src);
	//	}

	//	i = sended_len + BLOK_LEN - i;
	//	if (i>=length) i=length;
	//	while(sended_len < i)
	//	{
	//        	*((unsigned short *)data) = (*((unsigned short *)pData));
	//		data+=2;
	//		pData+=2;
	//		sended_len+=2;
	//	}
	//	sended_len = i;
	//	// IP Header
	//	unsigned short ip_len		= sended_len - blok*BLOK_LEN + (OurTxPacketIpHeader->ip_hl_v&0x0F)*sizeof(unsigned int);
	//	OurTxPacketIpHeader->ip_len 	= SWAP16(ip_len);
 //               OurTxPacketIpHeader->ip_sum	= 0;
	//	checksum = SWAP16(IPChksum((unsigned short *)OurTxPacketIpHeader, (OurTxPacketIpHeader->ip_hl_v & 0x0F) * sizeof(unsigned int)/sizeof(unsigned short)));
	//	OurTxPacketIpHeader->ip_sum 	= checksum;
	//	// Transmit
	//	ready = ready & ProcessTxEmacPacket();
	//	TxtdList[TxBuffIndex].addr = (unsigned int)OurTxPacketEthHeader;
	//	TxtdList[TxBuffIndex].U_Status.S_Status.Length = SWAP16(OurTxPacketIpHeader->ip_len) + 14;
	//	TxtdList[TxBuffIndex].U_Status.S_Status.LastBuff = 1;
	//	if (TxBuffIndex == (NB_TX_BUFFERS - 1))	TxBuffIndex = 0; else TxBuffIndex ++;
	//	AT91C_BASE_EMAC->EMAC_NCR |= AT91C_EMAC_TSTART;
	//	blok++;
	//}
	//EmacTxCounter++;
	return ready;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void RequestARP(EthArp *h, u32 stat)
{
//	ArpHdr *pArp = (ArpHdr*)eth->data;

	if (ReverseWord(h->arp.op) == ARP_REQUEST) // ARP REPLY operation
	{     
		if (h->arp.tpa == ipAdr)
		{
			reqArpCount++;

			Buf_Desc *dsc = GetTxDesc();

			EthBuf *buf = GetSysTxBuffer();

			if (dsc == 0 || buf == 0) return;

			EthArp *t = (EthArp*)&buf->eth;

			t->eth.dest = h->eth.src;
			t->eth.src  = hwAdr;

			t->eth.protlen = SWAP16(PROT_ARP);

			t->arp.hrd = 0x100;	
			t->arp.pro = 8;	
			t->arp.hln = 6;	
			t->arp.pln = 4;	
			t->arp.op =  SWAP16(ARP_REPLY);				

			t->arp.tha = h->arp.sha;
			t->arp.sha = hwAdr;

			t->arp.tpa = h->arp.spa;
			t->arp.spa = ipAdr;

			buf->len = sizeof(EthArp);

			TransmitPacket(dsc, buf);
		};	
	}			
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

u16 IpChkSum(u16 *p, u16 size)
{
	register u32 sum = 0;
	register u32 t;

loop:

	__asm
	{
		LDRH	t, [p], #2 
		ADD		sum, t
		SUBS	size, size, #1
		BNE		loop

		LSR		t, sum, #16
		AND		sum, sum, #0xFFFF
		ADD		sum, sum, t
		ADD		sum, sum, sum, LSR#16 
	};
 
	return ~sum;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void RequestICMP(EthIcmp *h, u32 stat)
{
	if(h->icmp.type == ICMP_ECHO_REQUEST)
	{
		reqIcmpCount++;

		Buf_Desc *dsc = GetTxDesc();

		EthBuf *buf = GetSysTxBuffer();

		if (dsc == 0 || buf == 0) return;

		EthIcmp *t = (EthIcmp*)&buf->eth;

		t->eth.dest = h->eth.src;
		t->eth.src  = hwAdr;

		t->eth.protlen = SWAP16(PROT_IP);

		t->iph.hl_v = 0x45;	
		t->iph.tos = 0;		
		t->iph.len = h->iph.len;		
		t->iph.id = h->iph.id;		
		t->iph.off = 0;		
		t->iph.ttl = 64;		
		t->iph.p = PROT_ICMP;		
		t->iph.sum = 0;		
		t->iph.src = ipAdr;		
		t->iph.dst = h->iph.src;	

		t->iph.sum = IpChkSum((u16*)&t->iph, 10);

		u16 icmp_len = (ReverseWord(t->iph.len) - 20);	// Checksum of the ICMP Message

		if (icmp_len & 1)
		{
			*((byte*)&h->icmp + icmp_len) = 0;
			icmp_len ++;
		};

		icmp_len >>= 1;

		u16 *d = (u16*)&t->icmp;
		u16 *s = (u16*)&h->icmp;
		u16 c = icmp_len;

		while (c > 0)
		{
			*d++ = *s++; c--;
		};

		t->icmp.type = ICMP_ECHO_REPLY;
		t->icmp.code = 0;
		t->icmp.cksum = 0;

		t->icmp.cksum = IpChkSum((u16*)&t->icmp, icmp_len);

		buf->len = ReverseWord(t->iph.len) + sizeof(t->eth);

		TransmitPacket(dsc, buf);
	}
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void RequestDHCP(EthDhcp *h, u32 stat)
{
	if (h->dhcp.op != 1) return;

	i32 optLen = (i32)ReverseWord(h->iph.len) - sizeof(h->iph) - sizeof(h->udp) - 240;

	if (optLen < 3 || h->dhcp.magic != DHCPCOOKIE) return;

	i32 i = 0; 
	bool c = false;

	while (i < optLen)
	{
		if (h->dhcp.options[i] == 53)
		{
			c = true; break;
		};
		
		i++;

		i += h->dhcp.options[i];
	};

	if (!c) return;

	byte op = h->dhcp.options[i+2];

	if (op != DHCPDISCOVER && op != DHCPREQUEST) return;

	Buf_Desc *dsc = GetTxDesc();

	EthBuf *buf = GetSysTxBuffer();

	if (dsc == 0 || buf == 0) return;

	EthDhcp *t = (EthDhcp*)&buf->eth;

	t->eth.dest = hwBroadCast;
	t->eth.src  = hwAdr;

	t->eth.protlen = ReverseWord(PROT_IP);


	t->dhcp.op = 2;
	t->dhcp.htype = 1;
	t->dhcp.hlen = 6;
	t->dhcp.hops = 0;
	t->dhcp.xid = h->dhcp.xid;
	t->dhcp.secs = 0;
	t->dhcp.flags = 0;
	t->dhcp.ciaddr = 0;
	t->dhcp.yiaddr = DHCP_IP_ADDR;//IP32(192, 168, 10, 2); // New client IP
	t->dhcp.siaddr = ipAdr;
	t->dhcp.giaddr = 0;
	t->dhcp.chaddr = h->dhcp.chaddr; //h->eth.src;
	t->dhcp.magic = DHCPCOOKIE;

	DataPointer p(t->dhcp.options);

	*p.b++ = 53;
	*p.b++ = 1;
	*p.b++ = (op == DHCPDISCOVER) ? DHCPOFFER : DHCPACK;

	*p.b++ = 1; // Sub-net Mask
	*p.b++ = 4;
	*p.d++ = ipMask;

	*p.b++ = 51; // IP Address Lease Time
	*p.b++ = 4;
	*p.d++ = SWAP32(3600*24);

	*p.b++ = 54; // Server IP
	*p.b++ = 4;
	*p.d++ = ipAdr;

	*p.b++ = 33;	// Static Route
	*p.b++ = 8;
	*p.d++ = ipAdr;	// Destination
	*p.d++ = t->dhcp.yiaddr; // Router

	*p.b++ = -1; // End option

	u16 ipLen = sizeof(h->iph) + sizeof(h->udp) + (sizeof(t->dhcp) - sizeof(t->dhcp.options)) + (p.b - (byte*)t->dhcp.options);

	t->iph.hl_v = 0x45;	
	t->iph.tos = 0;		
	t->iph.len = ReverseWord(ipLen);		
	t->iph.id = h->iph.id;		
	t->iph.off = 0;		
	t->iph.ttl = 64;		
	t->iph.p = PROT_UDP;		
	t->iph.sum = 0;		
	t->iph.src = ipAdr;		
	t->iph.dst = -1; //BroadCast	

	t->iph.sum = IpChkSum((u16*)&t->iph, 10);

	t->udp.src = BOOTPS;
	t->udp.dst = BOOTPC;
	t->udp.len = ReverseWord(ipLen - sizeof(t->iph));
	t->udp.xsum = 0;

	buf->len = ipLen + sizeof(t->eth);

	TransmitPacket(dsc, buf);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//static void RequestMyUDP(EthUdp *h, u32 stat)
//{
//	Buf_Desc *buf = GetTxDesc();
//
//	if (buf == 0) return;
//
//
//	EthUdp *t = (EthUdp*)buf->addr;
//
//	t->eth.dest = h->eth.src;
//	t->eth.src  = hwAdr;
//
//	t->eth.protlen = SWAP16(PROT_IP);
//
//	t->iph.hl_v = 0x45;	
//	t->iph.tos = 0;		
//	t->iph.len = h->iph.len;		
//	t->iph.id = h->iph.id;		
//	t->iph.off = 0;		
//	t->iph.ttl = 64;		
//	t->iph.p = PROT_UDP;		
//	t->iph.sum = 0;		
//	t->iph.src = ipAdr;		
//	t->iph.dst = h->iph.src;	
//
//	t->iph.sum = IpChkSum((u16*)&t->iph, 10);
//
//	t->udp.src = udpInPort;
//	t->udp.dst = udpOutPort;
//	t->udp.len = h->udp.len;
//	t->udp.xsum = 0;
//
//	TransmitPacket(buf, ReverseWord(t->iph.len) + sizeof(t->eth));
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void RequestUDP(EthUdp *h, u32 stat)
{
	switch (h->udp.dst)
	{
		case BOOTPS:	RequestDHCP((EthDhcp*)h, stat); break;
		case udpInPort: RequestTrap(h, stat); break;
	};

	reqUdpCount++;

}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void RequestIP(EthIp *h, u32 stat)
{
//	IPheader *iph = (IPheader*)(eth->data);	

	if (h->iph.hl_v != 0x45) return;

	reqIpCount++;

	switch(h->iph.p)
	{
		case PROT_ICMP:	RequestICMP((EthIcmp*)h, stat);	break; 

		case PROT_UDP:	if ((stat & RD_IP_CHECK) == RD_IP_UDP_OK) { RequestUDP((EthUdp*)h, stat); };		break;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void RecieveFrame()
{
	Buf_Desc &buf = Rx_Desc[RxBufIndex];

	register u32 t = HW::GMAC->RSR;
	HW::GMAC->RSR = RSR_HNO | RSR_RXOVR | RSR_REC | RSR_BNA;

	if (t & RSR_BNA)	{ countBNA++;	/*HW::GMAC->RSR = RSR_BNA;*/	};
	if (t & RSR_REC)	{ countREC++;	/*HW::GMAC->RSR = RSR_REC;*/	};
	if (t & RSR_RXOVR)	{ countRXOVR++;	/*HW::GMAC->RSR = RSR_RXOVR;*/	};
	if (t & RSR_HNO)	{ countHNO++;	/*HW::GMAC->RSR = RSR_HNO;*/	};


	if((buf.addr & OWNERSHIP_BIT) == 0)
	{
		FreeTxDesc();

		return;
	};


//	UpdateStatistic();

	// Receive one packet

	EthPtr ep;

	if ((buf.stat & (RD_EOF|RD_SOF)) == (RD_EOF|RD_SOF)) // buffer contains a whole frame
	{
		ep.eth = (EthHdr*)(buf.addr & ~3);

		switch (ReverseWord(ep.eth->protlen))
		{
			case PROT_ARP: // ARP Packet format

				RequestARP(ep.earp, buf.stat);
				break; 

			case PROT_IP:	// IP protocol frame

				if (buf.stat & RD_IP_CHECK)
				{
					RequestIP(ep.eip, buf.stat);
				};

				break;
		};
	};

	buf.addr &= ~OWNERSHIP_BIT;
	++rxCount;
	RxBufIndex = (buf.addr & WRAP_BIT) ? 0 : (RxBufIndex+1);
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
	HW::GMAC->SA[0].B = hwAdr.B;
	HW::GMAC->SA[0].T = hwAdr.T;

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
				EmacIsConnected = true;
			};
			
			break;

		case CONNECTED:

			if (!CheckLink())
			{
				HW::GMAC->NCR &= ~GMAC_RXEN;
				StartLink();
				stateEMAC = LINKING;
				EmacIsConnected = false;
			}
			else
			{
				RecieveFrame();
			};


			break;

	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


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

	HW::GMAC->DCFGR = GMAC_DRBS(ETH_RX_DRBS)|GMAC_FBLDO_INCR4|GMAC_TXCOEN; // DMA Receive Buffer Size 512 bytes

	HW::GMAC->RBQP = (u32)&Rx_Desc[0]; // Set Rx Queue pointer to descriptor list.
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void tx_descr_init (void)
{
	TxBufIndex = 0;

	for (u32 i = 0; i < NUM_TX_DSC; i++)
	{
		Tx_Desc[i].addr = 0;//(u32)&tx_buf[i];
		Tx_Desc[i].stat = TD_TRANSMIT_OK;
	};
  
	Tx_Desc[NUM_TX_DSC-1].stat |= TD_TRANSMIT_WRAP; // Set the WRAP bit at the end of the list descriptor. 

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





