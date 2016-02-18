#ifndef EMAC_DEF_H__13_06_2013__09_47
#define EMAC_DEF_H__13_06_2013__09_47

#include "core.h"


/* EMAC Memory Buffer configuration. */
#define NUM_RX_BUF          8          /* 0x2000 for Rx (64*128=8K)         */
#define ETH_RX_DRBS			8
#define ETH_RX_BUF_SIZE     (ETH_RX_DRBS * 64)       /* EMAC Receive buffer size.         */


#define NUM_TX_DSC          4         /* 0x0600 for Tx                     */
#define ETH_TX_BUF_SIZE     1536        /* EMAC Transmit buffer size         */

#define AT91C_PHY_ADDR      0

#define OWNERSHIP_BIT		1
#define WRAP_BIT			2

#define ARP_REQUEST			0x0001
#define ARP_REPLY			0x0002
#define PROT_ARP			0x0806
#define PROT_IP				0x0800
#define PROT_ICMP			0x01
#define PROT_UDP			17
#define PROT_TCP			6
#define ICMP_ECHO_REQUEST	0x08
#define ICMP_ECHO_REPLY		0x00

#define SWAP16(x)	((u16)((x) << 8) | ((x) >> 8))
#define SWAP32(x)	(((x)>>24) | (((x)&0x00ff0000) >> 8) | (((x)&0x0000ff00) << 8) | (((u32)(x)<<24)))


#define BOOTPS SWAP16(67)	// server DHCP
#define BOOTPC SWAP16(68)	// client DHCP

#define	DHCPDISCOVER	1  
#define	DHCPOFFER		2  
#define	DHCPREQUEST		3  
#define	DHCPDECLINE		4  
#define	DHCPACK			5  
#define	DHCPNAK			6  
#define	DHCPRELEASE		7  
#define DHCPCOOKIE		0x63538263

///* Absolute IO access macros */
//#define pEMAC   AT91C_BASE_EMACB
//#define pPIOA   AT91C_BASE_PIOA
//#define pRSTC   AT91C_BASE_RSTC
//#define pAIC    AT91C_BASE_AIC
//#define pPMC    AT91C_BASE_PMC

#define PHYA 1

#define IP32(b1, b2, b3, b4) (((u32)b1&0xFF)|(((u32)b2&0xFF)<<8)|(((u32)b3&0xFF)<<16)|(((u32)b4&0xFF)<<24))


