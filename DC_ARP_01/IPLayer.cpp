// IPLayer.cpp: implementation of the CIPLayer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DC_ARP_01.h"
#include "IPLayer.h"
#include "ARPLayer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIPLayer::CIPLayer( char* pName )
: CBaseLayer( pName )
{
	ResetHeader( );
}

CIPLayer::~CIPLayer()
{
}

void CIPLayer::ResetHeader()
{
	m_sHeader.ip_verlen = 0x00;
	m_sHeader.ip_tos = 0x00;
	m_sHeader.ip_len = 0x0000;
	m_sHeader.ip_id = 0x0000;
	m_sHeader.ip_fragoff = 0x0000;
	m_sHeader.ip_ttl = 0x00;
	m_sHeader.ip_proto = 0x00;
	m_sHeader.ip_cksum = 0x00;
	memset( m_sHeader.ip_src, 0, 4);
	memset( m_sHeader.ip_dst, 0, 4);
	memset( m_sHeader.ip_data, 0, IP_DATA_SIZE);
	memset(default_netmask, 0, 4);
}

void CIPLayer::SetSrcIPAddress(unsigned char* src_ip)
{
	memcpy( m_sHeader.ip_src, src_ip, 4);
}

void CIPLayer::SetDstIPAddress(unsigned char* dst_ip)
{
	memcpy( m_sHeader.ip_dst, dst_ip, 4);
}

void CIPLayer::SetSrcNetmask(unsigned char* src_netmask)
{
	memcpy( my_netmask, src_netmask, 4);
}

void CIPLayer::SetFragOff(unsigned short fragoff)
{
	m_sHeader.ip_fragoff = fragoff;
}

void CIPLayer::IPAddressMasking(unsigned char* dst_ip, unsigned char* src_ip, unsigned char* mask_ip)
{
	dst_ip[0] = src_ip[0] & mask_ip[0];
	dst_ip[1] = src_ip[1] & mask_ip[1];	
	dst_ip[2] = src_ip[2] & mask_ip[2];
	dst_ip[3] = src_ip[3] & mask_ip[3];
}

BOOL CIPLayer::Send(unsigned char* ppayload, int nlength)
{
	memcpy( m_sHeader.ip_data, ppayload, nlength ) ;
	
	BOOL bSuccess = FALSE ;
	m_sHeader.ip_tos = 0x0;

	unsigned char my_address[4];
	unsigned char opposite_address[4];
	
	IPAddressMasking(my_address, m_sHeader.ip_src, my_netmask);
	IPAddressMasking(opposite_address, m_sHeader.ip_dst, my_netmask);

	if (memcmp(my_address, opposite_address, 4) != 0)
	{
		BOOL isDestinationExist = FALSE;
		list<STATIC_IP_ROUTING_RECORD>::iterator iter = routingTable.begin();
		for(; iter != routingTable.end(); iter++)
		{	
			if(memcmp((*iter).destination_ip, my_address, 4) == 0)
			{
				((CARPLayer*)GetUnderLayer())->next_ethernet_type = ETHER_PROTO_TYPE_ARP;
				((CARPLayer*)GetUnderLayer())->setTargetIPAddress((*iter).gateway_ip);
				bSuccess = mp_UnderLayer->Send(ppayload,sizeof(ppayload));

				if(bSuccess)
				{
					((CARPLayer*)GetUnderLayer())->next_ethernet_type = ETHER_PROTO_TYPE_IP;
					((CARPLayer*)GetUnderLayer())->setTargetIPAddress(m_sHeader.ip_dst);
					bSuccess = mp_UnderLayer->Send(ppayload,sizeof(ppayload));
				}
			}
			break;
		}

		if(isDestinationExist == FALSE)
		{
			list<STATIC_IP_ROUTING_RECORD>::iterator iter = routingTable.begin();
			for(; iter != routingTable.end(); iter++)
			{	
				if(memcmp((*iter).netmask_ip, default_netmask, 4) == 0)
				{
					((CARPLayer*)GetUnderLayer())->next_ethernet_type = ETHER_PROTO_TYPE_ARP;
					((CARPLayer*)GetUnderLayer())->setTargetIPAddress((*iter).gateway_ip);
					bSuccess = mp_UnderLayer->Send(ppayload,sizeof(ppayload));

					if(bSuccess)
					{
						((CARPLayer*)GetUnderLayer())->next_ethernet_type = ETHER_PROTO_TYPE_IP;
						((CARPLayer*)GetUnderLayer())->setTargetIPAddress(m_sHeader.ip_dst);
						bSuccess = mp_UnderLayer->Send(ppayload,sizeof(ppayload));
					}
				}
				break;
			}
		}
	}
	else
	{
		((CARPLayer*)GetUnderLayer())->next_ethernet_type = ETHER_PROTO_TYPE_ARP;
		((CARPLayer*)GetUnderLayer())->setTargetIPAddress(m_sHeader.ip_dst);
		bSuccess = mp_UnderLayer->Send(ppayload,sizeof(ppayload));

		if(bSuccess)
		{
			((CARPLayer*)GetUnderLayer())->next_ethernet_type = ETHER_PROTO_TYPE_IP;
			((CARPLayer*)GetUnderLayer())->setTargetIPAddress(m_sHeader.ip_dst);
			bSuccess = mp_UnderLayer->Send(ppayload,sizeof(ppayload));
		}
	}
	return bSuccess;
}

