// EthernetLayer.cpp: implementation of the CEthernetLayer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DC_ARP_01.h"
#include "EthernetLayer.h"
#include "ARPLayer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEthernetLayer::CEthernetLayer( char* pName )
: CBaseLayer( pName )
{
	ResetHeader( ) ;
}

CEthernetLayer::~CEthernetLayer()
{
}

void CEthernetLayer::ResetHeader()
{
	memset( m_sHeader.enet_dstaddr.addrs, 0, 6 ) ;
	memset( m_sHeader.enet_srcaddr.addrs, 0, 6 ) ;
	memset( m_sHeader.enet_data, 0, ETHER_MAX_DATA_SIZE ) ;
	m_sHeader.enet_type = 0x0608 ; // 0x0800
}

unsigned char* CEthernetLayer::GetEnetDstAddress() 
{
	return m_sHeader.enet_srcaddr.addrs;
}

unsigned char* CEthernetLayer::GetEnetSrcAddress()
{
	return m_sHeader.enet_dstaddr.addrs;
}

void CEthernetLayer::SetEnetSrcAddress(unsigned char *pAddress)
{
	memcpy( &m_sHeader.enet_srcaddr.addrs, pAddress, 6 ) ;

}

void CEthernetLayer::SetEnetDstAddress(unsigned char *pAddress)
{

	memcpy( &m_sHeader.enet_dstaddr.addrs, pAddress, 6 ) ;
}

BOOL CEthernetLayer::Send(unsigned char *ppayload, int nlength)
{
	memcpy( m_sHeader.enet_data, ppayload, nlength ) ;

	BOOL bSuccess = FALSE ;
	bSuccess = mp_UnderLayer->Send((unsigned char*) &m_sHeader,nlength+ETHER_HEADER_SIZE);

	return bSuccess ;
}
	
BOOL CEthernetLayer::Receive( unsigned char* ppayload )
{
	PETHERNET_HEADER pFrame = (PETHERNET_HEADER) ppayload ;

	BOOL bSuccess = FALSE ;
							// 받은 패킷의 시작지와 자신의 맥주소와 같으면 받지않는다.

	list<CIPLayer::INTERFACE_STRUCT>::iterator iter = device_list.begin();
	for(; iter != device_list.end(); iter++)
	{

	}

	if( memcmp((char *)pFrame->enet_srcaddr.S_un.s_ether_addr,(char *)m_sHeader.enet_srcaddr.S_un.s_ether_addr,6) != 0)
	{						// 받은 패킷의 목적지와 자신의 맥주소와 같지 않고, 받은 패킷의 목적지 주소가 브로드캐스트이면 받음.
		if ( memcmp((char *)pFrame->enet_dstaddr.S_un.s_ether_addr,(char *)m_sHeader.enet_srcaddr.S_un.s_ether_addr,6) == 0 ||
			 memcmp((char *)pFrame->enet_dstaddr.S_un.s_ether_addr,BROADCAST_ADDR, 6) == 0)
		{					//포트번호를 봄. 실질적으로 상관 x 
			if(ntohs(pFrame->enet_type) == ntohs(ETHER_PROTO_TYPE_IP))
			{
				bSuccess = mp_aUpperLayer[1]->Receive((unsigned char*) pFrame->enet_data);
			}
			else if(ntohs(pFrame->enet_type) == ntohs(ETHER_PROTO_TYPE_ARP))
			{
				bSuccess = mp_aUpperLayer[0]->Receive((unsigned char*) pFrame->enet_data);
			}
		}
	}
	return bSuccess ;
}
