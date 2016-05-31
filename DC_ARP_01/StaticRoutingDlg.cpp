// StaticRoutingDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "DC_ARP_01.h"
#include "StaticRoutingDlg.h"
#include "afxdialogex.h"
#include "IPLayer.h"


// CStaticRoutingDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(CStaticRoutingDlg, CDialogEx)

CStaticRoutingDlg::CStaticRoutingDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CStaticRoutingDlg::IDD, pParent)
{

}

CStaticRoutingDlg::~CStaticRoutingDlg()
{
}

void CStaticRoutingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_ROUTE_DST_IP, RouteDstIPAddress);
	DDX_Control(pDX, IDC_STATIC_ROUTE_NETMASK_IP, RouteNetmaskIPAddress);
	DDX_Control(pDX, IDC_STATIC_ROUTE_GATEWAY_IP, RouteGatewayIPAddress);
	DDX_Control(pDX, IDC_FLAG_UP_CHECKBOX, RouteFlagUpCheck);
	DDX_Control(pDX, IDC_FLAG_GATEWAY_CHECKBOX, RouteFlagGatewayCheck);
	DDX_Control(pDX, IDC_FLAG_HOST_CHECKBOX, RouteFlagHostCheck);
	DDX_Control(pDX, IDC_STATIC_ROUTE_DEVICE, RouteInterfaceCombo);
	DDX_Control(pDX, IDC_METRIC_EDIT_CONTROL, RouteMetric);
}


BEGIN_MESSAGE_MAP(CStaticRoutingDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CStaticRoutingDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CStaticRoutingDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CStaticRoutingDlg 메시지 처리기입니다.


BOOL CStaticRoutingDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	this->RouteInterfaceCombo.AddString("interface");

	return TRUE;
}


void CStaticRoutingDlg::OnBnClickedOk()
{
	UpdateData( TRUE );
	RouteDstIPAddress.GetAddress(dstIPAddress[0],dstIPAddress[1],dstIPAddress[2],dstIPAddress[3]);
	RouteNetmaskIPAddress.GetAddress(netmaskIPAddress[0],netmaskIPAddress[1],netmaskIPAddress[2],netmaskIPAddress[3]);
	RouteGatewayIPAddress.GetAddress(gatewayIPAddress[0],gatewayIPAddress[1],gatewayIPAddress[2],gatewayIPAddress[3]);
	
	BOOL isUPChecked = IsDlgButtonChecked(IDC_FLAG_UP_CHECKBOX);
	BOOL isGatewayChecked = IsDlgButtonChecked(IDC_FLAG_GATEWAY_CHECKBOX);
	BOOL isHostChecked = IsDlgButtonChecked(IDC_FLAG_HOST_CHECKBOX);
	flag = 0x0;
	flag_string = "";
	if(isUPChecked)
	{
		flag |= FLAG_UP;
		flag_string.Append("U");
	}
	if(isGatewayChecked)
	{
		flag |= FLAG_GATEWAY;
		flag_string.Append("G");
	}
	else
		flag_string.Append("  ");
	if(isHostChecked)
	{
		flag |= FLAG_HOST_SPECIFIT;
		flag_string.Append("H");
	}
	else
		flag_string.Append("  ");
	int n = RouteInterfaceCombo.GetCurSel();
	RouteInterfaceCombo.GetLBText(n,interface_info);
	
	GetDlgItem(IDC_METRIC_EDIT_CONTROL)->GetWindowTextA(metric);
	CDialogEx::OnOK();
}


void CStaticRoutingDlg::OnBnClickedCancel()
{
	CDialogEx::OnCancel();
}
