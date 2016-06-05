// StaticRoutingDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "DC_ARP_01.h"
#include "StaticRoutingDlg.h"
#include "afxdialogex.h"
#include "IPLayer.h"
#include "DC_ARP_01Dlg.h"

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
	DDX_Control(pDX, IDC_STATIC_ROUTE_SOURCE_IP, RouteSrcIPAddress);
	DDX_Control(pDX, IDC_STATIC_ROUTE_SOURCE_NETMASK, RouteSrcNetmask);
}


BEGIN_MESSAGE_MAP(CStaticRoutingDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CStaticRoutingDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CStaticRoutingDlg::OnBnClickedCancel)
	ON_NOTIFY(IPN_FIELDCHANGED, IDC_STATIC_ROUTE_DST_IP, &CStaticRoutingDlg::OnIpnFieldchangedStaticRouteDstIp)
	ON_CBN_SELCHANGE(IDC_STATIC_ROUTE_DEVICE, OnComboEnetAddr)
	ON_NOTIFY(IPN_FIELDCHANGED, IDC_STATIC_ROUTE_SOURCE_IP, &CStaticRoutingDlg::OnIpnFieldchangedStaticRouteSourceIp)
END_MESSAGE_MAP()


// CStaticRoutingDlg 메시지 처리기입니다.


BOOL CStaticRoutingDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	CString device_description;

	
	for(int i = 0; i < NI_COUNT_NIC; i++){
		if(!((CDC_ARP_01Dlg*)GetParent())->m_NI->GetAdapterObject(i))
			break;
		device_description = ((CDC_ARP_01Dlg*)GetParent())->m_NI->GetAdapterObject(i)->description;
		device_description.Trim();
		RouteInterfaceCombo.AddString(device_description);
		RouteInterfaceCombo.SetCurSel(0);

		PPACKET_OID_DATA OidData;
		OidData = (PPACKET_OID_DATA)malloc(sizeof(PACKET_OID_DATA));
		OidData->Oid = 0x01010101;
		OidData->Length = 6;

		LPADAPTER adapter = PacketOpenAdapter(((CDC_ARP_01Dlg*)GetParent())->m_NI->GetAdapterObject(0)->name);
		PacketRequest(adapter, FALSE, OidData);

		srcMACAddress.Format("%.2X%.2X%.2X%.2X%.2X%.2X",OidData->Data[0],OidData->Data[1],OidData->Data[2],OidData->Data[3],OidData->Data[4],OidData->Data[5]) ;
	}
	return TRUE;
}


void CStaticRoutingDlg::OnComboEnetAddr()
{
	UpdateData(TRUE);

	int nIndex = RouteInterfaceCombo.GetCurSel();
	((CDC_ARP_01Dlg*)GetParent())->m_NI->GetAdapterObject(nIndex)->name; //뭐 골랏는지.

	PPACKET_OID_DATA OidData; //공간할당
	OidData = (PPACKET_OID_DATA)malloc(sizeof(PACKET_OID_DATA));
	OidData->Oid = 0x01010101;
	OidData->Length = 6;

	LPADAPTER adapter = PacketOpenAdapter(((CDC_ARP_01Dlg*)GetParent())->m_NI->GetAdapterObject(nIndex)->name);
	CString device_description = ((CDC_ARP_01Dlg*)GetParent())->m_NI->GetAdapterObject(nIndex)->description;
	device_description.Trim();

	PacketRequest(adapter, FALSE, OidData);

	srcMACAddress.Format("%.2X%.2X%.2X%.2X%.2X%.2X",OidData->Data[0],OidData->Data[1],OidData->Data[2],OidData->Data[3],OidData->Data[4],OidData->Data[5]) ;

	UpdateData(FALSE);
}
void CStaticRoutingDlg::OnBnClickedOk()
{
	UpdateData( TRUE );
	RouteDstIPAddress.GetAddress(dstIPAddress[0],dstIPAddress[1],dstIPAddress[2],dstIPAddress[3]);
	RouteNetmaskIPAddress.GetAddress(netmaskIPAddress[0],netmaskIPAddress[1],netmaskIPAddress[2],netmaskIPAddress[3]);
	RouteGatewayIPAddress.GetAddress(gatewayIPAddress[0],gatewayIPAddress[1],gatewayIPAddress[2],gatewayIPAddress[3]);
	RouteSrcIPAddress.GetAddress(srcIPAddress[0],srcIPAddress[1],srcIPAddress[2],srcIPAddress[3]);

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

	int bit_mask = 0x1;
	int additional_bit_mask = 8;
	for(int i = 0; i < 8; i++)
	{
		if(bit_mask & dstIPAddress[3])
		{
			netmaskLength += additional_bit_mask;
			break;
		}
		else
		{
			bit_mask << 1;
			additional_bit_mask -= 1;
		}
	}
	int n = RouteInterfaceCombo.GetCurSel();
	selectedRow = n;
	RouteInterfaceCombo.GetLBText(n,interface_info);
	
	GetDlgItem(IDC_METRIC_EDIT_CONTROL)->GetWindowTextA(metric);
	CDialogEx::OnOK();
}


