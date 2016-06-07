#include "StdAfx.h"
#include "ARPLayer.h"
#include "NILayer.h"

CARPLayer::CARPLayer(char* pName)
: CBaseLayer( pName ), ARP_REQUEST(0x100), ARP_REPLY(0x200) // bigendian ���� ���⶧���� �̷��� ����.
{
	ResetHeader();
}


CARPLayer::~CARPLayer(void)
{
}

void CARPLayer::ResetHeader()
{
	arpHeader.arpHardwareType = 0x0100;
	arpHeader.arpProtocolType = 0x0008;
	arpHeader.arpHardwareAddrSize = 0x6;
	arpHeader.arpProtocolAddrSize = 0x4;
	arpHeader.arpOperationType = 0;
	memset(arpHeader.arpSenderHardwareAddress, 0, 6);
	memset(arpHeader.arpSenderIPAddress, 0, 4);
	memset(arpHeader.arpTargetHardwareAddress, 0, 6);
	memset(arpHeader.arpTargetIPAddress, 0, 4);
	memset(ownMACAddress, 0, 6);
	memset(ownIPAddress, 0, 4);
	memset(targetMACAddress, 0, 6);
}


void CARPLayer::setARPOperationType(unsigned char operationType)
{
	arpHeader.arpOperationType = operationType;
}


void CARPLayer::setSenderIPAddress(unsigned char* senderIP)
{
	arpHeader.arpSenderIPAddress[0] = ownIPAddress[0] = senderIP[0];
	arpHeader.arpSenderIPAddress[1] = ownIPAddress[1] = senderIP[1];
	arpHeader.arpSenderIPAddress[2] = ownIPAddress[2] = senderIP[2];
	arpHeader.arpSenderIPAddress[3] = ownIPAddress[3] = senderIP[3];
}


void CARPLayer::setSenderHardwareAddress(unsigned char* senderHard)
{
	memcpy(arpHeader.arpSenderHardwareAddress, senderHard, 6);
	memcpy(ownMACAddress, senderHard, 6);
}


void CARPLayer::setTargetIPAddress(unsigned char* targetIP)
{
	arpHeader.arpTargetIPAddress[0] = targetIPAddress[0] = targetIP[0];
	arpHeader.arpTargetIPAddress[1] = targetIPAddress[1] = targetIP[1];
	arpHeader.arpTargetIPAddress[2] = targetIPAddress[2] = targetIP[2];
	arpHeader.arpTargetIPAddress[3] = targetIPAddress[3] = targetIP[3];
}


void CARPLayer::setTargetHardwareAddress(unsigned char* targetHard)
{
	memcpy(arpHeader.arpTargetHardwareAddress, targetHard, 6);
	memcpy(targetMACAddress, targetHard, 6);
}

void CARPLayer::setEthernetHardwareAddress(unsigned char* macAddress)
{
	((CEthernetLayer*)GetUnderLayer())->SetEnetSrcAddress(macAddress);
}

void CARPLayer::setNICard(int adapter_number)
{
	((CNILayer*)((CEthernetLayer*)GetUnderLayer())->GetUnderLayer())->SetOpenedAdapterObject(((CNILayer*)((CEthernetLayer*)GetUnderLayer())->GetUnderLayer())->adapterOpenedIndexList[adapter_number]);
}

unsigned char* CARPLayer::getHardwareAddressByGivenIPAddress(unsigned char* ipAddress)
{
	unsigned char return_value[6];
	list<ARP_CACHE_RECORD>::iterator cacheIter = arpCacheTable.begin();
	for(cacheIter; cacheIter != arpCacheTable.end(); cacheIter++)// gratuitous �ƴ϶��, cache�� �ִ� ��ŭ for��������.
	{
		if(memcmp((*cacheIter).ipAddress, ipAddress, 4) == 0) //�������� ip�� ���� ip�� �ִٸ� 
		{
			memcpy(return_value,(*cacheIter).ethernetAddress,6);
			break;
		}
	}
	return return_value;
}

list<CARPLayer::ARP_CACHE_RECORD> CARPLayer::getARPCacheTable(void)
{
	return arpCacheTable;
}

void CARPLayer::setAdapter(CString adapter)
{
	this->adapter = adapter;
}