/* -------- GMAC_NCR : (GMAC Offset: 0x000) Network Control Register -------- */
#define GMAC_LBL (0x1u << 1) /**< \brief (GMAC_NCR) Loop Back Local */
#define GMAC_RXEN (0x1u << 2) /**< \brief (GMAC_NCR) Receive Enable */
#define GMAC_TXEN (0x1u << 3) /**< \brief (GMAC_NCR) Transmit Enable */
#define GMAC_MPE (0x1u << 4) /**< \brief (GMAC_NCR) Management Port Enable */
#define GMAC_CLRSTAT (0x1u << 5) /**< \brief (GMAC_NCR) Clear Statistics Registers */
#define GMAC_INCSTAT (0x1u << 6) /**< \brief (GMAC_NCR) Increment Statistics Registers */
#define GMAC_WESTAT (0x1u << 7) /**< \brief (GMAC_NCR) Write Enable for Statistics Registers */
#define GMAC_BP (0x1u << 8) /**< \brief (GMAC_NCR) Back pressure */
#define GMAC_TSTART (0x1u << 9) /**< \brief (GMAC_NCR) Start Transmission */
#define GMAC_THALT (0x1u << 10) /**< \brief (GMAC_NCR) Transmit Halt */
#define GMAC_TXPF (0x1u << 11) /**< \brief (GMAC_NCR) Transmit Pause Frame */
#define GMAC_TXZQPF (0x1u << 12) /**< \brief (GMAC_NCR) Transmit Zero Quantum Pause Frame */
#define GMAC_RDS (0x1u << 14) /**< \brief (GMAC_NCR) Read Snapshot */
#define GMAC_SRTSM (0x1u << 15) /**< \brief (GMAC_NCR) Store Receive Time Stamp to Memory */
#define GMAC_ENPBPR (0x1u << 16) /**< \brief (GMAC_NCR) Enable PFC Priority-based Pause Reception */
#define GMAC_TXPBPF (0x1u << 17) /**< \brief (GMAC_NCR) Transmit PFC Priority-based Pause Frame */
#define GMAC_FNP (0x1u << 18) /**< \brief (GMAC_NCR) Flush Next Packet */
/* -------- GMAC_NCFGR : (GMAC Offset: 0x004) Network Configuration Register -------- */
#define GMAC_SPD (0x1u << 0) /**< \brief (GMAC_NCFGR) Speed */
#define GMAC_FD (0x1u << 1) /**< \brief (GMAC_NCFGR) Full Duplex */
#define GMAC_DNVLAN (0x1u << 2) /**< \brief (GMAC_NCFGR) Discard Non-VLAN FRAMES */
#define GMAC_JFRAME (0x1u << 3) /**< \brief (GMAC_NCFGR) Jumbo Frame Size */
#define GMAC_CAF (0x1u << 4) /**< \brief (GMAC_NCFGR) Copy All Frames */
#define GMAC_NBC (0x1u << 5) /**< \brief (GMAC_NCFGR) No Broadcast */
#define GMAC_MTIHEN (0x1u << 6) /**< \brief (GMAC_NCFGR) Multicast Hash Enable */
#define GMAC_UNIHEN (0x1u << 7) /**< \brief (GMAC_NCFGR) Unicast Hash Enable */
#define GMAC_MAXFS (0x1u << 8) /**< \brief (GMAC_NCFGR) 1536 Maximum Frame Size */
#define GMAC_RTY (0x1u << 12) /**< \brief (GMAC_NCFGR) Retry Test */
#define GMAC_PEN (0x1u << 13) /**< \brief (GMAC_NCFGR) Pause Enable */
#define GMAC_RXBUFO_P 14
#define GMAC_RXBUFO_M (0x3u << GMAC_RXBUFO_P) /**< \brief (GMAC_NCFGR) Receive Buffer Offset */
#define GMAC_RXBUFO(value) ((GMAC_RXBUFO_M & ((value) << GMAC_NCFGR_RXBUFO_P)))
#define GMAC_LFERD (0x1u << 16) /**< \brief (GMAC_NCFGR) Length Field Error Frame Discard */
#define GMAC_RFCS (0x1u << 17) /**< \brief (GMAC_NCFGR) Remove FCS */
#define GMAC_CLK_P 18
#define GMAC_CLK_M (0x7u << GMAC_CLK_P) /**< \brief (GMAC_NCFGR) MDC CLock Division */
#define   GMAC_CLK_MCK_8 (0x0u << 18) /**< \brief (GMAC_NCFGR) MCK divided by 8 (MCK up to 20 MHz) */
#define   GMAC_CLK_MCK_16 (0x1u << 18) /**< \brief (GMAC_NCFGR) MCK divided by 16 (MCK up to 40 MHz) */
#define   GMAC_CLK_MCK_32 (0x2u << 18) /**< \brief (GMAC_NCFGR) MCK divided by 32 (MCK up to 80 MHz) */
#define   GMAC_CLK_MCK_48 (0x3u << 18) /**< \brief (GMAC_NCFGR) MCK divided by 48 (MCK up to 120MHz) */
#define   GMAC_CLK_MCK_64 (0x4u << 18) /**< \brief (GMAC_NCFGR) MCK divided by 64 (MCK up to 160 MHz) */
#define   GMAC_CLK_MCK_96 (0x5u << 18) /**< \brief (GMAC_NCFGR) MCK divided by 96 (MCK up to 240 MHz) */
#define GMAC_DBW_P 21
#define GMAC_DBW_M (0x3u << GMAC_DBW_P) /**< \brief (GMAC_NCFGR) Data Bus Width */
#define GMAC_DBW(value) ((GMAC_DBW_M & ((value) << GMAC_DBW_P)))
#define GMAC_DCPF (0x1u << 23) /**< \brief (GMAC_NCFGR) Disable Copy of Pause Frames */
#define GMAC_RXCOEN (0x1u << 24) /**< \brief (GMAC_NCFGR) Receive Checksum Offload Enable */
#define GMAC_EFRHD (0x1u << 25) /**< \brief (GMAC_NCFGR) Enable Frames Received in Half Duplex */
#define GMAC_IRXFCS (0x1u << 26) /**< \brief (GMAC_NCFGR) Ignore RX FCS */
#define GMAC_IPGSEN (0x1u << 28) /**< \brief (GMAC_NCFGR) IP Stretch Enable */
#define GMAC_RXBP (0x1u << 29) /**< \brief (GMAC_NCFGR) Receive Bad Preamble */
#define GMAC_IRXER (0x1u << 30) /**< \brief (GMAC_NCFGR) Ignore IPG GRXER */
/* -------- GMAC_NSR : (GMAC Offset: 0x008) Network Status Register -------- */
#define GMAC_MDIO (0x1u << 1) /**< \brief (GMAC_NSR) MDIO Input Status */
#define GMAC_IDLE (0x1u << 2) /**< \brief (GMAC_NSR) PHY Management Logic Idle */
/* -------- GMAC_UR : (GMAC Offset: 0x00C) User Register -------- */
#define GMAC_MII (0x1u << 0) /**< \brief (GMAC_UR)  */
/* -------- GMAC_DCFGR : (GMAC Offset: 0x010) DMA Configuration Register -------- */
#define GMAC_FBLDO_P 0
#define GMAC_FBLDO_M (0x1fu << GMAC_FBLDO_P) /**< \brief (GMAC_DCFGR) Fixed Burst Length for DMA Data Operations: */
#define GMAC_FBLDO_SINGLE (0x1u << 0) /**< \brief (GMAC_DCFGR) 00001: Always use SINGLE AHB bursts */
#define GMAC_FBLDO_INCR4 (0x4u << 0) /**< \brief (GMAC_DCFGR) 001xx: Attempt to use INCR4 AHB bursts (Default) */
#define GMAC_FBLDO_INCR8 (0x8u << 0) /**< \brief (GMAC_DCFGR) 01xxx: Attempt to use INCR8 AHB bursts */
#define GMAC_FBLDO_INCR16 (0x10u << 0) /**< \brief (GMAC_DCFGR) 1xxxx: Attempt to use INCR16 AHB bursts */
#define GMAC_ESMA (0x1u << 6) /**< \brief (GMAC_DCFGR) Endian Swap Mode Enable for Management Descriptor Accesses */
#define GMAC_ESPA (0x1u << 7) /**< \brief (GMAC_DCFGR) Endian Swap Mode Enable for Packet Data Accesses */
#define GMAC_TXCOEN (0x1u << 11) /**< \brief (GMAC_DCFGR) Transmitter Checksum Generation Offload Enable */
#define GMAC_DRBS_P 16
#define GMAC_DRBS_M (0xffu << GMAC_DRBS_P) /**< \brief (GMAC_DCFGR) DMA Receive Buffer Size */
#define GMAC_DRBS(value) ((GMAC_DRBS_M & ((value) << GMAC_DRBS_P)))
/* -------- GMAC_TSR : (GMAC Offset: 0x014) Transmit Status Register -------- */
#define TSR_UBR (0x1u << 0) /**< \brief (GMAC_TSR) Used Bit Read */
#define TSR_COL (0x1u << 1) /**< \brief (GMAC_TSR) Collision Occurred */
#define TSR_RLE (0x1u << 2) /**< \brief (GMAC_TSR) Retry Limit Exceeded */
#define TSR_TXGO (0x1u << 3) /**< \brief (GMAC_TSR) Transmit Go */
#define TSR_TFC (0x1u << 4) /**< \brief (GMAC_TSR) Transmit Frame Corruption due to AHB error */
#define TSR_TXCOMP (0x1u << 5) /**< \brief (GMAC_TSR) Transmit Complete */
#define TSR_UND (0x1u << 6) /**< \brief (GMAC_TSR) Transmit Under Run */
#define TSR_HRESP (0x1u << 8) /**< \brief (GMAC_TSR) HRESP Not OK */
/* -------- GMAC_RBQB : (GMAC Offset: 0x018) Receive Buffer Queue Base Address -------- */
#define GMAC_RBQB_ADDR_Pos 2
#define GMAC_RBQB_ADDR_Msk (0x3fffffffu << GMAC_RBQB_ADDR_Pos) /**< \brief (GMAC_RBQB) Receive buffer queue base address */
#define GMAC_RBQB_ADDR(value) ((GMAC_RBQB_ADDR_Msk & ((value) << GMAC_RBQB_ADDR_Pos)))
/* -------- GMAC_TBQB : (GMAC Offset: 0x01C) Transmit Buffer Queue Base Address -------- */
#define GMAC_TBQB_ADDR_Pos 2
#define GMAC_TBQB_ADDR_Msk (0x3fffffffu << GMAC_TBQB_ADDR_Pos) /**< \brief (GMAC_TBQB) Transmit Buffer Queue Base Address */
#define GMAC_TBQB_ADDR(value) ((GMAC_TBQB_ADDR_Msk & ((value) << GMAC_TBQB_ADDR_Pos)))
/* -------- GMAC_RSR : (GMAC Offset: 0x020) Receive Status Register -------- */
#define RSR_BNA (0x1u << 0) /**< \brief (GMAC_RSR) Buffer Not Available */
#define RSR_REC (0x1u << 1) /**< \brief (GMAC_RSR) Frame Received */
#define RSR_RXOVR (0x1u << 2) /**< \brief (GMAC_RSR) Receive Overrun */
#define RSR_HNO (0x1u << 3) /**< \brief (GMAC_RSR) HRESP Not OK */
/* -------- GMAC_ISR : (GMAC Offset: 0x024) Interrupt Status Register -------- */
#define ISR_MFS	(0x1u << 0) /**< \brief (GMAC_ISR) Management Frame Sent */
#define ISR_RCOMP 	(0x1u << 1) /**< \brief (GMAC_ISR) Receive Complete */
#define ISR_RXUBR 	(0x1u << 2) /**< \brief (GMAC_ISR) RX Used Bit Read */
#define ISR_TXUBR 	(0x1u << 3) /**< \brief (GMAC_ISR) TX Used Bit Read */
#define ISR_TUR	(0x1u << 4) /**< \brief (GMAC_ISR) Transmit Under Run */
#define ISR_RLEX	(0x1u << 5) /**< \brief (GMAC_ISR) Retry Limit Exceeded */
#define ISR_TFCE	(0x1u << 6) /**< \brief (GMAC_ISR) Transmit Frame Corruption due to AHB error */
#define ISR_TCOMP	(0x1u << 7) /**< \brief (GMAC_ISR) Transmit Complete */
#define ISR_ROVR	(0x1u << 10) /**< \brief (GMAC_ISR) Receive Overrun */
#define ISR_HRSP	(0x1u << 11) /**< \brief (GMAC_ISR) HRESP Not OK */
#define ISR_PFNZ	(0x1u << 12) /**< \brief (GMAC_ISR) Pause Frame with Non-zero Pause Quantum Received */
#define ISR_PTZ	(0x1u << 13) /**< \brief (GMAC_ISR) Pause Time Zero */
#define ISR_PFTR	(0x1u << 14) /**< \brief (GMAC_ISR) Pause Frame Transmitted */
#define ISR_DRQFR	(0x1u << 18) /**< \brief (GMAC_ISR) PTP Delay Request Frame Received */
#define ISR_SFR	(0x1u << 19) /**< \brief (GMAC_ISR) PTP Sync Frame Received */
#define ISR_DRQFT	(0x1u << 20) /**< \brief (GMAC_ISR) PTP Delay Request Frame Transmitted */
#define ISR_SFT	(0x1u << 21) /**< \brief (GMAC_ISR) PTP Sync Frame Transmitted */
#define ISR_PDRQFR (0x1u << 22) /**< \brief (GMAC_ISR) PDelay Request Frame Received */
#define ISR_PDRSFR (0x1u << 23) /**< \brief (GMAC_ISR) PDelay Response Frame Received */
#define ISR_PDRQFT (0x1u << 24) /**< \brief (GMAC_ISR) PDelay Request Frame Transmitted */
#define ISR_PDRSFT (0x1u << 25) /**< \brief (GMAC_ISR) PDelay Response Frame Transmitted */
#define ISR_SRI	(0x1u << 26) /**< \brief (GMAC_ISR) TSU Seconds Register Increment */
#define ISR_WOL	(0x1u << 28) /**< \brief (GMAC_ISR) Wake On LAN */
/* -------- GMAC_IER : (GMAC Offset: 0x028) Interrupt Enable Register -------- */
#define GMAC_IER_MFS (0x1u << 0) /**< \brief (GMAC_IER) Management Frame Sent */
#define GMAC_IER_RCOMP (0x1u << 1) /**< \brief (GMAC_IER) Receive Complete */
#define GMAC_IER_RXUBR (0x1u << 2) /**< \brief (GMAC_IER) RX Used Bit Read */
#define GMAC_IER_TXUBR (0x1u << 3) /**< \brief (GMAC_IER) TX Used Bit Read */
#define GMAC_IER_TUR (0x1u << 4) /**< \brief (GMAC_IER) Transmit Under Run */
#define GMAC_IER_RLEX (0x1u << 5) /**< \brief (GMAC_IER) Retry Limit Exceeded or Late Collision */
#define GMAC_IER_TFC (0x1u << 6) /**< \brief (GMAC_IER) Transmit Frame Corruption due to AHB error */
#define GMAC_IER_TCOMP (0x1u << 7) /**< \brief (GMAC_IER) Transmit Complete */
#define GMAC_IER_ROVR (0x1u << 10) /**< \brief (GMAC_IER) Receive Overrun */
#define GMAC_IER_HRESP (0x1u << 11) /**< \brief (GMAC_IER) HRESP Not OK */
#define GMAC_IER_PFNZ (0x1u << 12) /**< \brief (GMAC_IER) Pause Frame with Non-zero Pause Quantum Received */
#define GMAC_IER_PTZ (0x1u << 13) /**< \brief (GMAC_IER) Pause Time Zero */
#define GMAC_IER_PFTR (0x1u << 14) /**< \brief (GMAC_IER) Pause Frame Transmitted */
#define GMAC_IER_EXINT (0x1u << 15) /**< \brief (GMAC_IER) External Interrupt */
#define GMAC_IER_DRQFR (0x1u << 18) /**< \brief (GMAC_IER) PTP Delay Request Frame Received */
#define GMAC_IER_SFR (0x1u << 19) /**< \brief (GMAC_IER) PTP Sync Frame Received */
#define GMAC_IER_DRQFT (0x1u << 20) /**< \brief (GMAC_IER) PTP Delay Request Frame Transmitted */
#define GMAC_IER_SFT (0x1u << 21) /**< \brief (GMAC_IER) PTP Sync Frame Transmitted */
#define GMAC_IER_PDRQFR (0x1u << 22) /**< \brief (GMAC_IER) PDelay Request Frame Received */
#define GMAC_IER_PDRSFR (0x1u << 23) /**< \brief (GMAC_IER) PDelay Response Frame Received */
#define GMAC_IER_PDRQFT (0x1u << 24) /**< \brief (GMAC_IER) PDelay Request Frame Transmitted */
#define GMAC_IER_PDRSFT (0x1u << 25) /**< \brief (GMAC_IER) PDelay Response Frame Transmitted */
#define GMAC_IER_SRI (0x1u << 26) /**< \brief (GMAC_IER) TSU Seconds Register Increment */
#define GMAC_IER_WOL (0x1u << 28) /**< \brief (GMAC_IER) Wake On LAN */
/* -------- GMAC_IDR : (GMAC Offset: 0x02C) Interrupt Disable Register -------- */
#define GMAC_IDR_MFS (0x1u << 0) /**< \brief (GMAC_IDR) Management Frame Sent */
#define GMAC_IDR_RCOMP (0x1u << 1) /**< \brief (GMAC_IDR) Receive Complete */
#define GMAC_IDR_RXUBR (0x1u << 2) /**< \brief (GMAC_IDR) RX Used Bit Read */
#define GMAC_IDR_TXUBR (0x1u << 3) /**< \brief (GMAC_IDR) TX Used Bit Read */
#define GMAC_IDR_TUR (0x1u << 4) /**< \brief (GMAC_IDR) Transmit Under Run */
#define GMAC_IDR_RLEX (0x1u << 5) /**< \brief (GMAC_IDR) Retry Limit Exceeded or Late Collision */
#define GMAC_IDR_TFC (0x1u << 6) /**< \brief (GMAC_IDR) Transmit Frame Corruption due to AHB error */
#define GMAC_IDR_TCOMP (0x1u << 7) /**< \brief (GMAC_IDR) Transmit Complete */
#define GMAC_IDR_ROVR (0x1u << 10) /**< \brief (GMAC_IDR) Receive Overrun */
#define GMAC_IDR_HRESP (0x1u << 11) /**< \brief (GMAC_IDR) HRESP Not OK */
#define GMAC_IDR_PFNZ (0x1u << 12) /**< \brief (GMAC_IDR) Pause Frame with Non-zero Pause Quantum Received */
#define GMAC_IDR_PTZ (0x1u << 13) /**< \brief (GMAC_IDR) Pause Time Zero */
#define GMAC_IDR_PFTR (0x1u << 14) /**< \brief (GMAC_IDR) Pause Frame Transmitted */
#define GMAC_IDR_EXINT (0x1u << 15) /**< \brief (GMAC_IDR) External Interrupt */
#define GMAC_IDR_DRQFR (0x1u << 18) /**< \brief (GMAC_IDR) PTP Delay Request Frame Received */
#define GMAC_IDR_SFR (0x1u << 19) /**< \brief (GMAC_IDR) PTP Sync Frame Received */
#define GMAC_IDR_DRQFT (0x1u << 20) /**< \brief (GMAC_IDR) PTP Delay Request Frame Transmitted */
#define GMAC_IDR_SFT (0x1u << 21) /**< \brief (GMAC_IDR) PTP Sync Frame Transmitted */
#define GMAC_IDR_PDRQFR (0x1u << 22) /**< \brief (GMAC_IDR) PDelay Request Frame Received */
#define GMAC_IDR_PDRSFR (0x1u << 23) /**< \brief (GMAC_IDR) PDelay Response Frame Received */
#define GMAC_IDR_PDRQFT (0x1u << 24) /**< \brief (GMAC_IDR) PDelay Request Frame Transmitted */
#define GMAC_IDR_PDRSFT (0x1u << 25) /**< \brief (GMAC_IDR) PDelay Response Frame Transmitted */
#define GMAC_IDR_SRI (0x1u << 26) /**< \brief (GMAC_IDR) TSU Seconds Register Increment */
#define GMAC_IDR_WOL (0x1u << 28) /**< \brief (GMAC_IDR) Wake On LAN */
/* -------- GMAC_IMR : (GMAC Offset: 0x030) Interrupt Mask Register -------- */
#define GMAC_IMR_MFS (0x1u << 0) /**< \brief (GMAC_IMR) Management Frame Sent */
#define GMAC_IMR_RCOMP (0x1u << 1) /**< \brief (GMAC_IMR) Receive Complete */
#define GMAC_IMR_RXUBR (0x1u << 2) /**< \brief (GMAC_IMR) RX Used Bit Read */
#define GMAC_IMR_TXUBR (0x1u << 3) /**< \brief (GMAC_IMR) TX Used Bit Read */
#define GMAC_IMR_TUR (0x1u << 4) /**< \brief (GMAC_IMR) Transmit Under Run */
#define GMAC_IMR_RLEX (0x1u << 5) /**< \brief (GMAC_IMR) Retry Limit Exceeded */
#define GMAC_IMR_TFC (0x1u << 6) /**< \brief (GMAC_IMR) Transmit Frame Corruption due to AHB error */
#define GMAC_IMR_TCOMP (0x1u << 7) /**< \brief (GMAC_IMR) Transmit Complete */
#define GMAC_IMR_ROVR (0x1u << 10) /**< \brief (GMAC_IMR) Receive Overrun */
#define GMAC_IMR_HRESP (0x1u << 11) /**< \brief (GMAC_IMR) HRESP Not OK */
#define GMAC_IMR_PFNZ (0x1u << 12) /**< \brief (GMAC_IMR) Pause Frame with Non-zero Pause Quantum Received */
#define GMAC_IMR_PTZ (0x1u << 13) /**< \brief (GMAC_IMR) Pause Time Zero */
#define GMAC_IMR_PFTR (0x1u << 14) /**< \brief (GMAC_IMR) Pause Frame Transmitted */
#define GMAC_IMR_EXINT (0x1u << 15) /**< \brief (GMAC_IMR) External Interrupt */
#define GMAC_IMR_DRQFR (0x1u << 18) /**< \brief (GMAC_IMR) PTP Delay Request Frame Received */
#define GMAC_IMR_SFR (0x1u << 19) /**< \brief (GMAC_IMR) PTP Sync Frame Received */
#define GMAC_IMR_DRQFT (0x1u << 20) /**< \brief (GMAC_IMR) PTP Delay Request Frame Transmitted */
#define GMAC_IMR_SFT (0x1u << 21) /**< \brief (GMAC_IMR) PTP Sync Frame Transmitted */
#define GMAC_IMR_PDRQFR (0x1u << 22) /**< \brief (GMAC_IMR) PDelay Request Frame Received */
#define GMAC_IMR_PDRSFR (0x1u << 23) /**< \brief (GMAC_IMR) PDelay Response Frame Received */
#define GMAC_IMR_PDRQFT (0x1u << 24) /**< \brief (GMAC_IMR) PDelay Request Frame Transmitted */
#define GMAC_IMR_PDRSFT (0x1u << 25) /**< \brief (GMAC_IMR) PDelay Response Frame Transmitted */
/* -------- GMAC_MAN : (GMAC Offset: 0x034) PHY Maintenance Register -------- */
#define GMAC_MAN_DATA_Pos 0
#define GMAC_MAN_DATA_Msk (0xffffu << GMAC_MAN_DATA_Pos) /**< \brief (GMAC_MAN) PHY Data */
#define GMAC_MAN_DATA(value) ((GMAC_MAN_DATA_Msk & ((value) << GMAC_MAN_DATA_Pos)))
#define GMAC_MAN_WTN_Pos 16
#define GMAC_MAN_WTN_Msk (0x3u << GMAC_MAN_WTN_Pos) /**< \brief (GMAC_MAN) Write Ten */
#define GMAC_MAN_WTN(value) ((GMAC_MAN_WTN_Msk & ((value) << GMAC_MAN_WTN_Pos)))
#define GMAC_MAN_REGA_Pos 18
#define GMAC_MAN_REGA_Msk (0x1fu << GMAC_MAN_REGA_Pos) /**< \brief (GMAC_MAN) Register Address */
#define GMAC_MAN_REGA(value) ((GMAC_MAN_REGA_Msk & ((value) << GMAC_MAN_REGA_Pos)))
#define GMAC_MAN_PHYA_Pos 23
#define GMAC_MAN_PHYA_Msk (0x1fu << GMAC_MAN_PHYA_Pos) /**< \brief (GMAC_MAN) PHY Address */
#define GMAC_MAN_PHYA(value) ((GMAC_MAN_PHYA_Msk & ((value) << GMAC_MAN_PHYA_Pos)))
#define GMAC_MAN_OP_Pos 28
#define GMAC_MAN_OP_Msk (0x3u << GMAC_MAN_OP_Pos) /**< \brief (GMAC_MAN) Operation */
#define GMAC_MAN_OP(value) ((GMAC_MAN_OP_Msk & ((value) << GMAC_MAN_OP_Pos)))
#define GMAC_MAN_CLTTO (0x1u << 30) /**< \brief (GMAC_MAN) Clause 22 Operation */
#define GMAC_MAN_WZO (0x1u << 31) /**< \brief (GMAC_MAN) Write ZERO */
/* -------- GMAC_RPQ : (GMAC Offset: 0x038) Received Pause Quantum Register -------- */
#define GMAC_RPQ_RPQ_Pos 0
#define GMAC_RPQ_RPQ_Msk (0xffffu << GMAC_RPQ_RPQ_Pos) /**< \brief (GMAC_RPQ) Received Pause Quantum */
/* -------- GMAC_TPQ : (GMAC Offset: 0x03C) Transmit Pause Quantum Register -------- */
#define GMAC_TPQ_TPQ_Pos 0
#define GMAC_TPQ_TPQ_Msk (0xffffu << GMAC_TPQ_TPQ_Pos) /**< \brief (GMAC_TPQ) Transmit Pause Quantum */
#define GMAC_TPQ_TPQ(value) ((GMAC_TPQ_TPQ_Msk & ((value) << GMAC_TPQ_TPQ_Pos)))
/* -------- GMAC_HRB : (GMAC Offset: 0x080) Hash Register Bottom [31:0] -------- */
#define GMAC_HRB_ADDR_Pos 0
#define GMAC_HRB_ADDR_Msk (0xffffffffu << GMAC_HRB_ADDR_Pos) /**< \brief (GMAC_HRB) Hash Address */
#define GMAC_HRB_ADDR(value) ((GMAC_HRB_ADDR_Msk & ((value) << GMAC_HRB_ADDR_Pos)))
/* -------- GMAC_HRT : (GMAC Offset: 0x084) Hash Register Top [63:32] -------- */
#define GMAC_HRT_ADDR_Pos 0
#define GMAC_HRT_ADDR_Msk (0xffffffffu << GMAC_HRT_ADDR_Pos) /**< \brief (GMAC_HRT) Hash Address */
#define GMAC_HRT_ADDR(value) ((GMAC_HRT_ADDR_Msk & ((value) << GMAC_HRT_ADDR_Pos)))
/* -------- GMAC_SAB1 : (GMAC Offset: 0x088) Specific Address 1 Bottom [31:0] Register -------- */
#define GMAC_SAB1_ADDR_Pos 0
#define GMAC_SAB1_ADDR_Msk (0xffffffffu << GMAC_SAB1_ADDR_Pos) /**< \brief (GMAC_SAB1) Specific Address 1 */
#define GMAC_SAB1_ADDR(value) ((GMAC_SAB1_ADDR_Msk & ((value) << GMAC_SAB1_ADDR_Pos)))
/* -------- GMAC_SAT1 : (GMAC Offset: 0x08C) Specific Address 1 Top [47:32] Register -------- */
#define GMAC_SAT1_ADDR_Pos 0
#define GMAC_SAT1_ADDR_Msk (0xffffu << GMAC_SAT1_ADDR_Pos) /**< \brief (GMAC_SAT1) Specific Address 1 */
#define GMAC_SAT1_ADDR(value) ((GMAC_SAT1_ADDR_Msk & ((value) << GMAC_SAT1_ADDR_Pos)))
/* -------- GMAC_SAB2 : (GMAC Offset: 0x090) Specific Address 2 Bottom [31:0] Register -------- */
#define GMAC_SAB2_ADDR_Pos 0
#define GMAC_SAB2_ADDR_Msk (0xffffffffu << GMAC_SAB2_ADDR_Pos) /**< \brief (GMAC_SAB2) Specific Address 2 */
#define GMAC_SAB2_ADDR(value) ((GMAC_SAB2_ADDR_Msk & ((value) << GMAC_SAB2_ADDR_Pos)))
/* -------- GMAC_SAT2 : (GMAC Offset: 0x094) Specific Address 2 Top [47:32] Register -------- */
#define GMAC_SAT2_ADDR_Pos 0
#define GMAC_SAT2_ADDR_Msk (0xffffu << GMAC_SAT2_ADDR_Pos) /**< \brief (GMAC_SAT2) Specific Address 2 */
#define GMAC_SAT2_ADDR(value) ((GMAC_SAT2_ADDR_Msk & ((value) << GMAC_SAT2_ADDR_Pos)))
/* -------- GMAC_SAB3 : (GMAC Offset: 0x098) Specific Address 3 Bottom [31:0] Register -------- */
#define GMAC_SAB3_ADDR_Pos 0
#define GMAC_SAB3_ADDR_Msk (0xffffffffu << GMAC_SAB3_ADDR_Pos) /**< \brief (GMAC_SAB3) Specific Address 3 */
#define GMAC_SAB3_ADDR(value) ((GMAC_SAB3_ADDR_Msk & ((value) << GMAC_SAB3_ADDR_Pos)))
/* -------- GMAC_SAT3 : (GMAC Offset: 0x09C) Specific Address 3 Top [47:32] Register -------- */
#define GMAC_SAT3_ADDR_Pos 0
#define GMAC_SAT3_ADDR_Msk (0xffffu << GMAC_SAT3_ADDR_Pos) /**< \brief (GMAC_SAT3) Specific Address 3 */
#define GMAC_SAT3_ADDR(value) ((GMAC_SAT3_ADDR_Msk & ((value) << GMAC_SAT3_ADDR_Pos)))
/* -------- GMAC_SAB4 : (GMAC Offset: 0x0A0) Specific Address 4 Bottom [31:0] Register -------- */
#define GMAC_SAB4_ADDR_Pos 0
#define GMAC_SAB4_ADDR_Msk (0xffffffffu << GMAC_SAB4_ADDR_Pos) /**< \brief (GMAC_SAB4) Specific Address 4 */
#define GMAC_SAB4_ADDR(value) ((GMAC_SAB4_ADDR_Msk & ((value) << GMAC_SAB4_ADDR_Pos)))
/* -------- GMAC_SAT4 : (GMAC Offset: 0x0A4) Specific Address 4 Top [47:32] Register -------- */
#define GMAC_SAT4_ADDR_Pos 0
#define GMAC_SAT4_ADDR_Msk (0xffffu << GMAC_SAT4_ADDR_Pos) /**< \brief (GMAC_SAT4) Specific Address 4 */
#define GMAC_SAT4_ADDR(value) ((GMAC_SAT4_ADDR_Msk & ((value) << GMAC_SAT4_ADDR_Pos)))
/* -------- GMAC_TIDM[4] : (GMAC Offset: 0x0A8) Type ID Match 1 Register -------- */
#define GMAC_TIDM_TID_Pos 0
#define GMAC_TIDM_TID_Msk (0xffffu << GMAC_TIDM_TID_Pos) /**< \brief (GMAC_TIDM[4]) Type ID Match 1 */
#define GMAC_TIDM_TID(value) ((GMAC_TIDM_TID_Msk & ((value) << GMAC_TIDM_TID_Pos)))
/* -------- GMAC_IPGS : (GMAC Offset: 0x0BC) IPG Stretch Register -------- */
#define GMAC_IPGS_FL_Pos 0
#define GMAC_IPGS_FL_Msk (0xffffu << GMAC_IPGS_FL_Pos) /**< \brief (GMAC_IPGS) Frame Length */
#define GMAC_IPGS_FL(value) ((GMAC_IPGS_FL_Msk & ((value) << GMAC_IPGS_FL_Pos)))
/* -------- GMAC_SVLAN : (GMAC Offset: 0x0C0) Stacked VLAN Register -------- */
#define GMAC_SVLAN_VLAN_TYPE_Pos 0
#define GMAC_SVLAN_VLAN_TYPE_Msk (0xffffu << GMAC_SVLAN_VLAN_TYPE_Pos) /**< \brief (GMAC_SVLAN) User Defined VLAN_TYPE Field */
#define GMAC_SVLAN_VLAN_TYPE(value) ((GMAC_SVLAN_VLAN_TYPE_Msk & ((value) << GMAC_SVLAN_VLAN_TYPE_Pos)))
#define GMAC_SVLAN_ESVLAN (0x1u << 31) /**< \brief (GMAC_SVLAN) Enable Stacked VLAN Processing Mode */
/* -------- GMAC_TPFCP : (GMAC Offset: 0x0C4) Transmit PFC Pause Register -------- */
#define GMAC_TPFCP_PEV_Pos 0
#define GMAC_TPFCP_PEV_Msk (0xffu << GMAC_TPFCP_PEV_Pos) /**< \brief (GMAC_TPFCP) Priority Enable Vector */
#define GMAC_TPFCP_PEV(value) ((GMAC_TPFCP_PEV_Msk & ((value) << GMAC_TPFCP_PEV_Pos)))
#define GMAC_TPFCP_PQ_Pos 8
#define GMAC_TPFCP_PQ_Msk (0xffu << GMAC_TPFCP_PQ_Pos) /**< \brief (GMAC_TPFCP) Pause Quantum */
#define GMAC_TPFCP_PQ(value) ((GMAC_TPFCP_PQ_Msk & ((value) << GMAC_TPFCP_PQ_Pos)))
/* -------- GMAC_SAMB1 : (GMAC Offset: 0x0C8) Specific Address 1 Mask Bottom [31:0] Register -------- */
#define GMAC_SAMB1_ADDR_Pos 0
#define GMAC_SAMB1_ADDR_Msk (0xffffffffu << GMAC_SAMB1_ADDR_Pos) /**< \brief (GMAC_SAMB1) Specific Address 1 Mask */
#define GMAC_SAMB1_ADDR(value) ((GMAC_SAMB1_ADDR_Msk & ((value) << GMAC_SAMB1_ADDR_Pos)))
/* -------- GMAC_SAMT1 : (GMAC Offset: 0x0CC) Specific Address 1 Mask Top [47:32] Register -------- */
#define GMAC_SAMT1_ADDR_Pos 0
#define GMAC_SAMT1_ADDR_Msk (0xffffu << GMAC_SAMT1_ADDR_Pos) /**< \brief (GMAC_SAMT1) Specific Address 1 Mask */
#define GMAC_SAMT1_ADDR(value) ((GMAC_SAMT1_ADDR_Msk & ((value) << GMAC_SAMT1_ADDR_Pos)))
/* -------- GMAC_OTLO : (GMAC Offset: 0x100) Octets Transmitted [31:0] Register -------- */
#define GMAC_OTLO_TXO_Pos 0
#define GMAC_OTLO_TXO_Msk (0xffffffffu << GMAC_OTLO_TXO_Pos) /**< \brief (GMAC_OTLO) Transmitted Octets */
/* -------- GMAC_OTHI : (GMAC Offset: 0x104) Octets Transmitted [47:32] Register -------- */
#define GMAC_OTHI_TXO_Pos 0
#define GMAC_OTHI_TXO_Msk (0xffffu << GMAC_OTHI_TXO_Pos) /**< \brief (GMAC_OTHI) Transmitted Octets */
/* -------- GMAC_FT : (GMAC Offset: 0x108) Frames Transmitted Register -------- */
#define GMAC_FT_FTX_Pos 0
#define GMAC_FT_FTX_Msk (0xffffffffu << GMAC_FT_FTX_Pos) /**< \brief (GMAC_FT) Frames Transmitted without Error */
/* -------- GMAC_BCFT : (GMAC Offset: 0x10C) Broadcast Frames Transmitted Register -------- */
#define GMAC_BCFT_BFTX_Pos 0
#define GMAC_BCFT_BFTX_Msk (0xffffffffu << GMAC_BCFT_BFTX_Pos) /**< \brief (GMAC_BCFT) Broadcast Frames Transmitted without Error */
/* -------- GMAC_MFT : (GMAC Offset: 0x110) Multicast Frames Transmitted Register -------- */
#define GMAC_MFT_MFTX_Pos 0
#define GMAC_MFT_MFTX_Msk (0xffffffffu << GMAC_MFT_MFTX_Pos) /**< \brief (GMAC_MFT) Multicast Frames Transmitted without Error */
/* -------- GMAC_PFT : (GMAC Offset: 0x114) Pause Frames Transmitted Register -------- */
#define GMAC_PFT_PFTX_Pos 0
#define GMAC_PFT_PFTX_Msk (0xffffu << GMAC_PFT_PFTX_Pos) /**< \brief (GMAC_PFT) Pause Frames Transmitted Register */
/* -------- GMAC_BFT64 : (GMAC Offset: 0x118) 64 Byte Frames Transmitted Register -------- */
#define GMAC_BFT64_NFTX_Pos 0
#define GMAC_BFT64_NFTX_Msk (0xffffffffu << GMAC_BFT64_NFTX_Pos) /**< \brief (GMAC_BFT64) 64 Byte Frames Transmitted without Error */
/* -------- GMAC_TBFT127 : (GMAC Offset: 0x11C) 65 to 127 Byte Frames Transmitted Register -------- */
#define GMAC_TBFT127_NFTX_Pos 0
#define GMAC_TBFT127_NFTX_Msk (0xffffffffu << GMAC_TBFT127_NFTX_Pos) /**< \brief (GMAC_TBFT127) 65 to 127 Byte Frames Transmitted without Error */
/* -------- GMAC_TBFT255 : (GMAC Offset: 0x120) 128 to 255 Byte Frames Transmitted Register -------- */
#define GMAC_TBFT255_NFTX_Pos 0
#define GMAC_TBFT255_NFTX_Msk (0xffffffffu << GMAC_TBFT255_NFTX_Pos) /**< \brief (GMAC_TBFT255) 128 to 255 Byte Frames Transmitted without Error */
/* -------- GMAC_TBFT511 : (GMAC Offset: 0x124) 256 to 511 Byte Frames Transmitted Register -------- */
#define GMAC_TBFT511_NFTX_Pos 0
#define GMAC_TBFT511_NFTX_Msk (0xffffffffu << GMAC_TBFT511_NFTX_Pos) /**< \brief (GMAC_TBFT511) 256 to 511 Byte Frames Transmitted without Error */
/* -------- GMAC_TBFT1023 : (GMAC Offset: 0x128) 512 to 1023 Byte Frames Transmitted Register -------- */
#define GMAC_TBFT1023_NFTX_Pos 0
#define GMAC_TBFT1023_NFTX_Msk (0xffffffffu << GMAC_TBFT1023_NFTX_Pos) /**< \brief (GMAC_TBFT1023) 512 to 1023 Byte Frames Transmitted without Error */
/* -------- GMAC_TBFT1518 : (GMAC Offset: 0x12C) 1024 to 1518 Byte Frames Transmitted Register -------- */
#define GMAC_TBFT1518_NFTX_Pos 0
#define GMAC_TBFT1518_NFTX_Msk (0xffffffffu << GMAC_TBFT1518_NFTX_Pos) /**< \brief (GMAC_TBFT1518) 1024 to 1518 Byte Frames Transmitted without Error */
/* -------- GMAC_GTBFT1518 : (GMAC Offset: 0x130) Greater Than 1518 Byte Frames Transmitted Register -------- */
#define GMAC_GTBFT1518_NFTX_Pos 0
#define GMAC_GTBFT1518_NFTX_Msk (0xffffffffu << GMAC_GTBFT1518_NFTX_Pos) /**< \brief (GMAC_GTBFT1518) Greater than 1518 Byte Frames Transmitted without Error */
/* -------- GMAC_TUR : (GMAC Offset: 0x134) Transmit Under Runs Register -------- */
#define GMAC_TUR_TXUNR_Pos 0
#define GMAC_TUR_TXUNR_Msk (0x3ffu << GMAC_TUR_TXUNR_Pos) /**< \brief (GMAC_TUR) Transmit Under Runs */
/* -------- GMAC_SCF : (GMAC Offset: 0x138) Single Collision Frames Register -------- */
#define GMAC_SCF_SCOL_Pos 0
#define GMAC_SCF_SCOL_Msk (0x3ffffu << GMAC_SCF_SCOL_Pos) /**< \brief (GMAC_SCF) Single Collision */
/* -------- GMAC_MCF : (GMAC Offset: 0x13C) Multiple Collision Frames Register -------- */
#define GMAC_MCF_MCOL_Pos 0
#define GMAC_MCF_MCOL_Msk (0x3ffffu << GMAC_MCF_MCOL_Pos) /**< \brief (GMAC_MCF) Multiple Collision */
/* -------- GMAC_EC : (GMAC Offset: 0x140) Excessive Collisions Register -------- */
#define GMAC_EC_XCOL_Pos 0
#define GMAC_EC_XCOL_Msk (0x3ffu << GMAC_EC_XCOL_Pos) /**< \brief (GMAC_EC) Excessive Collisions */
/* -------- GMAC_LC : (GMAC Offset: 0x144) Late Collisions Register -------- */
#define GMAC_LC_LCOL_Pos 0
#define GMAC_LC_LCOL_Msk (0x3ffu << GMAC_LC_LCOL_Pos) /**< \brief (GMAC_LC) Late Collisions */
/* -------- GMAC_DTF : (GMAC Offset: 0x148) Deferred Transmission Frames Register -------- */
#define GMAC_DTF_DEFT_Pos 0
#define GMAC_DTF_DEFT_Msk (0x3ffffu << GMAC_DTF_DEFT_Pos) /**< \brief (GMAC_DTF) Deferred Transmission */
/* -------- GMAC_CSE : (GMAC Offset: 0x14C) Carrier Sense Errors Register -------- */
#define GMAC_CSE_CSR_Pos 0
#define GMAC_CSE_CSR_Msk (0x3ffu << GMAC_CSE_CSR_Pos) /**< \brief (GMAC_CSE) Carrier Sense Error */
/* -------- GMAC_ORLO : (GMAC Offset: 0x150) Octets Received [31:0] Received -------- */
#define GMAC_ORLO_RXO_Pos 0
#define GMAC_ORLO_RXO_Msk (0xffffffffu << GMAC_ORLO_RXO_Pos) /**< \brief (GMAC_ORLO) Received Octets */
/* -------- GMAC_ORHI : (GMAC Offset: 0x154) Octets Received [47:32] Received -------- */
#define GMAC_ORHI_RXO_Pos 0
#define GMAC_ORHI_RXO_Msk (0xffffu << GMAC_ORHI_RXO_Pos) /**< \brief (GMAC_ORHI) Received Octets */
/* -------- GMAC_FR : (GMAC Offset: 0x158) Frames Received Register -------- */
#define GMAC_FR_FRX_Pos 0
#define GMAC_FR_FRX_Msk (0xffffffffu << GMAC_FR_FRX_Pos) /**< \brief (GMAC_FR) Frames Received without Error */
/* -------- GMAC_BCFR : (GMAC Offset: 0x15C) Broadcast Frames Received Register -------- */
#define GMAC_BCFR_BFRX_Pos 0
#define GMAC_BCFR_BFRX_Msk (0xffffffffu << GMAC_BCFR_BFRX_Pos) /**< \brief (GMAC_BCFR) Broadcast Frames Received without Error */
/* -------- GMAC_MFR : (GMAC Offset: 0x160) Multicast Frames Received Register -------- */
#define GMAC_MFR_MFRX_Pos 0
#define GMAC_MFR_MFRX_Msk (0xffffffffu << GMAC_MFR_MFRX_Pos) /**< \brief (GMAC_MFR) Multicast Frames Received without Error */
/* -------- GMAC_PFR : (GMAC Offset: 0x164) Pause Frames Received Register -------- */
#define GMAC_PFR_PFRX_Pos 0
#define GMAC_PFR_PFRX_Msk (0xffffu << GMAC_PFR_PFRX_Pos) /**< \brief (GMAC_PFR) Pause Frames Received Register */
/* -------- GMAC_BFR64 : (GMAC Offset: 0x168) 64 Byte Frames Received Register -------- */
#define GMAC_BFR64_NFRX_Pos 0
#define GMAC_BFR64_NFRX_Msk (0xffffffffu << GMAC_BFR64_NFRX_Pos) /**< \brief (GMAC_BFR64) 64 Byte Frames Received without Error */
/* -------- GMAC_TBFR127 : (GMAC Offset: 0x16C) 65 to 127 Byte Frames Received Register -------- */
#define GMAC_TBFR127_NFRX_Pos 0
#define GMAC_TBFR127_NFRX_Msk (0xffffffffu << GMAC_TBFR127_NFRX_Pos) /**< \brief (GMAC_TBFR127) 65 to 127 Byte Frames Received without Error */
/* -------- GMAC_TBFR255 : (GMAC Offset: 0x170) 128 to 255 Byte Frames Received Register -------- */
#define GMAC_TBFR255_NFRX_Pos 0
#define GMAC_TBFR255_NFRX_Msk (0xffffffffu << GMAC_TBFR255_NFRX_Pos) /**< \brief (GMAC_TBFR255) 128 to 255 Byte Frames Received without Error */
/* -------- GMAC_TBFR511 : (GMAC Offset: 0x174) 256 to 511Byte Frames Received Register -------- */
#define GMAC_TBFR511_NFRX_Pos 0
#define GMAC_TBFR511_NFRX_Msk (0xffffffffu << GMAC_TBFR511_NFRX_Pos) /**< \brief (GMAC_TBFR511) 256 to 511 Byte Frames Received without Error */
/* -------- GMAC_TBFR1023 : (GMAC Offset: 0x178) 512 to 1023 Byte Frames Received Register -------- */
#define GMAC_TBFR1023_NFRX_Pos 0
#define GMAC_TBFR1023_NFRX_Msk (0xffffffffu << GMAC_TBFR1023_NFRX_Pos) /**< \brief (GMAC_TBFR1023) 512 to 1023 Byte Frames Received without Error */
/* -------- GMAC_TBFR1518 : (GMAC Offset: 0x17C) 1024 to 1518 Byte Frames Received Register -------- */
#define GMAC_TBFR1518_NFRX_Pos 0
#define GMAC_TBFR1518_NFRX_Msk (0xffffffffu << GMAC_TBFR1518_NFRX_Pos) /**< \brief (GMAC_TBFR1518) 1024 to 1518 Byte Frames Received without Error */
/* -------- GMAC_TMXBFR : (GMAC Offset: 0x180) 1519 to Maximum Byte Frames Received Register -------- */
#define GMAC_TMXBFR_NFRX_Pos 0
#define GMAC_TMXBFR_NFRX_Msk (0xffffffffu << GMAC_TMXBFR_NFRX_Pos) /**< \brief (GMAC_TMXBFR) 1519 to Maximum Byte Frames Received without Error */
/* -------- GMAC_UFR : (GMAC Offset: 0x184) Undersize Frames Received Register -------- */
#define GMAC_UFR_UFRX_Pos 0
#define GMAC_UFR_UFRX_Msk (0x3ffu << GMAC_UFR_UFRX_Pos) /**< \brief (GMAC_UFR) Undersize Frames Received */
/* -------- GMAC_OFR : (GMAC Offset: 0x188) Oversize Frames Received Register -------- */
#define GMAC_OFR_OFRX_Pos 0
#define GMAC_OFR_OFRX_Msk (0x3ffu << GMAC_OFR_OFRX_Pos) /**< \brief (GMAC_OFR) Oversized Frames Received */
/* -------- GMAC_JR : (GMAC Offset: 0x18C) Jabbers Received Register -------- */
#define GMAC_JR_JRX_Pos 0
#define GMAC_JR_JRX_Msk (0x3ffu << GMAC_JR_JRX_Pos) /**< \brief (GMAC_JR) Jabbers Received */
/* -------- GMAC_FCSE : (GMAC Offset: 0x190) Frame Check Sequence Errors Register -------- */
#define GMAC_FCSE_FCKR_Pos 0
#define GMAC_FCSE_FCKR_Msk (0x3ffu << GMAC_FCSE_FCKR_Pos) /**< \brief (GMAC_FCSE) Frame Check Sequence Errors */
/* -------- GMAC_LFFE : (GMAC Offset: 0x194) Length Field Frame Errors Register -------- */
#define GMAC_LFFE_LFER_Pos 0
#define GMAC_LFFE_LFER_Msk (0x3ffu << GMAC_LFFE_LFER_Pos) /**< \brief (GMAC_LFFE) Length Field Frame Errors */
/* -------- GMAC_RSE : (GMAC Offset: 0x198) Receive Symbol Errors Register -------- */
#define GMAC_RSE_RXSE_Pos 0
#define GMAC_RSE_RXSE_Msk (0x3ffu << GMAC_RSE_RXSE_Pos) /**< \brief (GMAC_RSE) Receive Symbol Errors */
/* -------- GMAC_AE : (GMAC Offset: 0x19C) Alignment Errors Register -------- */
#define GMAC_AE_AER_Pos 0
#define GMAC_AE_AER_Msk (0x3ffu << GMAC_AE_AER_Pos) /**< \brief (GMAC_AE) Alignment Errors */
/* -------- GMAC_RRE : (GMAC Offset: 0x1A0) Receive Resource Errors Register -------- */
#define GMAC_RRE_RXRER_Pos 0
#define GMAC_RRE_RXRER_Msk (0x3ffffu << GMAC_RRE_RXRER_Pos) /**< \brief (GMAC_RRE) Receive Resource Errors */
/* -------- GMAC_ROE : (GMAC Offset: 0x1A4) Receive Overrun Register -------- */
#define GMAC_ROE_RXOVR_Pos 0
#define GMAC_ROE_RXOVR_Msk (0x3ffu << GMAC_ROE_RXOVR_Pos) /**< \brief (GMAC_ROE) Receive Overruns */
/* -------- GMAC_IHCE : (GMAC Offset: 0x1A8) IP Header Checksum Errors Register -------- */
#define GMAC_IHCE_HCKER_Pos 0
#define GMAC_IHCE_HCKER_Msk (0xffu << GMAC_IHCE_HCKER_Pos) /**< \brief (GMAC_IHCE) IP Header Checksum Errors */
/* -------- GMAC_TCE : (GMAC Offset: 0x1AC) TCP Checksum Errors Register -------- */
#define GMAC_TCE_TCKER_Pos 0
#define GMAC_TCE_TCKER_Msk (0xffu << GMAC_TCE_TCKER_Pos) /**< \brief (GMAC_TCE) TCP Checksum Errors */
/* -------- GMAC_UCE : (GMAC Offset: 0x1B0) UDP Checksum Errors Register -------- */
#define GMAC_UCE_UCKER_Pos 0
#define GMAC_UCE_UCKER_Msk (0xffu << GMAC_UCE_UCKER_Pos) /**< \brief (GMAC_UCE) UDP Checksum Errors */
/* -------- GMAC_TSSS : (GMAC Offset: 0x1C8) 1588 Timer Sync Strobe Seconds Register -------- */
#define GMAC_TSSS_VTS_Pos 0
#define GMAC_TSSS_VTS_Msk (0xffffffffu << GMAC_TSSS_VTS_Pos) /**< \brief (GMAC_TSSS) Value of Timer Seconds Register Capture */
#define GMAC_TSSS_VTS(value) ((GMAC_TSSS_VTS_Msk & ((value) << GMAC_TSSS_VTS_Pos)))
/* -------- GMAC_TSSN : (GMAC Offset: 0x1CC) 1588 Timer Sync Strobe Nanoseconds Register -------- */
#define GMAC_TSSN_VTN_Pos 0
#define GMAC_TSSN_VTN_Msk (0x3fffffffu << GMAC_TSSN_VTN_Pos) /**< \brief (GMAC_TSSN) Value Timer Nanoseconds Register Capture */
#define GMAC_TSSN_VTN(value) ((GMAC_TSSN_VTN_Msk & ((value) << GMAC_TSSN_VTN_Pos)))
/* -------- GMAC_TS : (GMAC Offset: 0x1D0) 1588 Timer Seconds Register -------- */
#define GMAC_TS_TCS_Pos 0
#define GMAC_TS_TCS_Msk (0xffffffffu << GMAC_TS_TCS_Pos) /**< \brief (GMAC_TS) Timer Count in Seconds */
#define GMAC_TS_TCS(value) ((GMAC_TS_TCS_Msk & ((value) << GMAC_TS_TCS_Pos)))
/* -------- GMAC_TN : (GMAC Offset: 0x1D4) 1588 Timer Nanoseconds Register -------- */
#define GMAC_TN_TNS_Pos 0
#define GMAC_TN_TNS_Msk (0x3fffffffu << GMAC_TN_TNS_Pos) /**< \brief (GMAC_TN) Timer Count in Nanoseconds */
#define GMAC_TN_TNS(value) ((GMAC_TN_TNS_Msk & ((value) << GMAC_TN_TNS_Pos)))
/* -------- GMAC_TA : (GMAC Offset: 0x1D8) 1588 Timer Adjust Register -------- */
#define GMAC_TA_ITDT_Pos 0
#define GMAC_TA_ITDT_Msk (0x3fffffffu << GMAC_TA_ITDT_Pos) /**< \brief (GMAC_TA) Increment/Decrement */
#define GMAC_TA_ITDT(value) ((GMAC_TA_ITDT_Msk & ((value) << GMAC_TA_ITDT_Pos)))
#define GMAC_TA_ADJ (0x1u << 31) /**< \brief (GMAC_TA) Adjust 1588 Timer */
/* -------- GMAC_TI : (GMAC Offset: 0x1DC) 1588 Timer Increment Register -------- */
#define GMAC_TI_CNS_Pos 0
#define GMAC_TI_CNS_Msk (0xffu << GMAC_TI_CNS_Pos) /**< \brief (GMAC_TI) Count Nanoseconds */
#define GMAC_TI_CNS(value) ((GMAC_TI_CNS_Msk & ((value) << GMAC_TI_CNS_Pos)))
#define GMAC_TI_ACNS_Pos 8
#define GMAC_TI_ACNS_Msk (0xffu << GMAC_TI_ACNS_Pos) /**< \brief (GMAC_TI) Alternative Count Nanoseconds */
#define GMAC_TI_ACNS(value) ((GMAC_TI_ACNS_Msk & ((value) << GMAC_TI_ACNS_Pos)))
#define GMAC_TI_NIT_Pos 16
#define GMAC_TI_NIT_Msk (0xffu << GMAC_TI_NIT_Pos) /**< \brief (GMAC_TI) Number of Increments */
#define GMAC_TI_NIT(value) ((GMAC_TI_NIT_Msk & ((value) << GMAC_TI_NIT_Pos)))
/* -------- GMAC_EFTS : (GMAC Offset: 0x1E0) PTP Event Frame Transmitted Seconds -------- */
#define GMAC_EFTS_RUD_Pos 0
#define GMAC_EFTS_RUD_Msk (0xffffffffu << GMAC_EFTS_RUD_Pos) /**< \brief (GMAC_EFTS) Register Update */
/* -------- GMAC_EFTN : (GMAC Offset: 0x1E4) PTP Event Frame Transmitted Nanoseconds -------- */
#define GMAC_EFTN_RUD_Pos 0
#define GMAC_EFTN_RUD_Msk (0x3fffffffu << GMAC_EFTN_RUD_Pos) /**< \brief (GMAC_EFTN) Register Update */
/* -------- GMAC_EFRS : (GMAC Offset: 0x1E8) PTP Event Frame Received Seconds -------- */
#define GMAC_EFRS_RUD_Pos 0
#define GMAC_EFRS_RUD_Msk (0xffffffffu << GMAC_EFRS_RUD_Pos) /**< \brief (GMAC_EFRS) Register Update */
/* -------- GMAC_EFRN : (GMAC Offset: 0x1EC) PTP Event Frame Received Nanoseconds -------- */
#define GMAC_EFRN_RUD_Pos 0
#define GMAC_EFRN_RUD_Msk (0x3fffffffu << GMAC_EFRN_RUD_Pos) /**< \brief (GMAC_EFRN) Register Update */
/* -------- GMAC_PEFTS : (GMAC Offset: 0x1F0) PTP Peer Event Frame Transmitted Seconds -------- */
#define GMAC_PEFTS_RUD_Pos 0
#define GMAC_PEFTS_RUD_Msk (0xffffffffu << GMAC_PEFTS_RUD_Pos) /**< \brief (GMAC_PEFTS) Register Update */
/* -------- GMAC_PEFTN : (GMAC Offset: 0x1F4) PTP Peer Event Frame Transmitted Nanoseconds -------- */
#define GMAC_PEFTN_RUD_Pos 0
#define GMAC_PEFTN_RUD_Msk (0x3fffffffu << GMAC_PEFTN_RUD_Pos) /**< \brief (GMAC_PEFTN) Register Update */
/* -------- GMAC_PEFRS : (GMAC Offset: 0x1F8) PTP Peer Event Frame Received Seconds -------- */
#define GMAC_PEFRS_RUD_Pos 0
#define GMAC_PEFRS_RUD_Msk (0xffffffffu << GMAC_PEFRS_RUD_Pos) /**< \brief (GMAC_PEFRS) Register Update */
/* -------- GMAC_PEFRN : (GMAC Offset: 0x1FC) PTP Peer Event Frame Received Nanoseconds -------- */
#define GMAC_PEFRN_RUD_Pos 0
#define GMAC_PEFRN_RUD_Msk (0x3fffffffu << GMAC_PEFRN_RUD_Pos) /**< \brief (GMAC_PEFRN) Register Update */







