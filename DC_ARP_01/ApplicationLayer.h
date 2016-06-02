#pragma once

#include "BaseLayer.h"

class CApplicationLayer
: public CBaseLayer  
{
private:
	inline void		ResetHeader( ) ;
	CObject* mp_Dlg ;

public:
	unsigned char	*m_ppayload;
	int				m_length;

	BOOL Send(unsigned char* ppayload, int nlength);
	BOOL Receive(unsigned char* ppayload);

	CApplicationLayer( char* pName );
	virtual ~CApplicationLayer();

	typedef struct _APP_HEADER {
		unsigned long	app_seq_num;	// APP Sequential Number
		unsigned short	app_totlen	;	// total length of the data
		unsigned char	app_type	;	// type of application data

		unsigned char	app_data[ APP_DATA_SIZE ] ; // application data

	} APP_HEADER, *PAPP_HEADER ;

protected:
	APP_HEADER		m_sHeader ;
};