BOOL CARPLayer::Send(unsigned char* ppayload, int length)
{
		memcpy( arpHeader.arpData, ppayload, length );
		
		BOOL bSuccess = FALSE ;
		BOOL isCacheSameIP = FALSE;	//ĳ�� �̿밡������.
		BOOL isCacheSameMAC = FALSE;
		BOOL isGratuitousPacket = FALSE;	//gratuitous �߻� �ߴ���.
		list<ARP_CACHE_RECORD>::iterator cacheIter = arpCacheTable.begin();
		if(memcmp(targetIPAddress, ownIPAddress, 4) == 0)	// gratuitous���� Ȯ��.
			isGratuitousPacket = TRUE;
		else
		{
			for(cacheIter; cacheIter != arpCacheTable.end(); cacheIter++)// gratuitous �ƴ϶��, cache�� �ִ� ��ŭ for��������.
			{
				if(memcmp((*cacheIter).ipAddress, targetIPAddress, 4) == 0) //�������� ip�� ���� ip�� �ִٸ� 
				{
					isCacheSameIP = TRUE;
					break;
				}
				if(memcmp((*cacheIter).ethernetAddress, targetMACAddress, 6) == 0)
				{
					isCacheSameMAC = TRUE;
					break;
				}
			}
		}

		//if cache is vaild and complete record
		if(((isCacheSameIP == TRUE) || (isCacheSameMAC == TRUE)) && ((*cacheIter).isComplete == TRUE))	// ĳ�ÿ� ��밡���� ���� �����Ѵٸ�, 
		{	
			setTargetHardwareAddress((*cacheIter).ethernetAddress);	//ĳ�ÿ� �ִٸ� mac �ּҸ� �˰� �� ���̹Ƿ�, �� mac �ּҷ� �缳��.
			((CEthernetLayer*)GetUnderLayer())->m_sHeader.enet_type = next_ethernet_type;
			((CEthernetLayer*)GetUnderLayer())->SetEnetDstAddress((*cacheIter).ethernetAddress);// ethernet layer�� mac �ּҵ� �ٽ� ����.

		}
		else if(isCacheSameIP == TRUE)
		{
		}
		//it is not valid record
		else // ĳ�õ� ����, gratuitous�� �ƴϰ�.
		{
			memset(arpHeader.arpTargetHardwareAddress, 0, 6);
			((CEthernetLayer*)GetUnderLayer())->SetEnetDstAddress(BROADCAST_ADDR);
			((CEthernetLayer*)GetUnderLayer())->m_sHeader.enet_type = next_ethernet_type;


			ARP_CACHE_RECORD newRecord;
			newRecord.arpInterface = this->adapter;
			memset(newRecord.ethernetAddress, 0, 6);
			memcpy(newRecord.ipAddress, targetIPAddress, 4);
			newRecord.isComplete = FALSE;
			newRecord.lifeTimeCounter = INCOMPLETE_DELETE_TIME;

			arpCacheTable.push_back(newRecord);
		}
		
		if(next_ethernet_type == ETHER_PROTO_TYPE_IP)
			bSuccess = mp_UnderLayer->Send((unsigned char*)ppayload, length);
		else
		{
			// ----����---
			// 1. Gratuitous�� �´ٸ�, �׳� �ƹ� �۾� ���� �ʰ�, ��Ŷ ���� ����.
			// 2. ĳ�� �̿밡���ϴٸ�, ĳ�ÿ� �ִ� ������ ���ּ� �����ؼ� ����.
			// 3. �̿밡���� ĳ�� ����, gratitous�� �ƴ϶�� broadcast�� ������ �����ؼ� ����.

			arpHeader.arpHardwareType = 0x0100;
			arpHeader.arpProtocolType = 0x0008;
			arpHeader.arpHardwareAddrSize = 0x6;
			arpHeader.arpProtocolAddrSize = 0x4;
			arpHeader.arpOperationType = ARP_REQUEST;
			memcpy(arpHeader.arpSenderHardwareAddress, ownMACAddress, 6);
			memcpy(arpHeader.arpSenderIPAddress, ownIPAddress, 4);
			memcpy(arpHeader.arpTargetIPAddress, targetIPAddress, 4);
	

			bSuccess = mp_UnderLayer->Send((unsigned char*)&arpHeader,length+ARP_HEADER_SIZE);
		}
		return bSuccess;
}