#if 0

/* -------- EMAC_NCR : (EMAC Offset: 0x00) Network Control Register -------- */
#define EMAC_LB			(1u << 0)			/**< \brief (EMAC_NCR) LoopBack */
#define EMAC_LLB		(1u << 1)		/**< \brief (EMAC_NCR) Loopback local */
#define EMAC_RE			(1u << 2)			/**< \brief (EMAC_NCR) Receive enable */
#define EMAC_TE			(1u << 3)			/**< \brief (EMAC_NCR) Transmit enable */
#define EMAC_MPE		(1u << 4)		/**< \brief (EMAC_NCR) Management port enable */
#define EMAC_CLRSTAT 	(1u << 5)	/**< \brief (EMAC_NCR) Clear statistics registers */
#define EMAC_INCSTAT 	(1u << 6)	/**< \brief (EMAC_NCR) Increment statistics registers */
#define EMAC_WESTAT		(1u << 7)		/**< \brief (EMAC_NCR) Write enable for statistics registers */
#define EMAC_BP			(1u << 8)			/**< \brief (EMAC_NCR) Back pressure */
#define EMAC_TSTART		(1u << 9)		/**< \brief (EMAC_NCR) Start transmission */
#define EMAC_THALT		(1u << 10)		/**< \brief (EMAC_NCR) Transmit halt */
/* -------- EMAC_NCFGR : (EMAC Offset: 0x04) Network Configuration Register -------- */
#define EMAC_SPD (0x1u << 0) /**< \brief (EMAC_NCFGR) Speed */
#define EMAC_FD (0x1u << 1) /**< \brief (EMAC_NCFGR) Full Duplex */
#define EMAC_JFRAME (0x1u << 3) /**< \brief (EMAC_NCFGR) Jumbo Frames */
#define EMAC_CAF (0x1u << 4) /**< \brief (EMAC_NCFGR) Copy All Frames */
#define EMAC_NBC (0x1u << 5) /**< \brief (EMAC_NCFGR) No Broadcast */
#define EMAC_MTI (0x1u << 6) /**< \brief (EMAC_NCFGR) Multicast Hash Enable */
#define EMAC_UNI (0x1u << 7) /**< \brief (EMAC_NCFGR) Unicast Hash Enable */
#define EMAC_BIG (0x1u << 8) /**< \brief (EMAC_NCFGR) Receive 1536 bytes frames */
#define EMAC_CLK_Pos 10
#define EMAC_CLK_Msk (0x3u << EMAC_NCFGR_CLK_Pos) /**< \brief (EMAC_NCFGR) MDC clock divider */
#define	EMAC_CLK_HCLK_8 (0x0u << 10) /**< \brief (EMAC_NCFGR) MCK divided by 8 (MCK up to 20 MHz). */
#define	EMAC_CLK_HCLK_16 (0x1u << 10) /**< \brief (EMAC_NCFGR) MCK divided by 16 (MCK up to 40 MHz). */
#define	EMAC_CLK_HCLK_32 (0x2u << 10) /**< \brief (EMAC_NCFGR) MCK divided by 32 (MCK up to 80 MHz). */
#define	EMAC_CLK_HCLK_64 (0x3u << 10) /**< \brief (EMAC_NCFGR) MCK divided by 64 (MCK up to 160 MHz). */
#define EMAC_RTY (0x1u << 12) /**< \brief (EMAC_NCFGR) Retry test */
#define EMAC_PAE (0x1u << 13) /**< \brief (EMAC_NCFGR) Pause Enable */
#define EMAC_RBOF_Pos 14
#define EMAC_RBOF_Msk (0x3u << EMAC_NCFGR_RBOF_Pos) /**< \brief (EMAC_NCFGR) Receive Buffer Offset */
#define	EMAC_RBOF_OFFSET_0 (0x0u << 14) /**< \brief (EMAC_NCFGR) No offset from start of receive buffer. */
#define	EMAC_RBOF_OFFSET_1 (0x1u << 14) /**< \brief (EMAC_NCFGR) One-byte offset from start of receive buffer. */
#define	EMAC_RBOF_OFFSET_2 (0x2u << 14) /**< \brief (EMAC_NCFGR) Two-byte offset from start of receive buffer. */
#define	EMAC_RBOF_OFFSET_3 (0x3u << 14) /**< \brief (EMAC_NCFGR) Three-byte offset from start of receive buffer. */
#define EMAC_RLCE (0x1u << 16) /**< \brief (EMAC_NCFGR) Receive Length field Checking Enable */
#define EMAC_DRFCS (0x1u << 17) /**< \brief (EMAC_NCFGR) Discard Receive FCS */
#define EMAC_EFRHD (0x1u << 18) /**< \brief (EMAC_NCFGR)  */
#define EMAC_IRXFCS (0x1u << 19) /**< \brief (EMAC_NCFGR) Ignore RX FCS */
/* -------- EMAC_NSR : (EMAC Offset: 0x08) Network Status Register -------- */
#define EMAC_MDIO (0x1u << 1) /**< \brief (EMAC_NSR)  */
#define EMAC_IDLE (0x1u << 2) /**< \brief (EMAC_NSR)  */
/* -------- EMAC_TSR : (EMAC Offset: 0x14) Transmit Status Register -------- */
#define EMAC_UBR (0x1u << 0) /**< \brief (EMAC_TSR) Used Bit Read */
#define EMAC_COL (0x1u << 1) /**< \brief (EMAC_TSR) Collision Occurred */
#define EMAC_RLES (0x1u << 2) /**< \brief (EMAC_TSR) Retry Limit exceeded */
#define EMAC_TGO (0x1u << 3) /**< \brief (EMAC_TSR) Transmit Go */
#define EMAC_BEX (0x1u << 4) /**< \brief (EMAC_TSR) Buffers exhausted mid frame */
#define EMAC_COMP (0x1u << 5) /**< \brief (EMAC_TSR) Transmit Complete */
#define EMAC_UND (0x1u << 6) /**< \brief (EMAC_TSR) Transmit Underrun */
/* -------- EMAC_RSR : (EMAC Offset: 0x20) Receive Status Register -------- */
#define EMAC_BNA (0x1u << 0) /**< \brief (EMAC_RSR) Buffer Not Available */
#define EMAC_REC (0x1u << 1) /**< \brief (EMAC_RSR) Frame Received */
#define EMAC_OVR (0x1u << 2) /**< \brief (EMAC_RSR) Receive Overrun */
/* -------- EMAC_ISR : (EMAC Offset: 0x24) Interrupt Status Register -------- */
#define EMAC_MFD (0x1u << 0) /**< \brief (EMAC_ISR) Management Frame Done */
#define EMAC_RCOMP (0x1u << 1) /**< \brief (EMAC_ISR) Receive Complete */
#define EMAC_RXUBR (0x1u << 2) /**< \brief (EMAC_ISR) Receive Used Bit Read */
#define EMAC_TXUBR (0x1u << 3) /**< \brief (EMAC_ISR) Transmit Used Bit Read */
#define EMAC_TUND (0x1u << 4) /**< \brief (EMAC_ISR) Ethernet Transmit Buffer Underrun */
#define EMAC_RLEX (0x1u << 5) /**< \brief (EMAC_ISR) Retry Limit Exceeded */
#define EMAC_TXERR (0x1u << 6) /**< \brief (EMAC_ISR) Transmit Error */
#define EMAC_TCOMP (0x1u << 7) /**< \brief (EMAC_ISR) Transmit Complete */
#define EMAC_ROVR (0x1u << 10) /**< \brief (EMAC_ISR) Receive Overrun */
#define EMAC_HRESP (0x1u << 11) /**< \brief (EMAC_ISR) Hresp not OK */
#define EMAC_PFRE (0x1u << 12) /**< \brief (EMAC_ISR) Pause Frame Received */
#define EMAC_PTZ (0x1u << 13) /**< \brief (EMAC_ISR) Pause Time Zero */
/* -------- EMAC_IER : (EMAC Offset: 0x28) Interrupt Enable Register -------- */
#define EMAC_MFD (0x1u << 0) /**< \brief (EMAC_IER) Management Frame sent */
#define EMAC_RCOMP (0x1u << 1) /**< \brief (EMAC_IER) Receive Complete */
#define EMAC_RXUBR (0x1u << 2) /**< \brief (EMAC_IER) Receive Used Bit Read */
#define EMAC_TXUBR (0x1u << 3) /**< \brief (EMAC_IER) Transmit Used Bit Read */
#define EMAC_TUND (0x1u << 4) /**< \brief (EMAC_IER) Ethernet Transmit Buffer Underrun */
#define EMAC_RLE (0x1u << 5) /**< \brief (EMAC_IER) Retry Limit Exceeded */
#define EMAC_TXERR (0x1u << 6) /**< \brief (EMAC_IER)  */
#define EMAC_TCOMP (0x1u << 7) /**< \brief (EMAC_IER) Transmit Complete */
#define EMAC_ROVR (0x1u << 10) /**< \brief (EMAC_IER) Receive Overrun */
#define EMAC_HRESP (0x1u << 11) /**< \brief (EMAC_IER) Hresp not OK */
#define EMAC_PFR (0x1u << 12) /**< \brief (EMAC_IER) Pause Frame Received */
#define EMAC_PTZ (0x1u << 13) /**< \brief (EMAC_IER) Pause Time Zero */
/* -------- EMAC_IDR : (EMAC Offset: 0x2C) Interrupt Disable Register -------- */
#define EMAC_MFD (0x1u << 0) /**< \brief (EMAC_IDR) Management Frame sent */
#define EMAC_RCOMP (0x1u << 1) /**< \brief (EMAC_IDR) Receive Complete */
#define EMAC_RXUBR (0x1u << 2) /**< \brief (EMAC_IDR) Receive Used Bit Read */
#define EMAC_TXUBR (0x1u << 3) /**< \brief (EMAC_IDR) Transmit Used Bit Read */
#define EMAC_TUND (0x1u << 4) /**< \brief (EMAC_IDR) Ethernet Transmit Buffer Underrun */
#define EMAC_RLE (0x1u << 5) /**< \brief (EMAC_IDR) Retry Limit Exceeded */
#define EMAC_TXERR (0x1u << 6) /**< \brief (EMAC_IDR)  */
#define EMAC_TCOMP (0x1u << 7) /**< \brief (EMAC_IDR) Transmit Complete */
#define EMAC_ROVR (0x1u << 10) /**< \brief (EMAC_IDR) Receive Overrun */
#define EMAC_HRESP (0x1u << 11) /**< \brief (EMAC_IDR) Hresp not OK */
#define EMAC_PFR (0x1u << 12) /**< \brief (EMAC_IDR) Pause Frame Received */
#define EMAC_PTZ (0x1u << 13) /**< \brief (EMAC_IDR) Pause Time Zero */
/* -------- EMAC_IMR : (EMAC Offset: 0x30) Interrupt Mask Register -------- */
#define EMAC_MFD (0x1u << 0) /**< \brief (EMAC_IMR) Management Frame sent */
#define EMAC_RCOMP (0x1u << 1) /**< \brief (EMAC_IMR) Receive Complete */
#define EMAC_RXUBR (0x1u << 2) /**< \brief (EMAC_IMR) Receive Used Bit Read */
#define EMAC_TXUBR (0x1u << 3) /**< \brief (EMAC_IMR) Transmit Used Bit Read */
#define EMAC_TUND (0x1u << 4) /**< \brief (EMAC_IMR) Ethernet Transmit Buffer Underrun */
#define EMAC_RLE (0x1u << 5) /**< \brief (EMAC_IMR) Retry Limit Exceeded */
#define EMAC_TXERR (0x1u << 6) /**< \brief (EMAC_IMR)  */
#define EMAC_TCOMP (0x1u << 7) /**< \brief (EMAC_IMR) Transmit Complete */
#define EMAC_ROVR (0x1u << 10) /**< \brief (EMAC_IMR) Receive Overrun */
#define EMAC_HRESP (0x1u << 11) /**< \brief (EMAC_IMR) Hresp not OK */
#define EMAC_PFR (0x1u << 12) /**< \brief (EMAC_IMR) Pause Frame Received */
#define EMAC_PTZ (0x1u << 13) /**< \brief (EMAC_IMR) Pause Time Zero */
/* -------- EMAC_MAN : (EMAC Offset: 0x34) Phy Maintenance Register -------- */
#define EMAC_DATA_Pos 0
#define EMAC_DATA_Msk (0xffffu << EMAC_MAN_DATA_Pos) /**< \brief (EMAC_MAN)  */
#define EMAC_DATA(value) ((EMAC_MAN_DATA_Msk & ((value) << EMAC_MAN_DATA_Pos)))
#define EMAC_CODE_Pos 16
#define EMAC_CODE_Msk (0x3u << EMAC_MAN_CODE_Pos) /**< \brief (EMAC_MAN)  */
#define EMAC_CODE(value) ((EMAC_MAN_CODE_Msk & ((value) << EMAC_MAN_CODE_Pos)))
#define EMAC_REGA_Pos 18
#define EMAC_REGA_Msk (0x1fu << EMAC_MAN_REGA_Pos) /**< \brief (EMAC_MAN) Register Address */
#define EMAC_REGA(value) ((EMAC_MAN_REGA_Msk & ((value) << EMAC_MAN_REGA_Pos)))
#define EMAC_PHYA_Pos 23
#define EMAC_PHYA_Msk (0x1fu << EMAC_MAN_PHYA_Pos) /**< \brief (EMAC_MAN) PHY Address */
#define EMAC_PHYA(value) ((EMAC_MAN_PHYA_Msk & ((value) << EMAC_MAN_PHYA_Pos)))
#define EMAC_RW_Pos 28
#define EMAC_RW_Msk (0x3u << EMAC_MAN_RW_Pos) /**< \brief (EMAC_MAN) Read-write */
#define EMAC_RW(value) ((EMAC_MAN_RW_Msk & ((value) << EMAC_MAN_RW_Pos)))
#define EMAC_SOF_Pos 30
#define EMAC_SOF_Msk (0x3u << EMAC_MAN_SOF_Pos) /**< \brief (EMAC_MAN) Start of frame */
#define EMAC_SOF(value) ((EMAC_MAN_SOF_Msk & ((value) << EMAC_MAN_SOF_Pos)))
/* -------- EMAC_USRIO : (EMAC Offset: 0xC0) User Input/Output Register -------- */
#define EMAC_RMII (0x1u << 0) /**< \brief (EMAC_USRIO) Reduce MII */
#define EMAC_CLKEN (0x1u << 1) /**< \brief (EMAC_USRIO) Clock Enable */

