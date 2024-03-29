#include "core.h"
#include "emac.h"
#include "EMAC_DEF.h"
#include "tftp.h"
#include "list.h"
#include "main.h"

//#pragma diag_suppress 546,550,177

//#pragma O3
//#pragma Otime


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/* EMAC Memory Buffer configuration. */
#define NUM_RX_BUF          16          /* 0x2000 for Rx (64*128=8K)         */
//#define ETH_RX_DRBS			8
#define ETH_RX_BUF_SIZE     (1024)       /* EMAC Receive buffer size.         */

#define NUM_TX_DSC          16         /* 0x0600 for Tx                     */
//#define ETH_TX_BUF_SIZE     1536        /* EMAC Transmit buffer size         */

#define AT91C_PHY_ADDR      0
#define PHYA 0


/* Net_Config.c */
#define OUR_IP_ADDR   	IP32(192, 168, 3, 227)
#define DHCP_IP_ADDR   	IP32(192, 168, 3, 254)

static const MAC hwAdr = {0x12345678, 0x9ABC};
static const MAC hwBroadCast = {0xFFFFFFFF, 0xFFFF};
static const u32 ipAdr = OUR_IP_ADDR;//IP32(192, 168, 10, 1);
static const u32 ipMask = IP32(255, 255, 255, 0);

static const u16 udpInPort	= _SWAP16(69);
static const u16 udpOutPort = _SWAP16(69);

bool emacConnected = false;
bool emacEnergyDetected = false;
bool emacCableNormal = false;

u16 reg_BMSR = 0;
u16 reg_LINKMDCS = 0;
u16 reg_PHYCON1 = 0;
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

u32 trp[4] = {-1};

u16  txIpID = 0;


/* GMAC local IO buffers descriptors. */
Receive_Desc Rx_Desc[NUM_RX_BUF] __attribute__((at(0x20020000)));;
Transmit_Desc Tx_Desc[NUM_TX_DSC];

