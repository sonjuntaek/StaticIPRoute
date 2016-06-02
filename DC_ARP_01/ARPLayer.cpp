#include "StdAfx.h"
#include "ARPLayer.h"

CARPLayer::CARPLayer(char* pName)
: CBaseLayer( pName ), ARP_REQUEST(0x100), ARP_REPLY(0x200) // bigendian 으로 가기때문에 이렇게 설정.
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

		BOOL isCacheAvailable = FALSE;	//캐시 이용가능한지.
		BOOL isGratuitousPacket = FALSE;	//gratuitous 발생 했는지.
		list<ARP_CACHE_RECORD>::iterator cacheIter = arpCacheTable.begin();
		if(memcmp(targetIPAddress, ownIPAddress, 4) == 0)	// gratuitous인지 확인.
			isGratuitousPacket = TRUE;
		else
		{
			for(cacheIter; cacheIter != arpCacheTable.end(); cacheIter++)// gratuitous 아니라면, cache에 있는 만큼 for구문돌림.
			{
				if(memcmp((*cacheIter).ipAddress, targetIPAddress, 4) == 0) //보내려는 ip와 같은 ip가 있다면 
				{
					isCacheAvailable = TRUE;
					break;
				}
			}
		}

		//if cache is vaild and complete record
		if((isCacheAvailable == TRUE) && ((*cacheIter).isComplete == TRUE))	// 캐시에 사용가능한 것이 존재한다면, 
		{	
			setTargetHardwareAddress((*cacheIter).ethernetAddress);	//캐시에 있다면 mac 주소를 알게 된 것이므로, 이 mac 주소로 재설정.
			((CEthernetLayer*)GetUnderLayer())->SetEnetDstAddress((*cacheIter).ethernetAddress);// ethernet layer의 mac 주소도 다시 설정.
			((CEthernetLayer*)GetUnderLayer())->SetEnetType( 0x0008 ); //0x0800 //ARP로 보낼 필요 없으므로 IP로 타입 설정해줌 

		}
		else if(isGratuitousPacket == TRUE)
		{
		}
		//it is not valid record
		else // 캐시도 없고, gratuitous도 아니고.
		{
			memset(arpHeader.arpTargetHardwareAddress, 0, 6);
			((CEthernetLayer*)GetUnderLayer())->SetEnetDstAddress(BROADCAST_ADDR);
			((CEthernetLayer*)GetUnderLayer())->SetEnetType( 0x0608 ); //0x0806 ARP


			ARP_CACHE_RECORD newRecord;
			newRecord.arpInterface = this->adapter;
			memset(newRecord.ethernetAddress, 0, 6);
			memcpy(newRecord.ipAddress, targetIPAddress, 4);
			newRecord.isComplete = FALSE;
			newRecord.lifeTimeCounter = INCOMPLETE_DELETE_TIME;

			arpCacheTable.push_back(newRecord);
		}

		// ----정리---
		// 1. Gratuitous가 맞다면, 그냥 아무 작업 하지 않고, 패킷 만들어서 보냄.
		// 2. 캐시 이용가능하다면, 캐시에 있는 값으로 맥주소 설정해서 보냄.
		// 3. 이용가능한 캐시 없고, gratitous가 아니라면 broadcast로 목적지 설정해서 보냄.

		arpHeader.arpHardwareType = 0x0100;
		arpHeader.arpProtocolType = 0x0008;
		arpHeader.arpHardwareAddrSize = 0x6;
		arpHeader.arpProtocolAddrSize = 0x4;
		arpHeader.arpOperationType = ARP_REQUEST;
		memcpy(arpHeader.arpSenderHardwareAddress, ownMACAddress, 6);
		memcpy(arpHeader.arpSenderIPAddress, ownIPAddress, 4);
		memcpy(arpHeader.arpTargetIPAddress, targetIPAddress, 4);
	
		BOOL bSuccess = FALSE ;
		bSuccess = mp_UnderLayer->Send((unsigned char*)&arpHeader,length+ARP_HEADER_SIZE);

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
	memcpy(receivedARPTargetIPAddress, (unsigned char*)pARPFrame->arpTargetIPAddress, 4); //받아온 패킷을 통해 목적지ip, 시작지ip , 시작지mac 을 설정
	memcpy(receivedARPSenderIPAddress, (unsigned char*)pARPFrame->arpSenderIPAddress, 4);
	memcpy(receivedARPSenderHardwareAddress, (unsigned char*)pARPFrame->arpSenderHardwareAddress, 6);
	
	BOOL isARPRecordExist = FALSE;
	list<ARP_CACHE_RECORD>::iterator arpIter = arpCacheTable.begin();
	for(arpIter; arpIter != arpCacheTable.end(); arpIter++) // arp table을 본다.
	{
		if(memcmp((*arpIter).ipAddress,receivedARPSenderIPAddress, 4) == 0) //만약 해당 테이블의 ip 주소와 받은 패킷의 시작지 ip 주소 같다면
		{
			isARPRecordExist = TRUE; //이미 존재한다고 표시.
 			memcpy((*arpIter).ethernetAddress, receivedARPSenderHardwareAddress, 6);// 맥주소를 새로 받아온 걸로 갱신. 자세히는 gratitous, ???? , 중복상태를 나누어야하지만 그냥.. 
																					// 무조건 갱신하도록 함. 
			(*arpIter).isComplete = TRUE; // complete 되었다고 표시.
			(*arpIter).lifeTimeCounter = COMPLETE_DELETE_TIME;
			break;
		}
	}
	if (memcmp(ownIPAddress, receivedARPSenderIPAddress, 4) == 0) // gratitous인지 확인.
		GratitousOccur = TRUE;
	if (memcmp(ownIPAddress, receivedARPTargetIPAddress, 4) != 0) // gratitous아니라는 걸 확인.
		isNotMyIPAddress = TRUE;
		
	BOOL isProxyAvailable = FALSE; //proxy table에 사용될 게 있느냐
	list<ARP_CACHE_RECORD>::iterator proxyIter = arpProxyTable.begin();
	for(proxyIter; proxyIter != arpProxyTable.end(); proxyIter++)	//proxy table 살피기위해 for구문돌림.
	{
		if(memcmp((*proxyIter).ipAddress, receivedARPTargetIPAddress, 4) == 0)	//만약 같은 ip가 있다면. 즉, 받은 패킷을 대신 reply 해 줄 능력이 있느냐.
		{
			isProxyAvailable = TRUE; //proxy table 사용 가능이라 표시.
			break;
		}
	}

	if(GratitousOccur == FALSE)	//Gratitous가 발생하지 않았다면. Gratitous 발생은 arptable 갱신만을 필요로 하므로 위에서 다 처리되었음.
	{
		if(ntohs(pARPFrame->arpOperationType) == ntohs(ARP_REPLY)) // 받은 패킷이 reply라면
		{
			bSuccess = mp_aUpperLayer[0]->Receive((unsigned char*)pARPFrame->arpData); //상위계층으로 올려줌.
			return TRUE;
		}
		if(ntohs(pARPFrame->arpOperationType) == ntohs(ARP_REQUEST))		//받은 패킷이 request라면
		{
			if(isARPRecordExist == FALSE)// arptable에 중복되는 것이 없다면 table에 추가.
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

		// 주목! 이제 여기를 들어오는 건 받은 패킷이 ARP_REQUEST 상태만일 때일 것이다. reply를 보내주는 작업(send)은 진행 될 수 있다.

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
		{ // 받은 패킷의 목적지주소와 자신의 주소가 같다면, 보내줄 reply를 위한 mac주소를 설정한다. ip도 역시 설정해준다.
			memcpy(arpHeader.arpSenderHardwareAddress, ownMACAddress, 6);
			memcpy(arpHeader.arpSenderIPAddress, ownIPAddress, 4);
		}

		if(isProxyAvailable == TRUE) //proxytable 이용 가능하다면,
		{
			memcpy(arpHeader.arpSenderHardwareAddress, (*proxyIter).ethernetAddress, 6); // 그 proxytable에 있는 정보를 갖고 패킷을 형성해준다.
			memcpy(arpHeader.arpSenderIPAddress, (*proxyIter).ipAddress, 4);
		}
		
		if(isNotMyIPAddress == TRUE && isProxyAvailable == FALSE)// proxy도 이용하지 못하고, 목적지 ip와 자신의 ip가 다르니, 버리는 작업.
			return FALSE;
		

		//이제 지금까지 했던 작업을 토대로 나머지 패킷을 만들어주는 작업을 한다.
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
		
		//사실상 proxytable을 이용하는 경우에는 Receive를 하지 않고 send만 해주면 된다, 수정이 필요한 곳.
		bSuccess = mp_aUpperLayer[0]->Receive((unsigned char*)pARPFrame->arpData); // 나한테 들어온 request 패킷을 상위계층으로 올려주는 것.
		bSuccess = mp_UnderLayer->Send((unsigned char*)&arpHeader, ARP_HEADER_SIZE);// 내가 reply를 해주어야 하는 작업을 처리하는 것.

		
		// ----정리---
		// 1. Gratitous라면, 테이블 갱신만 해주고, 아무런 작업을 하지않음.
		// 2. 들어온 패킷의 목적지 ip가 자신의 ip와 같고, 들어온 패킷이 reply라면, 상위 계층으로 올려주고 작업 끝.
		// 3. 들어온 패킷의 목적지 ip가 자신의 ip와 같고, 들어온 패킷이 request라면,  reply 패킷을 만들어서 send를 해주고, 받은 request 패킷은 상위계층으로 올려줌.
		// 4. 들어온 패킷의 목적지 ip와 proxy table에 존재하는 ip가 일치한다면, 대신해서 mac주소 설정해준 뒤 reply 패킷을 만들어 send를 함.


		//discard this message.
		return bSuccess;
	}
	else	//gratitous 가 발생 했다면 그냥리턴, 얘는 테이블갱신만 해주면 작업이 끝나는 것이므로.
		return TRUE;
}