#endif


/* Receive status defintion */
#define RD_BROADCAST_ADDR   (1U << 31)  /* Broadcat address detected         */
#define RD_MULTICAST_HASH   (1U << 30)  /* MultiCast hash match              */
#define RD_UNICAST_HASH     (1U << 29)  /* UniCast hash match                */
//#define RD_EXTERNAL_ADDR    (1U << 28)  /* External Address match            */
#define RD_SARMF	        (1U << 27)  /* Specific address 1 match          */
#define RD_SA_MATCH         (3U << 25)  /* Specific address 2 match          */
#define RD_TYPE_ID          (3U << 22)  /* Type ID match                     */
#define RD_IP_CHECK         (3U << 22)  /* Type ID match                     */
#define RD_IP_OK			(1U << 22)  /* Type ID match                     */
#define RD_IP_TCP_OK		(2U << 22)  /* Type ID match                     */
#define RD_IP_UDP_OK		(3U << 22)  /* Type ID match                     */
#define RD_VLAN_TAG         (1U << 21)  /* VLAN tag detected                 */
#define RD_PRIORITY_TAG     (1U << 20)  /* PRIORITY tag detected             */
#define RD_VLAN_PRIORITY    (7U << 17)  /* PRIORITY Mask                     */
#define RD_CFI_IND          (1U << 16)  /* CFI indicator                     */
#define RD_EOF              (1U << 15)  /* EOF                               */
#define RD_SOF              (1U << 14)  /* SOF                               */
#define RD_BAD_FCS			(1U << 13)  /* Receive Buffer Offset Mask        */
#define RD_LENGTH_MASK      0x1FFF      /* Length of frame mask              */