BOOL CARPLayer::Receive(unsigned char* ppayload)
{

	PARP_HEADER pARPFrame = (PARP_HEADER)ppayload;
	
	BOOL bSuccess = FALSE ;
	BOOL GratitousOccur = FALSE ;
	BOOL isNotMyIPAddress = FALSE;

	unsigned char receivedARPTargetIPAddress[4];
	unsigned char receivedARPSenderIPAddress[4];
	unsigned char receivedARPSenderHardwareAddress[6];
	memcpy(receivedARPTargetIPAddress, (unsigned char*)pARPFrame->arpTargetIPAddress, 4); //�޾ƿ� ��Ŷ�� ���� ������ip, ������ip , ������mac �� ����
	memcpy(receivedARPSenderIPAddress, (unsigned char*)pARPFrame->arpSenderIPAddress, 4);
	memcpy(receivedARPSenderHardwareAddress, (unsigned char*)pARPFrame->arpSenderHardwareAddress, 6);
	
	BOOL isARPRecordExist = FALSE;
	list<ARP_CACHE_RECORD>::iterator arpIter = arpCacheTable.begin();
	for(arpIter; arpIter != arpCacheTable.end(); arpIter++) // arp table�� ����.
	{
		if(memcmp((*arpIter).ipAddress,receivedARPSenderIPAddress, 4) == 0) //���� �ش� ���̺��� ip �ּҿ� ���� ��Ŷ�� ������ ip �ּ� ���ٸ�
		{
			isARPRecordExist = TRUE; //�̹� �����Ѵٰ� ǥ��.
 			memcpy((*arpIter).ethernetAddress, receivedARPSenderHardwareAddress, 6);// ���ּҸ� ���� �޾ƿ� �ɷ� ����. �ڼ����� gratitous, ???? , �ߺ����¸� ������������� �׳�.. 
																					// ������ �����ϵ��� ��. 
			(*arpIter).isComplete = TRUE; // complete �Ǿ��ٰ� ǥ��.
			(*arpIter).lifeTimeCounter = COMPLETE_DELETE_TIME;
			break;
		}
	}
	if (memcmp(ownIPAddress, receivedARPSenderIPAddress, 4) == 0) // gratitous���� Ȯ��.
		GratitousOccur = TRUE;
	if (memcmp(ownIPAddress, receivedARPTargetIPAddress, 4) != 0) // gratitous�ƴ϶�� �� Ȯ��.
		isNotMyIPAddress = TRUE;
		
	BOOL isProxyAvailable = FALSE; //proxy table�� ���� �� �ִ���
	list<ARP_CACHE_RECORD>::iterator proxyIter = arpProxyTable.begin();
	for(proxyIter; proxyIter != arpProxyTable.end(); proxyIter++)	//proxy table ���Ǳ����� for��������.
	{
		if(memcmp((*proxyIter).ipAddress, receivedARPTargetIPAddress, 4) == 0)	//���� ���� ip�� �ִٸ�. ��, ���� ��Ŷ�� ��� reply �� �� �ɷ��� �ִ���.
		{
			isProxyAvailable = TRUE; //proxy table ��� �����̶� ǥ��.
			break;
		}
	}

	if(GratitousOccur == FALSE)	//Gratitous�� �߻����� �ʾҴٸ�. Gratitous �߻��� arptable ���Ÿ��� �ʿ�� �ϹǷ� ������ �� ó���Ǿ���.
	{
		if(ntohs(pARPFrame->arpOperationType) == ntohs(ARP_REPLY)) // ���� ��Ŷ�� reply���
		{
			((CIPLayer*)GetUpperLayer(0))->semaphore = 1;
			bSuccess = mp_aUpperLayer[0]->Receive((unsigned char*)pARPFrame->arpData); //������������ �÷���.
			return bSuccess;
		}
		if(ntohs(pARPFrame->arpOperationType) == ntohs(ARP_REQUEST))		//���� ��Ŷ�� request���
		{
			if(isARPRecordExist == FALSE)// arptable�� �ߺ��Ǵ� ���� ���ٸ� table�� �߰�.
			{
				ARP_CACHE_RECORD newRecord;
				newRecord.arpInterface = adapter;
				memcpy(newRecord.ethernetAddress, receivedARPSenderHardwareAddress, 6);
				memcpy(newRecord.ipAddress, receivedARPSenderIPAddress, 4);
				newRecord.isComplete = TRUE;
				newRecord.lifeTimeCounter = INCOMPLETE_DELETE_TIME;

				arpCacheTable.push_back(newRecord);
			}
		}

		// �ָ�! ���� ���⸦ ������ �� ���� ��Ŷ�� ARP_REQUEST ���¸��� ���� ���̴�. reply�� �����ִ� �۾�(send)�� ���� �� �� �ִ�.

		unsigned char tempHardwareAddress[6];
		unsigned char tempIPAddress[4];
		memset(tempHardwareAddress, 0, 6);
		memset(tempIPAddress, 0, 4);

		memcpy(tempHardwareAddress, receivedARPSenderHardwareAddress, 6);
		memcpy(tempIPAddress, receivedARPSenderIPAddress, 4);

		if ( (receivedARPTargetIPAddress[0] == ownIPAddress[0]) &&
			 (receivedARPTargetIPAddress[1] == ownIPAddress[1]) &&
			 (receivedARPTargetIPAddress[2] == ownIPAddress[2]) &&
			 (receivedARPTargetIPAddress[3] == ownIPAddress[3]))
		{ // ���� ��Ŷ�� �������ּҿ� �ڽ��� �ּҰ� ���ٸ�, ������ reply�� ���� mac�ּҸ� �����Ѵ�. ip�� ���� �������ش�.
			memcpy(arpHeader.arpSenderHardwareAddress, ownMACAddress, 6);
			memcpy(arpHeader.arpSenderIPAddress, ownIPAddress, 4);
		}

		if(isProxyAvailable == TRUE) //proxytable �̿� �����ϴٸ�,
		{
			memcpy(arpHeader.arpSenderHardwareAddress, (*proxyIter).ethernetAddress, 6); // �� proxytable�� �ִ� ������ ���� ��Ŷ�� �������ش�.
			memcpy(arpHeader.arpSenderIPAddress, (*proxyIter).ipAddress, 4);
		}
		
		if(isNotMyIPAddress == TRUE && isProxyAvailable == FALSE)// proxy�� �̿����� ���ϰ�, ������ ip�� �ڽ��� ip�� �ٸ���, ������ �۾�.
			return FALSE;
		

		//���� ���ݱ��� �ߴ� �۾��� ���� ������ ��Ŷ�� ������ִ� �۾��� �Ѵ�.
		memcpy(arpHeader.arpTargetHardwareAddress, tempHardwareAddress, 6); 
		memcpy(arpHeader.arpTargetIPAddress, tempIPAddress, 4);
			
		arpHeader.arpHardwareType = 0x0100;
		arpHeader.arpProtocolType = 0x0008;
		arpHeader.arpHardwareAddrSize = 0x6;
		arpHeader.arpProtocolAddrSize = 0x4;
		arpHeader.arpOperationType = ARP_REPLY;
		memset(arpHeader.arpData, 0, 1);

		((CEthernetLayer*)GetUnderLayer())->SetEnetDstAddress(arpHeader.arpTargetHardwareAddress);
		((CEthernetLayer*)GetUnderLayer())->SetEnetSrcAddress(arpHeader.arpSenderHardwareAddress);
		
		//��ǻ� proxytable�� �̿��ϴ� ��쿡�� Receive�� ���� �ʰ� send�� ���ָ� �ȴ�, ������ �ʿ��� ��.
		//bSuccess = mp_aUpperLayer[0]->Receive((unsigned char*)pARPFrame->arpData); // ������ ���� request ��Ŷ�� ������������ �÷��ִ� ��.
		bSuccess = mp_UnderLayer->Send((unsigned char*)&arpHeader, ARP_HEADER_SIZE);// ���� reply�� ���־�� �ϴ� �۾��� ó���ϴ� ��.

		
		// ----����---
		// 1. Gratitous���, ���̺� ���Ÿ� ���ְ�, �ƹ��� �۾��� ��������.
		// 2. ���� ��Ŷ�� ������ ip�� �ڽ��� ip�� ����, ���� ��Ŷ�� reply���, ���� �������� �÷��ְ� �۾� ��.
		// 3. ���� ��Ŷ�� ������ ip�� �ڽ��� ip�� ����, ���� ��Ŷ�� request���,  reply ��Ŷ�� ���� send�� ���ְ�, ���� request ��Ŷ�� ������������ �÷���.
		// 4. ���� ��Ŷ�� ������ ip�� proxy table�� �����ϴ� ip�� ��ġ�Ѵٸ�, ����ؼ� mac�ּ� �������� �� reply ��Ŷ�� ����� send�� ��.


		//discard this message.
		return bSuccess;
	}
	else	//gratitous �� �߻� �ߴٸ� �׳ɸ���, ��� ���̺��Ÿ� ���ָ� �۾��� ������ ���̹Ƿ�.
		return TRUE;
}