#include "StdAfx.h"
#include "ApplicationLayer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CApplicationLayer::CApplicationLayer( char* pName )
: CBaseLayer( pName ),
	mp_Dlg( NULL )
{
	ResetHeader();
}

CApplicationLayer::~CApplicationLayer()
{
}

void CApplicationLayer::ResetHeader( )
{
	m_sHeader.app_totlen  = 0x0000 ;
	m_sHeader.app_type    = 0x00 ;

	memset( m_sHeader.app_data, 0, APP_DATA_SIZE ) ;
}

BOOL CApplicationLayer::Send(unsigned char* ppayload, int nlength)
{
	BOOL bSuccess = TRUE;

	m_ppayload = ppayload;
	m_length = nlength;

   if(nlength <= APP_DATA_SIZE){
      m_sHeader.app_totlen = nlength;
      memcpy(m_sHeader.app_data,ppayload,nlength);
      bSuccess = mp_UnderLayer->Send((unsigned char*) &m_sHeader, nlength+APP_HEADER_SIZE);
   }
   return bSuccess;
}


BOOL CApplicationLayer::Receive(unsigned char* ppayload)
{
	BOOL bSuccess = TRUE;
	PAPP_HEADER pHeader = (PAPP_HEADER)ppayload;	
	bSuccess = mp_aUpperLayer[0]->Receive((unsigned char*)pHeader->app_data);
	
	return bSuccess;
}
