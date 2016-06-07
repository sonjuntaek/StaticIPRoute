// NILayer.h: interface for the CNILayer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NILayer_H__7857C9C2_B459_4DC8_B9B3_4E6C8B587B29__INCLUDED_)
#define AFX_NILayer_H__7857C9C2_B459_4DC8_B9B3_4E6C8B587B29__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseLayer.h"
#include <pcap.h>
#include <Packet32.h>

#define MAX_ADAPTER_COUNT 8

class CNILayer 
	: public CBaseLayer
{
protected:
	pcap_t			*m_AdapterObject;

public:
	BOOL			m_thrdSwitch;
	unsigned char*  m_ppayload;
	pcap_t*			adapterOpenedList[MAX_ADAPTER_COUNT];
	unsigned int	adapterOpenedIndexList[MAX_ADAPTER_COUNT];
	unsigned int	adapterOpenedSize;

	void			PacketStartDriver();

	pcap_if_t 		*GetAdapterObject(int iIndex);
	void			SetAdapterNumber(int iNum);
	void			SetAdapterList(LPADAPTER *plist);
	void			SetOpenedAdapterObject(int index);
	void			startThread();

	static UINT		ReadingThread(LPVOID pParam);
	static UINT		FileTransferThread(LPVOID pParam);
	PPACKET_OID_DATA& getNICAddress(int index);
	BOOL			Receive( unsigned char *ppayload, int adapter_number) ;
	//BOOL			Receive( unsigned char *ppayload) ;
	BOOL			Send( unsigned char *ppayload, int nlength );

	CNILayer( char* pName, LPADAPTER *pAdapterObject = NULL, int iNumAdapter = 0 );
	virtual ~CNILayer();

protected:
	int			m_iNumAdapter;
	pcap_if_t	*m_pAdapterList[NI_COUNT_NIC];
};

#endif // !defined(AFX_NILayer_H__7857C9C2_B459_4DC8_B9B3_4E6C8B587B29__INCLUDED_)