/* Transmit Status definition */
#define TD_TRANSMIT_OK      (1U << 31)  /* Transmit OK                       */
#define TD_TRANSMIT_WRAP    (1U << 30)  /* Wrap bit: mark the last descriptor*/
#define TD_TRANSMIT_ERR     (1U << 29)  /* RLE:transmit error                */
#define TD_TRANSMIT_UND     (1U << 28)  /* Transmit Underrun                 */
#define TD_BUF_EX           (1U << 27)  /* Buffers exhausted in mid frame    */
#define TD_TRANSMIT_NO_CRC  (1U << 16)  /* No CRC will be appended to frame  */
#define TD_LAST_BUF         (1U << 15)  /* Last buffer in TX frame           */
#define TD_LENGTH_MASK      0x1FFF      /* Length of frame mask              */

#define AT91C_OWNERSHIP_BIT 0x00000001  /* Buffer owned by software          */

/* DP83848C PHY Registers */
#define PHY_REG_BMCR        0x00        /* Basic Mode Control Register       */
#define PHY_REG_BMSR        0x01        /* Basic Mode Status Register        */
#define PHY_REG_IDR1        0x02        /* PHY Identifier 1                  */
#define PHY_REG_IDR2        0x03        /* PHY Identifier 2                  */
#define PHY_REG_ANAR        0x04        /* Auto-Negotiation Advertisement    */
#define PHY_REG_ANLPAR      0x05        /* Auto-Neg. Link Partner Abitily    */
#define PHY_REG_ANER        0x06        /* Auto-Neg. Expansion Register      */
#define PHY_REG_ANNPTR      0x07        /* Auto-Neg. Next Page TX            */