BOOL CIPLayer::Receive(unsigned char* ppayload)
{
	PIPLayer_HEADER pFrame = (PIPLayer_HEADER) ppayload ;
	
	BOOL bSuccess = FALSE ;
	// I am a host and this packet is mine
	if(memcmp(pFrame->ip_dst, m_sHeader.ip_src, 4) == 0)
	{
		if(pFrame->ip_tos == 0x0)	//ping received
		{
			bSuccess = mp_aUpperLayer[0]->Receive((unsigned char*)pFrame->ip_data);
			pFrame->ip_tos = 0x1;
			unsigned char temp[4];
			memcpy(temp, pFrame->ip_dst, 4);
			memcpy(pFrame->ip_dst, pFrame->ip_src, 4);
			memcpy(pFrame->ip_src, temp, 4);
			bSuccess = mp_UnderLayer->Send((unsigned char*)pFrame, sizeof(pFrame));
		}
		else if(pFrame->ip_tos == 0x1)	//ping success
		{
			bSuccess = mp_aUpperLayer[0]->Receive((unsigned char*)pFrame->ip_data);
		}
		return bSuccess ;
	}
	else
	{
		list<STATIC_IP_ROUTING_RECORD>::iterator iter = routingTable.begin();
		for(; iter != routingTable.end(); iter++)
		{

			unsigned char maskedData[4];
			IPAddressMasking(maskedData, pFrame->ip_dst, (*iter).netmask_ip);
			
			//This record has same networkID with received packet's ip destination data.
			if(memcmp((*iter).destination_ip, maskedData, 4) == 0)
			{
				list<INTERFACE_STRUCT>::iterator device_iter = device_list.begin();
				for(;device_iter != device_list.end(); device_iter++)
				{
					if((*device_iter).device_number == (*iter).device_number)
					{
						setProtocolStack((*device_iter).device_ip, (*device_iter).device_mac);
						break;
					}
				}

				unsigned char isFlagUp = IS_FLAG_UP((*iter).flag);
				unsigned char isFlagGateway = IS_FLAG_GATEWAY((*iter).flag);

				if( isFlagUp && isFlagGateway )
				{
					((CARPLayer*)GetUnderLayer())->next_ethernet_type = ETHER_PROTO_TYPE_ARP;
					((CARPLayer*)GetUnderLayer())->setTargetIPAddress((*iter).gateway_ip);
					bSuccess = mp_UnderLayer->Send(ppayload,sizeof(ppayload));

					if(bSuccess)
					{
						((CARPLayer*)GetUnderLayer())->next_ethernet_type = ETHER_PROTO_TYPE_IP;
						((CARPLayer*)GetUnderLayer())->setTargetIPAddress(pFrame->ip_dst);
						bSuccess = mp_UnderLayer->Send(ppayload,sizeof(ppayload));
					}
				
				}
				else if( isFlagUp )
				{
					((CARPLayer*)GetUnderLayer())->next_ethernet_type = ETHER_PROTO_TYPE_ARP;
					((CARPLayer*)GetUnderLayer())->setTargetIPAddress(pFrame->ip_dst);
					bSuccess = mp_UnderLayer->Send(ppayload,sizeof(ppayload));

					if(bSuccess)
					{
						((CARPLayer*)GetUnderLayer())->next_ethernet_type = ETHER_PROTO_TYPE_IP;
						((CARPLayer*)GetUnderLayer())->setTargetIPAddress(pFrame->ip_dst);
						bSuccess = mp_UnderLayer->Send(ppayload,sizeof(ppayload));
					}
				}
				break;
			}
		}
		return bSuccess;
	}
}

void CIPLayer::setProtocolStack(unsigned char* ipAddress, unsigned char* macAddress)
{
	((CARPLayer*)GetUnderLayer())->setSenderIPAddress(ipAddress);
	((CARPLayer*)GetUnderLayer())->setSenderHardwareAddress(macAddress);
	((CARPLayer*)GetUnderLayer())->setEthernetHardwareAddress(macAddress);
}