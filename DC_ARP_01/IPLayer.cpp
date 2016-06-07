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
	m_sHeader.ip_verlen = 0x4F;
	m_sHeader.ip_tos = 0x00;
	m_sHeader.ip_len = 0x40;
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
	memcpy(srcip, src_ip, 4);
}

void CIPLayer::SetDstIPAddress(unsigned char* dst_ip)
{
	memcpy( m_sHeader.ip_dst, dst_ip, 4);
	memcpy(destip, dst_ip, 4);
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
				int met=0; (*iter).metric.Format("%d", met);
				m_sHeader.ip_ttl = met;
				sendPacketViaGivenAddress(FALSE, (unsigned char*)ppayload, NULL, (*iter).gateway_ip, m_sHeader.ip_dst, nlength);
				break;
			}
		}

		if(isDestinationExist == FALSE)
		{
			list<STATIC_IP_ROUTING_RECORD>::iterator iter = routingTable.begin();
			for(; iter != routingTable.end(); iter++)
			{	
				if(memcmp((*iter).netmask_ip, default_netmask, 4) == 0)
				{
					int met=0; (*iter).metric.Format("%d", met);
					m_sHeader.ip_ttl = met;
					bSuccess = sendPacketViaGivenAddress(FALSE, (unsigned char*)ppayload, NULL, (*iter).gateway_ip, m_sHeader.ip_dst, nlength); 
					break;
				}
			}
		}
	}
	else
	{
		m_sHeader.ip_ttl = 2;
		sendPacketViaGivenAddress(FALSE, (unsigned char*)ppayload, NULL, m_sHeader.ip_dst, m_sHeader.ip_dst, nlength);
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
		if(pFrame->ip_tos == 0x1)	//ping received
		{
			bSuccess = mp_aUpperLayer[0]->Receive((unsigned char*)pFrame->ip_data);

			unsigned char temp[4];
			memcpy(temp, pFrame->ip_dst, 4);
			memcpy(pFrame->ip_dst, pFrame->ip_src, 4);
			memcpy(pFrame->ip_src, temp, 4);
			pFrame->ip_ttl = 28;
			pFrame->ip_tos = 0x2;
			list<STATIC_IP_ROUTING_RECORD>::iterator iter = routingTable.begin();
			unsigned char maskedData[4];
			for(; iter != routingTable.end(); iter++)
			{
				IPAddressMasking(maskedData, pFrame->ip_dst, (*iter).netmask_ip);
				if(memcmp((*iter).destination_ip, maskedData, 4) == 0)
				{
					list<INTERFACE_STRUCT>::iterator device_iter = device_list.begin();
					for(;device_iter != device_list.end(); device_iter++)
					{
						if((*device_iter).device_number == (*iter).device_number)
						{
							setProtocolStack((*device_iter).device_ip, (*device_iter).device_mac, (*device_iter).device_number);
							break;
						}
					}
				}
			}

			bSuccess = mp_UnderLayer->Send((unsigned char*)pFrame, 100);
		}
		else if(pFrame->ip_tos == 0x2)	//ping success
		{
			bSuccess = mp_aUpperLayer[0]->Receive((unsigned char*)pFrame->ip_data);
		}
		return bSuccess ;
	}
	else if(pFrame->ip_dst[3] == 0xff)
	{
		return FALSE;
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
						setProtocolStack((*device_iter).device_ip, (*device_iter).device_mac, (*device_iter).device_number);
						break;
					}
				}

				unsigned char isFlagUp = IS_FLAG_UP((*iter).flag);
				unsigned char isFlagGateway = IS_FLAG_GATEWAY((*iter).flag);
				
				int met=0; (*iter).metric.Format("%d", met);
				m_sHeader.ip_ttl = met;
				if( isFlagUp )
				{
					if( isFlagGateway )
						sendPacketViaGivenAddress(TRUE, (unsigned char*) pFrame, (*device_iter).device_ip, (*iter).gateway_ip, pFrame->ip_dst, 120);
					else
						sendPacketViaGivenAddress(TRUE, (unsigned char*) pFrame, (*device_iter).device_ip, pFrame->ip_dst, pFrame->ip_dst, 120);
					break;
				}
			}
		}
		return bSuccess;
	}
}

void CIPLayer::setProtocolStack(unsigned char* ipAddress, unsigned char* macAddress, int adapter_number)
{
	((CARPLayer*)GetUnderLayer())->setSenderIPAddress(ipAddress);
	((CARPLayer*)GetUnderLayer())->setSenderHardwareAddress(macAddress);
	((CARPLayer*)GetUnderLayer())->setEthernetHardwareAddress(macAddress);
	((CARPLayer*)GetUnderLayer())->setNICard(adapter_number);
}

BOOL CIPLayer::sendPacketViaGivenAddress(BOOL isHeaderedData, unsigned char* ppayload, unsigned char* route_address,
	unsigned char* arp_target, unsigned char* ip_target, unsigned int length)
{
	BOOL bSuccess = FALSE;
	
	unsigned char* packet;
	unsigned char* packet_reserv = (unsigned char*)malloc(length);
	if(isHeaderedData == FALSE)
	{
		m_sHeader.ip_verlen = 0x49;
		m_sHeader.ip_tos = 0x01;
		m_sHeader.ip_len = 0x20;
		m_sHeader.ip_id = 0x0000;
		m_sHeader.ip_fragoff = 0x0000;
		m_sHeader.ip_proto = 0x1;
		m_sHeader.ip_cksum = 0x0000;
		m_sHeader.ip_ttl = 24;
		memcpy( m_sHeader.ip_data, ppayload, length ) ;

		memcpy(m_sHeader.ip_dst, destip, 4);
		memcpy(m_sHeader.ip_src, srcip, 4);

		packet = (unsigned char*) &m_sHeader;
	}
	else
	{
		packet = ppayload;
		((CARPLayer*)GetUnderLayer())->setSenderIPAddress(route_address);
	}

	memcpy(packet_reserv, packet, length);
	
	PIPLayer_HEADER pFrame = (PIPLayer_HEADER) packet;
	((CARPLayer*)GetUnderLayer())->next_ethernet_type = ETHER_PROTO_TYPE_ARP;
	((CARPLayer*)GetUnderLayer())->setTargetIPAddress(arp_target);
	//bSuccess = mp_UnderLayer->Send((unsigned char*)ppayload,sizeof(*ppayload));

	bSuccess = mp_UnderLayer->Send((unsigned char*)packet, length);

	if(bSuccess)	//ARP Request Success
	{
		((CARPLayer*)GetUnderLayer())->next_ethernet_type = ETHER_PROTO_TYPE_IP;
		((CARPLayer*)GetUnderLayer())->setTargetIPAddress(ip_target);
		unsigned char targetMAC[6];
		memcpy(targetMAC, ((CARPLayer*)GetUnderLayer())->getHardwareAddressByGivenIPAddress(arp_target), 6);
		PIPLayer_HEADER pFrame = (PIPLayer_HEADER) packet;
		((CARPLayer*)GetUnderLayer())->setTargetHardwareAddress(targetMAC);
		bSuccess = mp_UnderLayer->Send((unsigned char*)packet_reserv, length);
	}
	return bSuccess;
}
