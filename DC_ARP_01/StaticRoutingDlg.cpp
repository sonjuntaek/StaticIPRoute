// StaticRoutingDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "DC_ARP_01.h"
#include "StaticRoutingDlg.h"
#include "afxdialogex.h"


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
}


BEGIN_MESSAGE_MAP(CStaticRoutingDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CStaticRoutingDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CStaticRoutingDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CStaticRoutingDlg 메시지 처리기입니다.




void CStaticRoutingDlg::OnBnClickedOk()
{
	CDialogEx::OnOK();
}


void CStaticRoutingDlg::OnBnClickedCancel()
{
	CDialogEx::OnCancel();
}