void CStaticRoutingDlg::OnBnClickedCancel()
{
	CDialogEx::OnCancel();
}


void CStaticRoutingDlg::OnIpnFieldchangedStaticRouteDstIp(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMIPADDRESS pIPAddr = reinterpret_cast<LPNMIPADDRESS>(pNMHDR);

	RouteDstIPAddress.GetAddress(dstIPAddress[0],dstIPAddress[1],dstIPAddress[2],dstIPAddress[3]);
	
	int firstByteVerified = VERIFY_BYTE(dstIPAddress[0]);
	int secondByteVerified = VERIFY_BYTE(dstIPAddress[1]);
	int thirdByteVerified = VERIFY_BYTE(dstIPAddress[2]);
	netmaskLength = 0;

	if ((firstByteVerified > 0)  && (secondByteVerified > 0) && (thirdByteVerified > 0))
	{
		RouteNetmaskIPAddress.SetAddress(FULL_BYTE, FULL_BYTE, FULL_BYTE, 0);
		netmaskLength = 24;
	}
	else if((firstByteVerified > 0)  && (secondByteVerified > 0))
	{
		RouteNetmaskIPAddress.SetAddress(FULL_BYTE, FULL_BYTE, 0, 0);
		netmaskLength = 16;
	}
	else if((firstByteVerified > 0))
	{
		RouteNetmaskIPAddress.SetAddress(FULL_BYTE, 0, 0, 0);
		netmaskLength = 8;
	}

	*pResult = 0;
}

void CStaticRoutingDlg::OnIpnFieldchangedStaticRouteSourceIp(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMIPADDRESS pIPAddr = reinterpret_cast<LPNMIPADDRESS>(pNMHDR);

	RouteSrcIPAddress.GetAddress(srcIPAddress[0],srcIPAddress[1],srcIPAddress[2],srcIPAddress[3]);
	
	int firstByteVerified = VERIFY_BYTE(srcIPAddress[0]);
	int secondByteVerified = VERIFY_BYTE(srcIPAddress[1]);
	int thirdByteVerified = VERIFY_BYTE(srcIPAddress[2]);

	if ((firstByteVerified > 0)  && (secondByteVerified > 0) && (thirdByteVerified > 0))
	{
		RouteSrcNetmask.SetAddress(FULL_BYTE, FULL_BYTE, FULL_BYTE, 0);
	}
	else if((firstByteVerified > 0)  && (secondByteVerified > 0))
	{
		RouteSrcNetmask.SetAddress(FULL_BYTE, FULL_BYTE, 0, 0);
	}
	else if((firstByteVerified > 0))
	{
		RouteSrcNetmask.SetAddress(FULL_BYTE, 0, 0, 0);
	}
	*pResult = 0;
}