/* PHY Extended Registers */
#define PHY_REG_PHYSTS      0x10        /* PHY Status Register               */
#define PHY_REG_MICR        0x11        /* MII Interrupt Control Register    */
#define PHY_REG_MISR        0x12        /* MII Interrupt Status Register     */
#define PHY_REG_FCSCR       0x14        /* False Carrier Sense Counter       */
#define PHY_REG_RECR        0x15        /* Receive Error Counter             */
#define PHY_REG_PCSR        0x16        /* PCS Sublayer Config. and Status   */
#define PHY_REG_RBR         0x17        /* RMII and Bypass Register          */
#define PHY_REG_LEDCR       0x18        /* LED Direct Control Register       */
#define PHY_REG_PHYCR       0x19        /* PHY Control Register              */
#define PHY_REG_10BTSCR     0x1A        /* 10Base-T Status/Control Register  */
#define PHY_REG_CDCTRL1     0x1B        /* CD Test Control and BIST Extens.  */
#define PHY_REG_EDCR        0x1D        /* Energy Detect Control Register    */

#define PHY_FULLD_100M      0x2100      /* Full Duplex 100Mbit               */
#define PHY_HALFD_100M      0x2000      /* Half Duplex 100Mbit               */
#define PHY_FULLD_10M       0x0100      /* Full Duplex 10Mbit                */
#define PHY_HALFD_10M       0x0000      /* Half Duplex 10MBit                */
#define PHY_AUTO_NEG        0x3000      /* Select Auto Negotiation           */

