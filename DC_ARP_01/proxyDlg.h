#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// proxyDlg 대화 상자입니다.

class proxyDlg : public CDialogEx
{
	DECLARE_DYNAMIC(proxyDlg)

public:
	proxyDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~proxyDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_PROXY_Dlg };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	virtual BOOL OnInitDialog();
	

	DECLARE_MESSAGE_MAP()
public:
	CComboBox			device;
	CIPAddressCtrl		proxyIPAddr;
	CEdit				proxyEthAddr;
	unsigned char		proxyIPAddrString[4];
	unsigned char		proxyMACAddr[6];
	CString				proxyAddr;
	CString				selectedDevice;
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
};
