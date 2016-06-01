#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#define VERIFIER			0xFF
#define FULL_BYTE			0xFF

#define VERIFY_BYTE(X)		((X) & (VERIFIER))

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
	virtual BOOL OnInitDialog();

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
	unsigned char		gatewayIPAddress[4];
	unsigned char		flag;
	unsigned char		netmaskLength;
	CString				flag_string;
	CString				interface_info;
	CString				metric;

	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CEdit RouteMetric;
	afx_msg void OnIpnFieldchangedStaticRouteDstIp(NMHDR *pNMHDR, LRESULT *pResult);
};