/* GMAC local buffers must be 8-byte aligned. */
//byte rx_buf[NUM_RX_BUF][ETH_RX_BUF_SIZE];
//byte tx_buf[NUM_TX_BUF][ETH_TX_BUF_SIZE];

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct SysEthBuf : public EthBuf
{
	byte data[((sizeof(EthDhcp) + 127) & ~127) - sizeof(EthBuf)];
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static SysEthBuf	sysTxBuf[4];
static byte			indSysTx = 0;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static List<EthBuf> txList;

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//struct GSTAT
//{
//	u32		FR;			
//	u32		BCFR;		
//	u32		MFR;		
//	u32		PFR;		
//	u32		BFR64;		
//	u32		TBFR127;	
//	u32		TBFR255;	
//	u32		TBFR511;	
//	u32		TBFR1023;	
//	u32		TBFR1518;	
//	u32		TMXBFR;		
//	u32		UFR;		
//	u32		OFR;		
//	u32		JR;			
//	u32		FCSE;		
//	u32		LFFE;		
//	u32		RSE;		
//	u32		AE;			
//	u32		RRE;		
//	u32		ROE;		
//	u32		IHCE;		
//	u32		TCE;		
//	u32		UCE;		
//};
//
//GSTAT stat = {0};

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

inline void EnableMDI() { /*HW::GMAC->NCR |= GMAC_MPE;*/ }
inline void DisableMDI() { /*HW::GMAC->NCR &= ~GMAC_MPE;*/ }
inline bool IsReadyPHY() { return (HW::ETH0->GMII_ADDRESS & GMII_MB) == 0; }
inline bool IsBusyPHY() { return !IsReadyPHY(); }
inline u16 ResultPHY() { return HW::ETH0->GMII_DATA; }

inline void StartLink() { linkState = 0; }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static SysEthBuf* GetSysTxBuffer()
{
	SysEthBuf &p = sysTxBuf[indSysTx];

	if (p.len == 0)
	{
		indSysTx = (indSysTx + 1) & 3;
		return &p;
	}
	else
	{
		return 0;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static Transmit_Desc* GetTxDesc()
{
	Transmit_Desc *p = 0;
	Transmit_Desc &td = Tx_Desc[TxBufIndex];

	if ((td.stat & TD0_OWN) == 0 && (td.ctrl & TD1_TBS1) == 0)
	{
		p = &td;
		TxBufIndex = (td.stat & TD0_TER) ? 0 : TxBufIndex + 1;
	};

	return p;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void FreeTxDesc()
{
	Transmit_Desc &td = Tx_Desc[TxFreeIndex];

	if ((td.stat & TD0_OWN) == 0 && (td.ctrl & TD1_TBS1) != 0)
	{
		td.ctrl = 0;

		EthBuf* b = (EthBuf*)(td.addr1 - 8);

		//if (b->next != 0)
		//{
		//	__breakpoint(0);
		//};

		b->len = 0;

//		*((u32*)(td.addr-4)) = 0;

//		td.addr = (u32)(trp+1);

		TxFreeIndex = (td.stat & TD0_TER) ? 0 : TxFreeIndex + 1;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//void EMAC_FreeRxDsc(Receive_Desc* dsc)
//{
//	if (dsc != 0)
//	{
//		dsc->stat |= RD0_OWN;
//		dsc->next = 0;
//	};
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//static void UpdateStatistic()
//{
//	using namespace HW;
//
//	stat.FR			+= 	GMAC->FR;			
//	stat.BCFR		+= 	GMAC->BCFR;		
//	stat.MFR		+= 	GMAC->MFR;		
//	stat.PFR		+= 	GMAC->PFR;		
//	stat.BFR64		+= 	GMAC->BFR64;	
//	stat.TBFR127	+= 	GMAC->TBFR127;	
//	stat.TBFR255 	+= 	GMAC->TBFR255;	
//	stat.TBFR511 	+= 	GMAC->TBFR511;	
//	stat.TBFR1023 	+= 	GMAC->TBFR1023;	
//	stat.TBFR1518 	+= 	GMAC->TBFR1518;	
//	stat.TMXBFR		+= 	GMAC->TMXBFR;		
//	stat.UFR 		+= 	GMAC->UFR;		
//	stat.OFR 		+= 	GMAC->OFR;		
//	stat.JR			+= 	GMAC->JR;			
//	stat.FCSE		+= 	GMAC->FCSE;		
//	stat.LFFE		+= 	GMAC->LFFE;		
//	stat.RSE		+= 	GMAC->RSE;		
//	stat.AE			+= 	GMAC->AE;			
//	stat.RRE		+= 	GMAC->RRE;		
//	stat.ROE		+= 	GMAC->ROE;		
//	stat.IHCE		+= 	GMAC->IHCE;		
//	stat.TCE		+= 	GMAC->TCE;		
//	stat.UCE		+= 	GMAC->UCE;		
//}

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

bool TransmitEth(EthBuf *b)
{
	if (b == 0 || b->len < sizeof(EthHdr))
	{
		return false;
	};

	b->eth.src = hwAdr;

	txList.Add(b);

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool TransmitIp(EthIpBuf *b)
{
	if (b == 0 || b->len < sizeof(EthIp))
	{
		return false;
	};

	b->iph.off = 0;		

	b->eth.protlen = SWAP16(PROT_IP);

	b->iph.hl_v = 0x45;	
	b->iph.tos = 0;		
	b->iph.ttl = 64;		
	b->iph.sum = 0;		
	b->iph.src = ipAdr;		
	b->iph.off = ReverseWord(b->iph.off);		
	b->iph.len = ReverseWord(b->len - sizeof(EthHdr));		

	b->iph.sum = IpChkSum((u16*)&b->iph, 10);

	return TransmitEth(b);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool TransmitFragIp(EthIpBuf *b)
{
	if (b == 0 || b->len < sizeof(EthIp))
	{
		return false;
	};

	b->eth.protlen = SWAP16(PROT_IP);

	b->iph.hl_v = 0x45;	
	b->iph.tos = 0;		
	b->iph.ttl = 64;		
	b->iph.sum = 0;		
	b->iph.src = ipAdr;		
	b->iph.off = ReverseWord(b->iph.off);		
	b->iph.len = ReverseWord(b->len - sizeof(EthHdr));		

	b->iph.sum = IpChkSum((u16*)&b->iph, 10);

	return TransmitEth(b);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool TransmitUdp(EthUdpBuf *b)
{
	if (b == 0 || b->len < sizeof(EthUdp))
	{
		return false;
	};

	b->iph.p = PROT_UDP;
	b->udp.src = udpOutPort;
	b->udp.xsum = 0;
	b->udp.len = ReverseWord(b->len - sizeof(EthIp));

	return TransmitIp(b);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool TransmitFragUdp(EthUdpBuf *b, u16 dst)
{
	if (b == 0 || b->len < sizeof(EthUdp))
	{
		return false;
	};

	b->iph.p = PROT_UDP;

	if ((b->iph.off & 0x1FFF) == 0)
	{
		b->udp.dst = dst;
		b->udp.src = udpOutPort;
		b->udp.xsum = 0;
		b->udp.len = ReverseWord((b->iph.off & 0x2000) ? b->udp.len :b->len - sizeof(EthIp));
	};

	return TransmitFragIp(b);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestARP(MEMB* mb)
{
	EthArp *h = (EthArp*)mb->data;

	if (ReverseWord(h->arp.op) == ARP_REQUEST) // ARP REPLY operation
	{     
		if (h->arp.tpa == ipAdr)
		{
			reqArpCount++;

//			Buf_Desc *dsc = GetTxDesc();

			EthBuf *buf = GetSysTxBuffer();

			if (buf == 0) return false;

			EthArp *t = (EthArp*)&buf->eth;

			t->eth.dest = h->eth.src;
//			t->eth.src  = hwAdr;

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

			TransmitEth(buf);
		};	
	};

	return false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestICMP(MEMB* mb)
{
	EthIcmp *h = (EthIcmp*)mb->data;

	if(h->icmp.type == ICMP_ECHO_REQUEST)
	{
		reqIcmpCount++;

//		Buf_Desc *dsc = GetTxDesc();

		EthIpBuf *buf = (EthIpBuf*)GetSysTxBuffer();

		if (/*dsc == 0 ||*/ buf == 0) return false;

		EthIcmp *t = (EthIcmp*)&buf->eth;

		t->eth.dest = h->eth.src;
//		t->eth.src  = hwAdr;

//		t->eth.protlen = SWAP16(PROT_IP);

		//t->iph.hl_v = 0x45;	
		//t->iph.tos = 0;		
		t->iph.len = h->iph.len;		
		t->iph.id = GetIpID(); //h->iph.id;		
		//t->iph.off = 0;		
		//t->iph.ttl = 64;		
		t->iph.p = PROT_ICMP;		
		//t->iph.sum = 0;		
		//t->iph.src = ipAdr;		
		t->iph.dst = h->iph.src;	

		//t->iph.sum = IpChkSum((u16*)&t->iph, 10);

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

		TransmitIp(buf);
	};

	return false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestDHCP(MEMB* mb)
{
	EthDhcp *h = (EthDhcp*)mb->data;

	if (h->dhcp.op != 1) return false;

	i32 optLen = (i32)ReverseWord(h->iph.len) - sizeof(h->iph) - sizeof(h->udp) - 240;

	if (optLen < 3 || h->dhcp.magic != DHCPCOOKIE) return false;

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

	if (!c) return false;

	byte op = h->dhcp.options[i+2];

	if (op != DHCPDISCOVER && op != DHCPREQUEST) return false;

	EthIpBuf *buf = (EthIpBuf*)GetSysTxBuffer();

	if (buf == 0) return false;

	EthDhcp *t = (EthDhcp*)&buf->eth;



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

	u16 ipLen = sizeof(EthDhcp) - sizeof(t->dhcp.options) + (p.b - (byte*)t->dhcp.options);

	t->eth.dest = hwBroadCast;
//	t->eth.src  = hwAdr;

//	t->eth.protlen = ReverseWord(PROT_IP);

	//t->iph.hl_v = 0x45;	
	//t->iph.tos = 0;		
//	t->iph.len = ReverseWord(ipLen);		
	t->iph.id = GetIpID(); //h->iph.id;		
//	t->iph.off = 0;		
//	t->iph.ttl = 64;		
	t->iph.p = PROT_UDP;		
//	t->iph.sum = 0;		
//	t->iph.src = ipAdr;		
	t->iph.dst = -1; //BroadCast	

//	t->iph.sum = IpChkSum((u16*)&t->iph, 10);

	t->udp.src = BOOTPS;
	t->udp.dst = BOOTPC;
	t->udp.len = ReverseWord(ipLen - sizeof(t->iph));
	t->udp.xsum = 0;

	buf->len = ipLen + sizeof(t->eth);

	TransmitIp(buf);

	return false;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestUDP(MEMB* mb)
{
	EthUdp *h = (EthUdp*)mb->data;

	bool c = false;

	switch (h->udp.dst)
	{
		case BOOTPS:	c = RequestDHCP(mb); break;
		case udpInPort: c = RequestTFTP(mb); break;
	};

	reqUdpCount++;

	return c;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool RequestIP(MEMB* mb, u32 stat)
{
	EthIp *h = (EthIp*)mb->data;

	if (h->iph.hl_v != 0x45) return false;

	reqIpCount++;

	bool c = false;

	switch(h->iph.p)
	{
		case PROT_ICMP:	c = RequestICMP(mb);	break; 

		case PROT_UDP:	if ((stat & RD_UDP_ERR) == 0) { c = RequestUDP(mb); };		break;
	};

	return c;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void RecieveFrame()
{
	Receive_Desc &buf = Rx_Desc[RxBufIndex];

	if(buf.stat & RD0_OWN)
	{
//		FreeTxDesc();
	}
	else
	{
		bool c = false;

		MEMB *mb = 0;

		if (buf.addr != 0 && (buf.stat & (RD0_LS|RD0_FS|RD0_CE|RD0_FT)) == (RD0_LS|RD0_FS|RD0_FT)) // buffer contains a whole frame
		{
			mb = (MEMB*)((byte*)buf.addr - sizeof(mb->next));

			EthHdr *eth = (EthHdr*)(buf.addr);

			switch (ReverseWord(eth->protlen))
			{
				case PROT_ARP: // ARP Packet format

					c = RequestARP(mb); break; 

				case PROT_IP:	// IP protocol frame

					if ((buf.stat & RD_IP_ERR) == 0)
					{
						c = RequestIP(mb, buf.stat);
					};

					break;
			};

			if (c)
			{
				buf.addr = 0;
			};
		};

		if (buf.addr == 0)
		{
			MEMB* nmb = AllocMemBuffer();

			if (nmb != 0)
			{
				buf.addr = nmb->data;
				//buf.ctrl = sizeof(nmb->data);
				buf.stat |= RD0_OWN;
			};
		}
		else
		{
			buf.stat |= RD0_OWN;		
		};

		++rxCount;

	//	HW::P5->BTGL(9);

		//Clear RU flag to resume processing
		HW::ETH0->STATUS = ETH_STATUS_RU_Msk;
		//Instruct the DMA to poll the receive descriptor list
		HW::ETH0->RECEIVE_POLL_DEMAND = 0;
	};

	RxBufIndex = (RxBufIndex >= ArraySize(Rx_Desc)) ? 0 : (RxBufIndex+1);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void UpdateTransmit()
{
	static byte i = 0;

	static EthBuf *buf = 0;
	static Transmit_Desc *dsc = 0;

	switch (i)
	{
		case 0:

			if ((buf = txList.Get()) != 0)
			{
				i += 1;
			}
			else
			{
				FreeTxDesc();
			};

			break;

		case 1:

			if ((dsc = GetTxDesc()) != 0)
			{
				//if ((buf->len & TD_LENGTH_MASK) == 0)
				//{
				//	__breakpoint(0);
				//};

				dsc->addr1 = (u32)&buf->eth;
				dsc->addr2 = 0;
				dsc->ctrl = TBS1(buf->len);
				dsc->stat |= TD0_LS|TD0_FS;
				dsc->stat |= TD0_OWN;

				HW::ETH0->MAC_CONFIGURATION |= MAC_TE;
				HW::ETH0->OPERATION_MODE |= ETH_OPERATION_MODE_ST_Msk;

				//Clear TU flag to resume processing
				HW::ETH0->STATUS = ETH_STATUS_TU_Msk;

				//Instruct the DMA to poll the transmit descriptor list
				HW::ETH0->TRANSMIT_POLL_DEMAND = 0;

				i += 1;
			}
			else
			{
				FreeTxDesc();
			};

			break;

		case 2:

			FreeTxDesc();

			i = 0;

			break;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


/*--------------------------- init_ethernet ---------------------------------*/

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

bool InitEMAC()
{

	using namespace HW;
	
	/* Initialize the GMAC ethernet controller. */
	u32 id1,id2;


    //Disable parity error trap
  //  HW::SCU_PARITY->PETE = 0;
    //Disable unaligned access trap
  //  HW::PPB->CCR &= ~PPB_CCR_UNALIGN_TRP_Msk;
 
    //Enable ETH0 peripheral clock
	HW::ETH_Enable();

    HW::PORT15->PDISC &= ~(PORT15_PDISC_PDIS8_Msk | PORT15_PDISC_PDIS9_Msk);
	HW::P2->BSET(10);
	HW::ETH0_CON->CON = CON_INFSEL|CON_RXD0(0)|CON_RXD1(0)|CON_CLK_RMII(0)|CON_CRS_DV(2)|CON_RXER(0)|CON_MDIO(1);

	HW::ETH0->BUS_MODE |= ETH_BUS_MODE_SWR_Msk;

	while (HW::ETH0->BUS_MODE & ETH_BUS_MODE_SWR_Msk);


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

	id1 = ReadPHY(PHY_REG_IDR1);
	id2 = ReadPHY(PHY_REG_IDR2);

	if (((id1 << 16) | (id2 & 0xfff0)) == DP83848C_ID)
	{
	};

	WritePHY(PHY_REG_BMCR,		BMCR_ANENABLE|BMCR_FULLDPLX);
	WritePHY(PHY_REG_ANAR,		ANAR_NPAGE|ANAR_100FULL|ANAR_100HALF|ANAR_10FULL|ANAR_10HALF|ANAR_CSMA);
	WritePHY(PHY_REG_DRC,		DRC_PLL_OFF);
	WritePHY(PHY_REG_OMSO,		OMSO_RMII_OVERRIDE);
	WritePHY(PHY_REG_EXCON,		EXCON_EDPD_EN);
	WritePHY(PHY_REG_PHYCON1,	0);
	WritePHY(PHY_REG_PHYCON2,	PHYCON2_HP_MDIX|PHYCON2_JABBER_EN|PHYCON2_POWER_SAVING);


	// Transmit and Receive disable. 

	HW::ETH0->MAC_CONFIGURATION = MAC_IPC|MAC_DM;


    //Set the MAC address
	HW::ETH0->MAC_ADDRESS0_LOW = hwAdr.B;
	HW::ETH0->MAC_ADDRESS0_HIGH = hwAdr.T;
 
    //Initialize hash table
    HW::ETH0->HASH_TABLE_LOW = 0;
    HW::ETH0->HASH_TABLE_HIGH = 0;
 
    //Configure the receive filter
	HW::ETH0->MAC_FRAME_FILTER = ETH_MAC_FRAME_FILTER_PR_Msk;//ETH_MAC_FRAME_FILTER_HPF_Msk | ETH_MAC_FRAME_FILTER_HMC_Msk;
    //Disable flow control
    HW::ETH0->FLOW_CONTROL = 0;
    //Enable store and forward mode
    //HW::ETH0->OPERATION_MODE = ETH_OPERATION_MODE_RSF_Msk | ETH_OPERATION_MODE_TSF_Msk;
 
    //Configure DMA bus mode
	//HW::ETH0->BUS_MODE = BUS_AAL | BUS_FB | BUS_USP | BUS_RPBL(16) | BUS_PR(0) | BUS_PBL(16);


	/* Initialize Tx and Rx DMA Descriptors */
	rx_descr_init ();
	tx_descr_init ();

   //Prevent interrupts from being generated when statistic counters reach
    //half their maximum value
    ETH0->MMC_TRANSMIT_INTERRUPT_MASK = ~0;
    ETH0->MMC_RECEIVE_INTERRUPT_MASK = ~0;
    ETH0->MMC_IPC_RECEIVE_INTERRUPT_MASK = ~0;
 
    //Disable MAC interrupts
	HW::ETH0->INTERRUPT_MASK = 0x208; // ETH_INTERRUPT_MASK_TSIM_Msk | ETH_INTERRUPT_MASK_PMTIM_Msk;
 
    //Enable the desired DMA interrupts
	HW::ETH0->INTERRUPT_ENABLE = 0; //ETH_INTERRUPT_ENABLE_NIE_Msk | ETH_INTERRUPT_ENABLE_RIE_Msk | ETH_INTERRUPT_ENABLE_TIE_Msk;
 
 
    //Enable MAC transmission and reception
	HW::ETH0->MAC_CONFIGURATION |= MAC_IPC|MAC_DM|MAC_RE; //ETH_MAC_CONFIGURATION_TE_Msk | ETH_MAC_CONFIGURATION_RE_Msk;

    //Enable DMA transmission and reception
	HW::ETH0->OPERATION_MODE |= ETH_OPERATION_MODE_SR_Msk; // | ETH_OPERATION_MODE_ST_Msk;


	StartLink();

	return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool UpdateLink()
{
	bool result = false;

	switch(linkState)
	{
		case 0:		

			ReqWritePHY(PHY_REG_LINKMDCS, LINKMDCS_CABLE_DIAG_EN);

			linkState++;

			break;

		case 1:		

			if (IsReadyPHY())
			{
				ReqReadPHY(PHY_REG_LINKMDCS);

				linkState++;
			};

			break;

		case 2:

			if (IsReadyPHY())
			{
				reg_LINKMDCS = ResultPHY();

				if ((reg_LINKMDCS & LINKMDCS_CABLE_DIAG_EN) == 0)
				{
					if ((reg_LINKMDCS & LINKMDCS_CABLE_DIAG_MASK) == 0)
					{
						emacCableNormal = true;
					}
					else
					{
						emacCableNormal = false;
					};

					ReqReadPHY(PHY_REG_PHYCON1);

					linkState++;
				}
				else
				{
					ReqReadPHY(PHY_REG_LINKMDCS);
				};
			};

			break;

		case 3:

			if (IsReadyPHY())
			{
				reg_PHYCON1 = ResultPHY();

				emacEnergyDetected = reg_PHYCON1 & PHYCON1_ENERGY_DETECT;

				if (reg_PHYCON1 & PHYCON1_LINK_STATUS)
				{
					emacCableNormal = true;

					ReqWritePHY(PHY_REG_BMCR, BMCR_ANENABLE|BMCR_FULLDPLX);

					linkState++;
				}
				else
				{
					ReqReadPHY(PHY_REG_PHYCON1);
				};
			};

			break;

		case 4:

			if (IsReadyPHY())
			{
				ReqReadPHY(PHY_REG_PHYCON1);

				linkState++;
			};

			break;

		case 5:

			if (IsReadyPHY())
			{
				reg_PHYCON1 = ResultPHY();

				if (reg_PHYCON1 & PHYCON1_OP_MODE_MASK /*BMSR_LINKST*/)
				{
					//ReadPHY(PHY_REG_ANLPAR);

					HW::ETH0->MAC_CONFIGURATION &= ~(MAC_DM|MAC_FES);

					if (reg_PHYCON1 & PHYCON1_OP_MODE_100BTX /*ANLPAR_100*/)	// Speed 100Mbit is enabled.
					{
						HW::ETH0->MAC_CONFIGURATION |= MAC_FES;
					};

					if (reg_PHYCON1 & PHYCON1_OP_MODE2 /*ANLPAR_DUPLEX*/)	//  Full duplex is enabled.
					{
						HW::ETH0->MAC_CONFIGURATION |= MAC_DM;
					};

					result = true;

					linkState++;
				}
				else
				{
					ReqReadPHY(PHY_REG_PHYCON1);
				};
			};

			break;
	};

	return result;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static bool CheckLink() // ���� ��� �����, �� ��������� false
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
	static byte i = 0;

	switch(stateEMAC)
	{
		case LINKING:

			if (UpdateLink())
			{
				stateEMAC = CONNECTED;
				HW::ETH0->MAC_CONFIGURATION |= MAC_RE;
				emacConnected = true;
			};
			
			break;

		case CONNECTED:

			if (!CheckLink())
			{
				HW::ETH0->MAC_CONFIGURATION &= ~MAC_RE;
				StartLink();
				stateEMAC = LINKING;
				emacConnected = false;
			}
			else
			{
				switch(i++)
				{
					case 0:	RecieveFrame();		break;
					case 1: UpdateTransmit();	break;
				};

				i &= 1;
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
		MEMB *b = AllocMemBuffer();

		if (b != 0)
		{
			Rx_Desc[i].stat = RD0_OWN;
			Rx_Desc[i].addr = b->data;
		}
		else
		{
			Rx_Desc[i].stat = 0;
			Rx_Desc[i].addr = 0;
		};

		Rx_Desc[i].ctrl = sizeof(b->data);
		Rx_Desc[i].next = 0;
	};

	Rx_Desc[NUM_RX_BUF-1].ctrl |= RD1_RER; // Set the WRAP bit at the end of the list descriptor.

	HW::ETH0->RECEIVE_DESCRIPTOR_LIST_ADDRESS = (u32)Rx_Desc; // Set Rx Queue pointer to descriptor list.
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void tx_descr_init (void)
{
	TxBufIndex = 0;

	for (u32 i = 0; i < NUM_TX_DSC; i++)
	{
		Tx_Desc[i].stat = TD0_IC;
		Tx_Desc[i].addr1 = 0;
		Tx_Desc[i].addr2 = 0;
		Tx_Desc[i].ctrl = 0;
	};
  
	Tx_Desc[NUM_TX_DSC-1].stat |= TD0_TER; // Set the WRAP bit at the end of the list descriptor. 

	HW::ETH0->TRANSMIT_DESCRIPTOR_LIST_ADDRESS = (u32)&Tx_Desc[0]; // Set Tx Queue pointer to descriptor list. 
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


static void WritePHY (byte PhyReg, u16 Value)
{
	HW::ETH0->GMII_DATA = Value;
	HW::ETH0->GMII_ADDRESS = GMII_PA(PHYA)|GMII_CR(0)|GMII_MR(PhyReg)|GMII_MW|GMII_MB;

	while (IsBusyPHY());
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static u16 ReadPHY (byte PhyReg)
{
	HW::ETH0->GMII_ADDRESS = GMII_PA(PHYA)|GMII_CR(0)|GMII_MR(PhyReg)|GMII_MB;

	while (IsBusyPHY());
  
	return ResultPHY();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void ReqWritePHY(byte PhyReg, u16 Value)
{
	HW::ETH0->GMII_DATA = Value;
	HW::ETH0->GMII_ADDRESS = GMII_PA(PHYA)|GMII_CR(0)|GMII_MR(PhyReg)|GMII_MW|GMII_MB;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static void ReqReadPHY(byte PhyReg)
{
	HW::ETH0->GMII_ADDRESS = GMII_PA(PHYA)|GMII_CR(0)|GMII_MR(PhyReg)|GMII_MB;
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