#define PHYSTS_LINKST		0x0001		/* Link status                       */
#define PHYSTS_SPEEDST		0x0002		/* Speed status: 1=10Mb, 0=100Mb     */
#define PHYSTS_DUPLEXST		0x0004		/* Duplex: 1=Full, 0=Half			 */


#define DP83848C_DEF_ADR    0x0100      /* Default PHY device address        */
#define DP83848C_ID         0x20005C90  /* PHY Identifier                    */


/* Basic mode control register. */
#define BMCR_RESV           0x007f      /* Unused...                         */
#define BMCR_CTST           0x0080      /* Collision test                    */
#define BMCR_FULLDPLX       0x0100      /* Full duplex                       */
#define BMCR_ANRESTART      0x0200      /* Auto negotiation restart          */
#define BMCR_ISOLATE        0x0400      /* Disconnect DM9161 from MII        */
#define BMCR_PDOWN          0x0800      /* Powerdown the DM9161              */
#define BMCR_ANENABLE       0x1000      /* Enable auto negotiation           */
#define BMCR_SPEED100       0x2000      /* Select 100Mbps                    */
#define BMCR_LOOPBACK       0x4000      /* TXD loopback bits                 */
#define BMCR_RESET          0x8000      /* Reset the DM9161                  */

/* Basic mode status register. */
#define BMSR_ERCAP          0x0001      /* Ext-reg capability                */
#define BMSR_JCD            0x0002      /* Jabber detected                   */
#define BMSR_LINKST         0x0004      /* Link status                       */
#define BMSR_ANEGCAPABLE    0x0008      /* Able to do auto-negotiation       */
#define BMSR_RFAULT         0x0010      /* Remote fault detected             */
#define BMSR_ANEGCOMPLETE   0x0020      /* Auto-negotiation complete         */
#define BMSR_MIIPRESUP      0x0040      /* MII Frame Preamble Suppression    */
#define BMSR_RESV           0x0780      /* Unused...                         */
#define BMSR_10HALF         0x0800      /* Can do 10mbps, half-duplex        */
#define BMSR_10FULL         0x1000      /* Can do 10mbps, full-duplex        */
#define BMSR_100HALF        0x2000      /* Can do 100mbps, half-duplex       */
#define BMSR_100FULL        0x4000      /* Can do 100mbps, full-duplex       */
#define BMSR_100BASE4       0x8000      /* Can do 100mbps, 4k packets        */

