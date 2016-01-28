#include "xtrap.h"
#include "trap.h"


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct	TrapHdr
{
	u32		counter;
	u16		errors;
	byte	version;
	byte	status;
	byte	device;
};	

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct	Trap
{
	TrapHdr	hdr;
	u16		cmd;
	byte	data[37];
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct	EthTrap
{
	EthUdp	eudp;
	Trap	trap;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct TrapReq
{
	TrapReq	*next;
	
	MAC		srcHA;
	u32		srcIP;
	u16		srcPort;
	u16		dlen;

	Trap	trap;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

struct TrapList
{

protected:

	TrapReq *first;
	TrapReq *last;

  public:

	TrapList() : first(0), last(0) {}

	TrapReq*	Get();
	void		Add(TrapReq* r);
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static TrapReq  traps[10];

static TrapList freeTrapList;
static TrapList reqTrapList;

static MAC ComputerEmacAddr = {0,0};	// Our Ethernet MAC address and IP address
static u32 ComputerIpAddr	= 0;
static u16 ComputerUDPPort	= 0;
static u32 ComputerOldIpAddr	= 0;
static u16 ComputerOldUDPPort	= 0;
static bool ComputerFind = false;


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TrapReq* TrapList::Get()
{
	TrapReq* r = first;

	if (r != 0)
	{
		first = r->next;

		if (first == 0)
		{
			last = 0;
		};

	};

	return r;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void TrapList::Add(TrapReq* r)
{
	if (r == 0)
	{
		return;
	};

	if (last == 0)
	{
		first = last = r;
		r->next = 0;
	}
	else
	{
		last->next = r;
		last = r;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//void HandleRxPacket(EthUdp *h, u32 stat)
//{
////	if((h->iph.hl_v & 0x0F) != 5) { return; }	// Если IP пакет имеет поля `OPTION`, явно не наш пакет
//
//	u16 l = (unsigned short)(ReverseWord(h->udp.len) - sizeof(UdpHdr));
//
//	bool recapture = false;
//
//	unsigned int i;
//
//	if (ComputerEmacAddr != h->eth.src || ComputerIpAddr != h->iph.src || ComputerUDPPort != ReverseWord(h->udp.src)) 
//	{
//		recapture = true;
//	};
//
//	if (recapture || !ComputerFind)
//	{
//		u32 new_ip = h->iph.src;
//		u32 old_ip = ComputerIpAddr;
//		u16 new_port = ReverseWord(h->udp.src);
//		u16 old_port = ComputerUDPPort;
//		
//		if (ComputerFind) 
//		{
//			TRAP_INFO_SendLostIP(new_ip, new_port);
//		}
//		else 
//		{
//			ComputerFind = true;
//		};
//		
//		ComputerEmacAddr = h->eth.src;
//		ComputerIpAddr  = h->iph.src;
//		ComputerUDPPort = h->udp.src;
//
//		TRAP_INFO_SendCaptureIP(old_ip, old_port);
//	}
//     
//	TRAP_HandleRxData((char*)(h) + sizeof(EthUdp), l);
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void RequestTrap(EthUdp *h, u32 stat)
{
	if ((h->udp.len = ReverseWord(h->udp.len)) < 19) return;

	TrapReq* req = freeTrapList.Get();

	if (req == 0) return;

	req->srcHA = h->eth.src;
	req->srcIP = h->iph.src;
	req->srcPort = h->udp.src;
	
	u16 len = req->dlen = h->udp.len - sizeof(h->udp);

	EthTrap *t = (EthTrap*)h;

	u32 *p = (u32*)&t->trap;
	u32 *d = (u32*)&req->trap;

	len = (len+3) >> 2;

	while (len > 0)	{ *d++ = *p++; };

	reqTrapList.Add(req);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void InitTraps()
{
	for (u32 i = 0; i < ArraySize(traps); i++)
	{
		freeTrapList.Add(&traps[i]);
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void UpdateTraps()
{
	static byte i = 0;
	static TrapReq *req = 0;

	switch(i)
	{
		case 0:

			req = reqTrapList.Get();

			if (req != 0)
			{
				i++;
			};

			break;

		case 1:

			if (!ComputerFind)
			{
				i = 3;
			}
			else if (ComputerEmacAddr != req->srcHA || ComputerIpAddr != req->srcIP || ComputerUDPPort != req->srcPort)
			{
				i = 2;
			}
			else
			{
				i = 4;
			};

			break;

		case 2:

			TRAP_INFO_SendLostIP(req->srcIP, ReverseWord(req->srcPort));

			i++;

			break;

		case 3:

			ComputerOldIpAddr	= ComputerIpAddr;
			ComputerOldUDPPort	= ComputerUDPPort;

			ComputerEmacAddr	= req->srcHA;
			ComputerIpAddr		= req->srcIP;
			ComputerUDPPort		= req->srcPort;

			TRAP_INFO_SendCaptureIP(ComputerOldIpAddr, ReverseWord(ComputerOldUDPPort));

			ComputerFind = true;

			i++;

			break;

		case 4:

			TRAP_HandleRxData((char*)(&req->trap), req->dlen);

			i = 0;

			break;
	};
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
