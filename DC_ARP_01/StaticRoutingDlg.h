#pragma once
#include "afxwin.h"
#include "afxcmn.h"

// CStaticRoutingDlg 대화 상자입니다.

class CStaticRoutingDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CStaticRoutingDlg)

public:
	CStaticRoutingDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CStaticRoutingDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_STATIC_IP_Dlg };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()

public:

	CIPAddressCtrl		RouteDstIPAddress;
	CIPAddressCtrl		RouteNetmaskIPAddress;
	CIPAddressCtrl		RouteGatewayIPAddress;
	CButton				RouteFlagUpCheck;
	CButton				RouteFlagGatewayCheck;
	CButton				RouteFlagHostCheck;
	CComboBox			RouteInterfaceCombo;

	unsigned char		dstIPAddress[4];
	unsigned char		netmaskIPAddress[4];
	unsigned char		gatewayIPaddress[4];
	unsigned char		flag;
	CString				interface_info;
	unsigned char		metric;

	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
};
