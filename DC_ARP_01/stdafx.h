// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__119ECB1B_6E70_4662_A2A9_A20B5201CA81__INCLUDED_)
#define AFX_STDAFX_H__119ECB1B_6E70_4662_A2A9_A20B5201CA81__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <iostream>
#include <fstream>
#include <cstdlib>
using namespace std;

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#include "targetver.h"
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 일부 CString 생성자는 명시적으로 선언됩니다.
// MFC의 공통 부분과 무시 가능한 경고 메시지에 대한 숨기기를 해제합니다.
#define _AFX_ALL_WARNINGS

#include <afxcontrolbars.h>     // MFC의 리본 및 컨트롤 막대 지원

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

//{{AFX_INSERT_LOCATION}}

#define MAX_LAYER_NUMBER		0xff

#define ETHER_MAX_SIZE			1514
#define ETHER_HEADER_SIZE		( sizeof(unsigned char) * 6 + \
								  sizeof(unsigned char) * 6 + \
								  sizeof(unsigned short))

#define ETHER_MAX_DATA_SIZE		( ETHER_MAX_SIZE - ETHER_HEADER_SIZE )
#define ETHER_PROTO_TYPE_IP		0x0008
#define ETHER_PROTO_TYPE_ARP	0x0608
#define ETHER_PROTO_TYPE_RARP	0x3508

#define ARP_HEADER_SIZE			( sizeof(unsigned short) +	\
								  sizeof(unsigned short) +	\
								  sizeof(unsigned char) + \
								  sizeof(unsigned char) + \
								  sizeof(unsigned short) + \
								  sizeof(unsigned char) * 6 + \
								  sizeof(unsigned char) * 4 + \
								  sizeof(unsigned char) * 6 + \
								  sizeof(unsigned char) * 4 )


#define ARP_DATA_SIZE			( ETHER_MAX_SIZE - (	ARP_HEADER_SIZE  + \
														ETHER_HEADER_SIZE ))

#define IP_HEADER_SIZE			( sizeof(unsigned short) * 4 +	\
								  sizeof(unsigned char) * 12  )
#define IP_DATA_SIZE			( ETHER_MAX_SIZE - (	ARP_HEADER_SIZE + \
														IP_HEADER_SIZE  + \
														ETHER_HEADER_SIZE ))

#define APP_HEADER_SIZE			( sizeof(unsigned short) +					\
								  sizeof(unsigned char)  +				\
								  sizeof(unsigned long) )
#define APP_DATA_SIZE 			( ETHER_MAX_DATA_SIZE - ( APP_HEADER_SIZE +		\
												          IP_HEADER_SIZE +		\
												          ARP_HEADER_SIZE ) )

#define NI_COUNT_NIC	10

unsigned static char BROADCAST_ADDR[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__119ECB1B_6E70_4662_A2A9_A20B5201CA81__INCLUDED_)