/* Advertisement control register. */
#define ANAR_SLCT           0x001f      /* Selector bits                     */
#define ANAR_CSMA           0x0001      /* Only selector supported           */
#define ANAR_10HALF         0x0020      /* Try for 10mbps half-duplex        */
#define ANAR_10FULL         0x0040      /* Try for 10mbps full-duplex        */
#define ANAR_100HALF        0x0080      /* Try for 100mbps half-duplex       */
#define ANAR_100FULL        0x0100      /* Try for 100mbps full-duplex       */
#define ANAR_100BASE4       0x0200      /* Try for 100mbps 4k packets        */
#define ANAR_FCS            0x0400      /* Try for Flow Control Support      */
#define ANAR_RESV           0x1800      /* Unused...                         */
#define ANAR_RFAULT         0x2000      /* Say we can detect faults          */
#define ANAR_LPACK          0x4000      /* Ack link partners response        */
#define ANAR_NPAGE          0x8000      /* Next page bit                     */

#define ANAR_FULL           (ANAR_100FULL | ANAR_10FULL | ANAR_CSMA)
#define ANAR_ALL            (ANAR_10HALF  | ANAR_10FULL | ANAR_100HALF | ANAR_100FULL)

/* Link partner ability register. */
#define ANLPAR_SLCT         0x001f      /* Same as advertise selector        */
#define ANLPAR_10HALF       0x0020      /* Can do 10mbps half-duplex         */
#define ANLPAR_10FULL       0x0040      /* Can do 10mbps full-duplex         */
#define ANLPAR_100HALF      0x0080      /* Can do 100mbps half-duplex        */
#define ANLPAR_100FULL      0x0100      /* Can do 100mbps full-duplex        */
#define ANLPAR_100BASE4     0x0200      /* Can do 100mbps 4k packets         */
#define ANLPAR_FCS          0x0400      /* Can do Flow Control Support       */
#define ANLPAR_RESV         0x1800      /* Unused...                         */
#define ANLPAR_RFAULT       0x2000      /* Link partner faulted              */
#define ANLPAR_LPACK        0x4000      /* Link partner acked us             */
#define ANLPAR_NPAGE        0x8000      /* Next page bit                     */

#define ANLPAR_DUPLEX       (ANLPAR_10FULL  | ANLPAR_100FULL)
#define ANLPAR_100          (ANLPAR_100FULL | ANLPAR_100HALF | ANLPAR_100BASE4)

/* Expansion register for auto-negotiation. */
#define ANER_LPANABLE       0x0001      /* Link Partner AutoNegotiation able */
#define ANER_PAGERX         0x0002      /* New Page received                 */
#define ANER_NPABLE         0x0004      /* Local Device next page able       */
#define ANER_LPNPABLE       0x0008      /* Link partner supports npage       */
#define ANER_PDF            0x0010      /* Parallel Detection fault          */
#define ANER_RESV           0xFFE0      /* Unused...                         */

/* PHY ID */
#define MII_DM9161_ID       0x0181b8a0
#define MII_AM79C875_ID     0x00225540  /* 0x00225541                        */

/* Definitions */
#define ETH_ADRLEN      6         /* Ethernet Address Length in bytes        */
#define IP_ADRLEN       4         /* IP Address Length in bytes              */
#define OS_HEADER_LEN   4         /* TCPnet 'os_frame' header size           */
                                  /* Frame Header length common for all      */
#define PHY_HEADER_LEN  (2*ETH_ADRLEN + 2) /* network interfaces.            */
#define ETH_MTU         1514      /* Ethernet Frame Max Transfer Unit        */
#define PPP_PROT_IP     0x0021    /* PPP Protocol type: IP                   */
#define TCP_DEF_WINSIZE 4380      /* TCP default window size                 */
#define PASSW_SZ        20        /* Authentication Password Buffer size     */

/* Network Interfaces */
#define NETIF_ETH       0         /* Network interface: Ethernet             */
#define NETIF_PPP       1         /* Network interface: PPP                  */
#define NETIF_SLIP      2         /* Network interface: Slip                 */
#define NETIF_LOCAL     3         /* Network interface: Localhost (loopback) */
#define NETIF_NULL      4         /* Network interface: Null (none)          */

/* Telnet Definitions */
#define TNET_LBUFSZ     96        /* Command Line buffer size (bytes)        */
#define TNET_HISTSZ     128       /* Command History buffer size (bytes)     */
#define TNET_FIFOSZ     128       /* Input character Fifo buffer (bytes)     */

/* SNMP-MIB Definitions */
#define MIB_INTEGER     0x02      /* MIB entry type INTEGER                  */
#define MIB_OCTET_STR   0x04      /* MIB entry type OCTET_STRING             */
#define MIB_OBJECT_ID   0x06      /* MIB entry type OBJECT_IDENTIFIER        */
#define MIB_IP_ADDR     0x40      /* MIB entry type IP ADDRESS (U8[4])       */
#define MIB_COUNTER     0x41      /* MIB entry type COUNTER (U32)            */
#define MIB_GAUGE       0x42      /* MIB entry type GAUGE (U32)              */
#define MIB_TIME_TICKS  0x43      /* MIB entry type TIME_TICKS               */
#define MIB_ATR_RO      0x80      /* MIB entry attribute READ_ONLY           */
#define MIB_OIDSZ       17        /* Max.size of Object ID value             */
#define MIB_STRSZ       110       /* Max.size of Octet String variable       */
#define MIB_READ        0         /* MIB entry Read access                   */
#define MIB_WRITE       1         /* MIB entry Write access                  */

/* SNMP-MIB Macros */
#define MIB_STR(s)      sizeof(s)-1, s
#define MIB_INT(o)      sizeof(o), (void *)&o
#define MIB_IP(ip)      4, (void *)&ip 
#define OID0(f,s)       (f*40 + s) 

/* Debug Module Definitions */
#define MODULE_MEM      0         /* Dynamic Memory Module ID                */
#define MODULE_ETH      1         /* Ethernet Module ID                      */
#define MODULE_PPP      2         /* PPP Module ID                           */
#define MODULE_SLIP     3         /* SLIP Module ID                          */
#define MODULE_ARP      4         /* ARP Module ID                           */
#define MODULE_IP       5         /* IP Module ID                            */
#define MODULE_ICMP     6         /* ICMP Module ID                          */
#define MODULE_IGMP     7         /* IGMP Module ID                          */
#define MODULE_UDP      8         /* UDP Module ID                           */
#define MODULE_TCP      9         /* TCP Module ID                           */
#define MODULE_NBNS     10        /* NBNS Module ID                          */
#define MODULE_DHCP     11        /* DHCP Module ID                          */
#define MODULE_DNS      12        /* DNS Module ID                           */
#define MODULE_SNMP     13        /* SNMP Module ID                          */
#define MODULE_BSD      14        /* BSD Socket Module ID                    */
#define MODULE_HTTP     15        /* HTTP Server Module ID                   */
#define MODULE_FTP      16        /* FTP Server Module ID                    */
#define MODULE_FTPC     17        /* FTP Client Module ID                    */
#define MODULE_TNET     18        /* Telnet Server Module ID                 */
#define MODULE_TFTP     19        /* TFTP Server Module ID                   */
#define MODULE_TFTPC    20        /* TFTP Client Module ID                   */
#define MODULE_SMTP     21        /* SMTP Client Module ID                   */



//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++







#endif // EMAC_DEF_H__13_06_2013__09_47
