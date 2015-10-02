#ifndef EMAC_H__02_03_2015__16_38
#define EMAC_H__02_03_2015__16_38

#include "types.h"

extern bool	InitEMAC();
extern void	UpdateEMAC();


__packed struct MAC
{
	u32 B;
	u16 T;

	inline void operator=(const MAC &v) { B = v.B; T = v.T; }
};

struct Buf_Desc
{
	u32 addr;
	u32 stat;
};


__packed struct EthHdr
{
	MAC		dest;	/* Destination node		*/
	MAC		src;	/* Source node			*/
	u16		protlen;	/* Protocol or length		*/
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct ArpHdr
{
	u16		hrd;		/* Format of hardware address	*/
	u16		pro;		/* Format of protocol address	*/
	byte	hln;		/* Length of hardware address	*/
	byte	pln;		/* Length of protocol address	*/
	u16		op;			/* Operation			*/
	MAC		sha;		/* Sender hardware address	*/
	u32		spa;		/* Sender protocol address	*/
	MAC		tha;		/* Target hardware address	*/
	u32		tpa;		/* Target protocol address	*/
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//* IP Header structure

__packed struct IPheader
{
	byte	hl_v;		/* header length and version	*/
	byte	tos;		/* type of service		*/
	u16		len;		/* total length			*/
	u16		id;			/* identification		*/
	u16		off;		/* fragment offset field	*/
	byte	ttl;		/* time to live			*/
	byte	p;			/* protocol			*/
	u16		sum;		/* checksum			*/
	u32		src;		/* Source IP address		*/
	u32		dst;		/* Destination IP address	*/
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//* Preudo IP Header
__packed struct IPPseudoheader
{
	u32		srcAdr;	/* Source IP address		*/
	u32		dstAdr;	/* Destination IP address	*/
	byte   	zero;
	byte   	proto;
	u16		size;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//* ICMP echo header structure

__packed struct IcmpEchoHdr
{
	byte	type;       /* type of message */
	byte	code;       /* type subcode */
	u16		cksum;      /* ones complement cksum of struct */
	u16		id;         /* identifier */
	u16		seq;        /* sequence number */
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//* UDP header structure
__packed struct UdpHdr
{
	u16	src;	/* UDP source port		*/
	u16	dst;	/* UDP destination port		*/
	u16	len;	/* Length of UDP packet		*/
	u16	xsum;	/* Checksum			*/
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct EthArp
{
	EthHdr	eth;
	ArpHdr	arp;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct EthIp
{
	EthHdr		eth;
	IPheader	iph;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct EthIcmp
{
	EthHdr		eth;
	IPheader	iph;
	IcmpEchoHdr	icmp;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct EthUdp
{
	EthHdr		eth;
	IPheader	iph;
	UdpHdr		udp;
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct DhcpHdr
{
	byte	op;
	byte	htype;
	byte	hlen;
	byte	hops;

	u32		xid;
	u16		secs;
	u16		flags;

	u32		ciaddr;
	u32		yiaddr;
	u32		siaddr;
	u32		giaddr;

	MAC		chaddr;
	byte	pad_chaddr[10];

	char	sname[64];
	char	file[128];
	u32		magic;
	byte	options[230];
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

__packed struct EthDhcp
{
	EthHdr		eth; // 14
	IPheader	iph; // 20
	UdpHdr		udp; // 8
	DhcpHdr		dhcp;// 
};

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif // EMAC_H__02_03_2015__16_38
