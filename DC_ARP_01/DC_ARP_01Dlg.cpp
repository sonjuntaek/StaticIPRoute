
// DC_ARP_01Dlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "DC_ARP_01.h"
#include "DC_ARP_01Dlg.h"
#include "afxdialogex.h"
#include "ARPLayer.h"
#include "IPLayer.h"
#include "proxyDlg.h"
#include "StaticRoutingDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();
// 대화 상자 데이터입니다.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CDC_ARP_01Dlg 대화 상자




CDC_ARP_01Dlg::CDC_ARP_01Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDC_ARP_01Dlg::IDD, pParent),
	CBaseLayer( "ArpDlg" ),
	m_bSendReady(FALSE)
	, m_unGratuitousAddressstes(_T(""))
{

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_LayerMgr.AddLayer( this ) ;
	m_LayerMgr.AddLayer( new CNILayer( "NI" ) ) ;
	m_LayerMgr.AddLayer( new CEthernetLayer( "Ethernet" ) ) ;
	m_LayerMgr.AddLayer( new CARPLayer( "ARP" ) );
	m_LayerMgr.AddLayer( new CIPLayer( "IP" ) );
	m_LayerMgr.AddLayer( new CApplicationLayer( "APP" ) ) ;

	m_LayerMgr.ConnectLayers("NI ( *Ethernet ( *ARP ( *IP ( *APP ( *ArpDlg ) ) ) +IP ( *APP ( *ArpDlg ) ) ) )");

	m_APP = (CApplicationLayer *)mp_UnderLayer;
	m_IP = (CIPLayer *)m_LayerMgr.GetLayer("IP");
	m_ARP = (CARPLayer *)m_LayerMgr.GetLayer("ARP");
	m_ETH = (CEthernetLayer *)m_LayerMgr.GetLayer("Ethernet");
	m_NI = (CNILayer *)m_LayerMgr.GetLayer("NI");
}

void CDC_ARP_01Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ARP_CACHE_TABLE_LIST, m_ArpTable);
	DDX_Control(pDX, IDC_PROXY_ARP_ENTRY_LIST, m_proxyARPEntry);
	DDX_Control(pDX, IDC_STATIC_ROUTING_TABLE, m_staticIPTable);
}

BEGIN_MESSAGE_MAP(CDC_ARP_01Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()

	ON_BN_CLICKED(IDC_ARP_ITEM_DELETE_BUTTON, &CDC_ARP_01Dlg::OnBnClickedArpItemDeleteButton)
	ON_BN_CLICKED(IDC_ARP_ALL_DELETE_BUTTON, &CDC_ARP_01Dlg::OnBnClickedArpAllDeleteButton)
	ON_BN_CLICKED(IDC_ARP_SEND_BUTTON, OnSendMessage)
	ON_BN_CLICKED(IDC_WINDOW_OK_BUTTON, &CDC_ARP_01Dlg::OnBnClickedWindowOkButton)
	ON_BN_CLICKED(IDC_ARP_SETTING_BUTTON, OnButtonAddrSet)
	ON_CBN_SELCHANGE(IDC_NICARD_COMBO, OnComboEnetAddr)

	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_PROXY_ADD_BUTTON, &CDC_ARP_01Dlg::OnBnClickedProxyAddButton)
	ON_BN_CLICKED(IDC_PROXY_DELETE_BUTTON, &CDC_ARP_01Dlg::OnBnClickedProxyDeleteButton)
	ON_BN_CLICKED(IDC_WINDOW_CLOSE_BUTTON, &CDC_ARP_01Dlg::OnBnClickedWindowCloseButton)
	ON_BN_CLICKED(IDC_ROUTING_ADD_BUTTON, &CDC_ARP_01Dlg::OnBnClickedRoutingAddButton)
	ON_BN_CLICKED(IDC_ROUTING_DELETE_BUTTON, &CDC_ARP_01Dlg::OnBnClickedRoutingDeleteButton)
END_MESSAGE_MAP()


// CDC_ARP_01Dlg 메시지 처리기

BOOL CDC_ARP_01Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	m_staticIPTable.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	char* columnText[7] = {"", "Destination", "Netmask", "Gateway", "Flag", "Interface", "Metric"};
	int tabWidth[7] = {1,115,115,115,50,90,45};
	LV_COLUMN levCol;

	levCol.mask = LVCF_FMT|LVCF_SUBITEM|LVCF_TEXT|LVCF_WIDTH;
	levCol.fmt = LVCFMT_CENTER;
	for(int i=0; i<7; i++){

		levCol.pszText=columnText[i];  //칼럼의 제목을 지정
		levCol.iSubItem=i;  //서브아이템의 인덱스를 지정
		levCol.cx=tabWidth[i];  //칼럼의 넓이를 지정
		m_staticIPTable.InsertColumn(i,&levCol);  //LVCOLUMN구조체로 만들어진 값을 토대로 리스트 컨트롤에 칼럼을 삽입
	}	

	ifstream inFile;
	inFile.open("input.txt");
	setRoutingTable(inFile);
	inFile.close();

	// 이 대화 상자의 아이콘을 설정합니다. 응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	SetRegstryMessage( ) ;
	SetTimer(2, 2000, NULL); // Timer 동작함. arptable을 2초마다 갱신해줌.
	SetDlgState(IPC_INITIALIZING);

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CDC_ARP_01Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ( nID == SC_CLOSE )
	{
		if ( MessageBox( "Are you sure ?", 
			"Question", 
			MB_YESNO | MB_ICONQUESTION ) 
			== IDNO )
			return ;
		else EndofProcess( ) ;
	}

	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다. 문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CDC_ARP_01Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CDC_ARP_01Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CDC_ARP_01Dlg::OnSendMessage()  
{
	unsigned char dst_ip[4];
	// TODO: Add your control notification handler code here
	UpdateData( TRUE ) ;

	SetTimer(1,3000,NULL);// file transfer 할 때 응답이 없으면 오류메세지 띄울 때 사용된건데 사실상 여기선 필요 없음.
	m_unDstIPAddr.GetAddress(dst_ip[0],dst_ip[1],dst_ip[2],dst_ip[3]);
	m_ARP->setTargetIPAddress(dst_ip);
	
	SendData( ) ;
	
	UpdateData( FALSE ) ;
}
void CDC_ARP_01Dlg::OnButtonAddrSet() //세팅버튼 눌렀을 때.
{
	// TODO: Add your control notification handler code here
	UpdateData( TRUE ) ;
	unsigned char src_ip[4];
	unsigned char dst_ip[4];
	unsigned char src_mac[12];
	unsigned char dst_mac[12];


	if ( !m_unDstEnetAddr || 
		!m_unSrcEnetAddr  )
	{
		MessageBox( "주소를 설정 오류발생", 
			"경고", 
			MB_OK | MB_ICONSTOP ) ;

		return ;
	}
	//setting button 눌렀을 때는 자신의 ip, mac 상대방의 ip가 설정됨.
	if ( m_bSendReady ){
		SetDlgState( IPC_ADDR_RESET ) ;
		SetDlgState( IPC_INITIALIZING ) ;
	}
	else{
		m_unSrcIPAddr.GetAddress(src_ip[0],src_ip[1],src_ip[2],src_ip[3]);
		m_unDstIPAddr.GetAddress(dst_ip[0],dst_ip[1],dst_ip[2],dst_ip[3]);

		m_IP->SetSrcIPAddress(src_ip);
		m_ARP->setSenderIPAddress(src_ip);

		sscanf(m_unSrcEnetAddr, "%02x%02x%02x%02x%02x%02x", &src_mac[0],&src_mac[1],&src_mac[2],&src_mac[3],&src_mac[4],&src_mac[5]);
		sscanf(m_unDstEnetAddr, "%02x%02x%02x%02x%02x%02x", &dst_mac[0],&dst_mac[1],&dst_mac[2],&dst_mac[3],&dst_mac[4],&dst_mac[5]);
		AfxMessageBox(m_unSrcEnetAddr);

		m_ETH->SetEnetSrcAddress(src_mac);
		m_ETH->SetEnetDstAddress(dst_mac);
		m_ARP->setSenderHardwareAddress(src_mac);

		int nIndex = m_ComboEnetName.GetCurSel();
		m_NI->SetAdapterNumber(nIndex);

		m_NI->PacketStartDriver();

		SetDlgState( IPC_ADDR_SET ) ;
		SetDlgState( IPC_READYTOSEND ) ;		
	}

	m_bSendReady = !m_bSendReady ;
}

void CDC_ARP_01Dlg::SendData()
{

	m_stMessage.SetString("Hello World!");
	int nlength = m_stMessage.GetLength();
	unsigned char* ppayload = new unsigned char[nlength+1];
	memcpy(ppayload,(unsigned char*)(LPCTSTR)m_stMessage,nlength);
	ppayload[nlength] = '\0';
	
	m_ARP->setSenderIPAddress((unsigned char*)srcIPAddrString);

	m_APP->Send(ppayload,m_stMessage.GetLength());
}

BOOL CDC_ARP_01Dlg::Receive(unsigned char *ppayload)
{
	CString Msg;
	int len_ppayload = strlen((char *)ppayload); 

	unsigned char *GetBuff = (unsigned char *)malloc(len_ppayload);
	memset(GetBuff,0,len_ppayload);
	memcpy(GetBuff,ppayload,len_ppayload);
	GetBuff[len_ppayload] = '\0';

	KillTimer(1);
	
	return TRUE ;
}

void CDC_ARP_01Dlg::SetDlgState(int state) // 다이얼로그 초기화 부분
{
	UpdateData( TRUE ) ;
	int i;
	CString device_description;

	CButton*			pARPItemDeleteButton = (CButton*) GetDlgItem( IDC_ARP_ITEM_DELETE_BUTTON ) ;
	CButton*			pARPAllDeleteButton = (CButton*) GetDlgItem( IDC_ARP_ALL_DELETE_BUTTON ) ;
	CButton*			pWindowOkButton = (CButton*) GetDlgItem( IDC_WINDOW_OK_BUTTON ) ;
	CButton*			pWindowCloseButton = (CButton*) GetDlgItem( IDC_WINDOW_CLOSE_BUTTON ) ;
	CButton*			pProxyDeleteButton = (CButton*) GetDlgItem( IDC_PROXY_DELETE_BUTTON ) ;
	CButton*			pProxyAddButton = (CButton*) GetDlgItem( IDC_PROXY_ADD_BUTTON  ) ;

	//CEdit*				pGratuitousARPAddress = (CEdit*) GetDlgItem( IDC_GRATUITOUS_ADDRESS_BOX ) ;

	CComboBox*			pEnetNameCombo = (CComboBox*)GetDlgItem( IDC_NICARD_COMBO );

	switch( state )
	{
	case IPC_INITIALIZING : // 첫 화면 세팅
		pWindowOkButton->EnableWindow( TRUE );
		//pGratuitousARPAddress->EnableWindow ( TRUE );
		m_proxyARPEntry.EnableWindow( FALSE );
		m_ArpTable.EnableWindow( FALSE ) ;
		break ;

	case IPC_READYTOSEND : // Send 버튼을 눌렀을 때 세팅
		//pGratuitousARPAddress->EnableWindow ( TRUE );
		m_proxyARPEntry.EnableWindow( TRUE );

		DWORD dwIP;
		//텍스트에 적힌 값 갖고오는거.

		m_ArpTable.EnableWindow( TRUE ) ;
		break ;

	case IPC_WAITFORACK :	break ;
	case IPC_ERROR :		break ;
	case IPC_ADDR_SET :	// Setting 버튼을 눌렀을 때
		
		pEnetNameCombo->EnableWindow( FALSE );

		m_NI->m_thrdSwitch = TRUE;
		break;
		
	case IPC_ADDR_RESET : // Reset 버튼을 눌렀을 때
		
		pEnetNameCombo->EnableWindow( TRUE );
		m_NI->m_thrdSwitch = FALSE;
		break ;
		
	case CFT_COMBO_SET :
		for(i=0;i<NI_COUNT_NIC;i++){
			if(!m_NI->GetAdapterObject(i))
				break;
			device_description = m_NI->GetAdapterObject(i)->description;
			device_description.Trim();
			pEnetNameCombo->AddString(device_description);
			pEnetNameCombo->SetCurSel(0);

			PPACKET_OID_DATA OidData;
			OidData = (PPACKET_OID_DATA)malloc(sizeof(PACKET_OID_DATA));
			OidData->Oid = 0x01010101;
			OidData->Length = 6;

			LPADAPTER adapter = PacketOpenAdapter(m_NI->GetAdapterObject(0)->name);
			PacketRequest(adapter, FALSE, OidData);

			m_unSrcEnetAddr.Format("%.2X%.2X%.2X%.2X%.2X%.2X",OidData->Data[0],OidData->Data[1],OidData->Data[2],OidData->Data[3],OidData->Data[4],OidData->Data[5]) ;
		}
		break;
	}

	UpdateData( FALSE ) ;
}

void CDC_ARP_01Dlg::OnComboEnetAddr()
{
	UpdateData(TRUE);

	int nIndex = m_ComboEnetName.GetCurSel();
	m_NI->GetAdapterObject(nIndex)->name; //뭐 골랏는지.

	PPACKET_OID_DATA OidData; //공간할당
	OidData = (PPACKET_OID_DATA)malloc(sizeof(PACKET_OID_DATA));
	OidData->Oid = 0x01010101;
	OidData->Length = 6;

	LPADAPTER adapter = PacketOpenAdapter(m_NI->GetAdapterObject(nIndex)->name);
	CString device_description = m_NI->GetAdapterObject(nIndex)->description;
	device_description.Trim();
	m_ARP->setAdapter(device_description);

	PacketRequest(adapter, FALSE, OidData);

	m_unSrcEnetAddr.Format("%.2X%.2X%.2X%.2X%.2X%.2X",OidData->Data[0],OidData->Data[1],OidData->Data[2],OidData->Data[3],OidData->Data[4],OidData->Data[5]) ;

	UpdateData(FALSE);
}

void CDC_ARP_01Dlg::EndofProcess()
{
	m_LayerMgr.DeAllocLayer( ) ;
}

void CDC_ARP_01Dlg::SetRegstryMessage()
{

}
void CDC_ARP_01Dlg::OnTimer(UINT nIDEvent) 
{
	switch(nIDEvent)
	{
	case 1:{
		KillTimer( 1 ) ;
		break;
		   }
	case 2:{
		m_ArpTable.ResetContent();
		list<CARPLayer::ARP_CACHE_RECORD>::iterator cacheIter = m_ARP->arpCacheTable.begin();
		for(cacheIter; cacheIter != m_ARP->arpCacheTable.end(); )
		{
			if(!(--(*cacheIter).lifeTimeCounter))
				m_ARP->arpCacheTable.erase(cacheIter++);
			else
			{
				CString record;
				CString ipAddress;
				ipAddress.Format(" %3d.%3d.%3d.%3d ", (unsigned char)(*cacheIter).ipAddress[0],(unsigned char)(*cacheIter).ipAddress[1],
													(unsigned char)(*cacheIter).ipAddress[2],(unsigned char)(*cacheIter).ipAddress[3] );
			
				record.Append(ipAddress);
				record.Append(getMACAddressString((*cacheIter).ethernetAddress));
				record.Append(getCompleteString((*cacheIter).isComplete));
				m_ArpTable.AddString(record.GetString());
				cacheIter++;
			}
		}
		m_ArpTable.UpdateData(TRUE);
		break;
		   }
	}

	CDialog::OnTimer(nIDEvent);
}

CString CDC_ARP_01Dlg::getCompleteString(BOOL isComplete)
{
	if(isComplete == TRUE)
		return CString(" Complete");
	else
		return CString(" Incomplete");
}

CString CDC_ARP_01Dlg::getMACAddressString(unsigned char* macAddress)
{
	if((macAddress[0] == 0) && (macAddress[1] == 0) &&
		(macAddress[2] == 0) && (macAddress[9] == 0) && 
		(macAddress[10] == 0) && (macAddress[11] == 0))
		return CString(" ??:??:??:??:??:?? ");
	else
	{
		CString returnString;
		returnString.Format(" %2X:%2X:%2X:%2X:%2X:%2X ",
				macAddress[0],macAddress[1],macAddress[2],
				macAddress[3],macAddress[4],macAddress[5]);
		return returnString.GetString();
	}
}

void CDC_ARP_01Dlg::OnBnClickedArpItemDeleteButton()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int i=0;
	int index = m_ArpTable.GetCurSel();
	if(index != LB_ERR) {
		list<CARPLayer::ARP_CACHE_RECORD>::iterator cacheIter = m_ARP->arpCacheTable.begin();
		for(cacheIter; cacheIter != m_ARP->arpCacheTable.end(); cacheIter++)
		{
			if(i == index){
 				cacheIter = m_ARP->arpCacheTable.erase(cacheIter);
				return;
			}
			else
				i++;
		}
	}
}

void CDC_ARP_01Dlg::OnBnClickedArpAllDeleteButton()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_ARP->arpCacheTable.clear();
}

void CDC_ARP_01Dlg::OnBnClickedWindowOkButton()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	AfxGetMainWnd()->PostMessage(WM_COMMAND,ID_APP_EXIT,0);
}

void CDC_ARP_01Dlg::OnIpnFieldchangedArpSendIp(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMIPADDRESS pIPAddr = reinterpret_cast<LPNMIPADDRESS>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	*pResult = 0;
}


void CDC_ARP_01Dlg::OnIpnFieldchangedOwnIpAddress(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMIPADDRESS pIPAddr = reinterpret_cast<LPNMIPADDRESS>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	*pResult = 0;
}

void CDC_ARP_01Dlg::SendDataEditMac(void) //맥주소 바꿔서 보내는 gratitous
{
	unsigned char src_mac[12];
	unsigned char dst_mac[12];
	m_stMessage.SetString("Hello World!");
	int nlength = m_stMessage.GetLength();
	unsigned char* ppayload = new unsigned char[nlength+1];
	memcpy(ppayload,(unsigned char*)(LPCTSTR)m_stMessage,nlength);
	ppayload[nlength] = '\0';
	sscanf(m_unGratuitousAddressstes, "%02x%02x%02x%02x%02x%02x", &src_mac[0],&src_mac[1],&src_mac[2],&src_mac[3],&src_mac[4],&src_mac[5]);//Mac주소 읽어옴.
	m_unDstEnetAddr.Format("%.2x%.2x%.2x%.2x%.2x%.2x",0xff,0xff,0xff,0xff,0xff,0xff) ;
	
	sscanf(m_unDstEnetAddr, "%02x%02x%02x%02x%02x%02x", &dst_mac[0],&dst_mac[1],&dst_mac[2],&dst_mac[3],&dst_mac[4],&dst_mac[5]);
	m_IP->SetSrcIPAddress((unsigned char*)srcIPAddrString);
	m_IP->SetDstIPAddress((unsigned char*)srcIPAddrString);
	m_ARP->setSenderIPAddress((unsigned char*)srcIPAddrString);
	m_ARP->setTargetIPAddress((unsigned char*)srcIPAddrString);

	m_ETH->SetEnetSrcAddress(src_mac);

	m_ARP->setSenderHardwareAddress((unsigned char*)src_mac);
	
	
	m_APP->Send(ppayload,m_stMessage.GetLength());
}

void CDC_ARP_01Dlg::OnEnChangeGratuitousAddressBox()
{
	// TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
	// __super::OnInitDialog() 함수를 재지정 
	//하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
	// 이 알림 메시지를 보내지 않습니다.

	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void CDC_ARP_01Dlg::OnBnClickedProxyAddButton()
{
	CString recordtext;
	CString recordipAddress;
	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	proxyDlg dlg(this);
	int result = dlg.DoModal();
	if(result == IDOK)
	{
		CARPLayer::ARP_CACHE_RECORD newRecord;
		
		newRecord.arpInterface = dlg.selectedDevice;
		memcpy(newRecord.ethernetAddress, dlg.proxyMACAddr, 6);
		memcpy(newRecord.ipAddress, dlg.proxyIPAddrString, 4);
		newRecord.isComplete = TRUE;
		recordtext.Append(newRecord.arpInterface);
		recordipAddress.Format(" %3d.%3d.%3d.%3d ", newRecord.ipAddress[0],newRecord.ipAddress[1],
									newRecord.ipAddress[2],newRecord.ipAddress[3] );
		recordtext.Append(recordipAddress);
		recordtext.Append(getMACAddressString(newRecord.ethernetAddress));
		m_proxyARPEntry.AddString(recordtext);

		m_ARP->arpProxyTable.push_back(newRecord);
	}
}


void CDC_ARP_01Dlg::OnBnClickedProxyDeleteButton()
{
	int i=0;
	int index = m_proxyARPEntry.GetCurSel();
	if(index != LB_ERR) {
		list<CARPLayer::ARP_CACHE_RECORD>::iterator cacheIter = m_ARP->arpProxyTable.begin();
		for(cacheIter; cacheIter != m_ARP->arpProxyTable.end(); cacheIter++)
		{
			if(i == index){
 				cacheIter = m_ARP->arpProxyTable.erase(cacheIter);
				break;
			}
			else
				i++;
		}
	
		m_proxyARPEntry.ResetContent();
		cacheIter = m_ARP->arpProxyTable.begin();
		for(cacheIter; cacheIter != m_ARP->arpProxyTable.end(); cacheIter++)
		{
			CString record;
			CString saveIpaddress;
			record.Append((*cacheIter).arpInterface);
			saveIpaddress.Format(" %3d.%3d.%3d.%3d ", (*cacheIter).ipAddress[0],(*cacheIter).ipAddress[1],
									(*cacheIter).ipAddress[2],(*cacheIter).ipAddress[3] );
			record.Append(saveIpaddress);
			record.Append(getMACAddressString((*cacheIter).ethernetAddress));
			record.Append(getCompleteString((*cacheIter).isComplete));
			m_proxyARPEntry.AddString(record.GetString());
		}
		m_proxyARPEntry.UpdateData(TRUE);
	}
	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
}

void CDC_ARP_01Dlg::OnBnClickedWindowCloseButton()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CDialogEx::OnCancel();
}


void CDC_ARP_01Dlg::setRoutingTable(ifstream& inFile)
{
	if (inFile != NULL)
	{
		readAndWriteTableDataFromFile(inFile);
	}
}

void CDC_ARP_01Dlg::readAndWriteTableDataFromFile(ifstream& inFile)
{
	int count = 0;
	CIPLayer::STATIC_IP_ROUTING_RECORD dlg[5];
	inFile>>count;
	for(int i = 0; i < count; i++)
	{
		dlg[i] = CIPLayer::STATIC_IP_ROUTING_RECORD();
		unsigned char flag_string[10];
		unsigned char interface_info[50];
		unsigned char metric[2];
		unsigned char nIndex;
		unsigned int  ip_data;
		inFile >> ip_data; dlg[i].destination_ip[0] = ip_data;
		inFile >> ip_data; dlg[i].destination_ip[1] = ip_data;
		inFile >> ip_data; dlg[i].destination_ip[2] = ip_data;
		inFile >> ip_data; dlg[i].destination_ip[3] = ip_data;
		
		inFile >> ip_data; dlg[i].netmask_ip[0] = ip_data;
		inFile >> ip_data; dlg[i].netmask_ip[1] = ip_data;
		inFile >> ip_data; dlg[i].netmask_ip[2] = ip_data;
		inFile >> ip_data; dlg[i].netmask_ip[3] = ip_data;
		
		inFile >> ip_data; dlg[i].gateway_ip[0] = ip_data;
		inFile >> ip_data; dlg[i].gateway_ip[1] = ip_data;
		inFile >> ip_data; dlg[i].gateway_ip[2] = ip_data;
		inFile >> ip_data; dlg[i].gateway_ip[3] = ip_data;

		inFile >> ip_data; dlg[i].own_ip[0] = ip_data;
		inFile >> ip_data; dlg[i].own_ip[1] = ip_data;
		inFile >> ip_data; dlg[i].own_ip[2] = ip_data;
		inFile >> ip_data; dlg[i].own_ip[3] = ip_data;

		inFile >> ip_data; dlg[i].flag = ip_data;
		inFile >> flag_string;
		inFile >> interface_info;
		inFile >> metric;
		inFile >> ip_data; dlg[i].netmask_length = ip_data;
		inFile >> ip_data; nIndex = ip_data;

		dlg[i].flag_string.Format("%s",flag_string);
		dlg[i].interface_info.Format("%s",interface_info);
		dlg[i].metric.Format("%s",metric);

		CString recordipAddress;
		char szText[50] = "";
		UpdateData(TRUE);

		LVITEM levItem;

		levItem.mask = LVIF_TEXT;
		levItem.iItem = 0;
		CIPLayer::STATIC_IP_ROUTING_RECORD newRecord;
		memcpy(newRecord.destination_ip,dlg[i].destination_ip,4);
		memcpy(newRecord.netmask_ip,dlg[i].netmask_ip,4);
		memcpy(newRecord.gateway_ip,dlg[i].gateway_ip,4);
		memcpy(newRecord.own_ip,dlg[i].own_ip,4);
		newRecord.flag = dlg[i].flag;
		newRecord.flag_string = dlg[i].flag_string;
		newRecord.interface_info = dlg[i].interface_info;
		newRecord.metric = dlg[i].metric;
		newRecord.netmask_length = dlg[i].netmask_length;
	
		BOOL isDeviceNotOpened = TRUE;
		list<CIPLayer::INTERFACE_STRUCT>::iterator iter = device_list.begin();
		for(; iter != device_list.end(); iter++)
		{
			if((*iter).device_number == nIndex)
			{
				isDeviceNotOpened = FALSE;
				break;
			}
		}
		if(isDeviceNotOpened == TRUE)
		{
			m_NI->SetAdapterNumber(nIndex);
			m_NI->PacketStartDriver();
			CIPLayer::INTERFACE_STRUCT newDevice;
			unsigned char dst_mac[12];
			memset(newDevice.device_ip, 0, 4);
			memset(newDevice.device_mac, 0, 6);
			memcpy(newDevice.device_ip, dlg[i].own_ip, 4);
			memcpy(newDevice.device_mac, m_NI->getNICAddress(nIndex), 6);
			
			memcpy(newDevice.device_mac, dst_mac, 6);
			device_list.push_back(newDevice);
		}	
		
		newRecord.device_number = nIndex;

		levItem.iSubItem = 0;
		sprintf(szText,"%s"," ");
		levItem.pszText=(LPSTR)szText;
		m_staticIPTable.InsertItem(&levItem);
	
		levItem.iSubItem = 1;
		recordipAddress.Format(" %3d.%3d.%3d.%3d  ", newRecord.destination_ip[0],newRecord.destination_ip[1],
									newRecord.destination_ip[2],newRecord.destination_ip[3] );
		sprintf(szText,"%s",recordipAddress);
		levItem.pszText=(LPSTR)szText;
		m_staticIPTable.SetItem(&levItem);
	
		levItem.iSubItem = 2;
		recordipAddress.Format(" %3d.%3d.%3d.%3d  ", newRecord.netmask_ip[0],newRecord.netmask_ip[1],
									newRecord.netmask_ip[2],newRecord.netmask_ip[3] );
		sprintf(szText,"%s",recordipAddress);
		levItem.pszText=(LPSTR)szText;
		m_staticIPTable.SetItem(&levItem);
		
		levItem.iSubItem = 3;
		if(IS_FLAG_GATEWAY(newRecord.flag))
		{
			recordipAddress.Format(" %3d.%3d.%3d.%3d  ", newRecord.gateway_ip[0],newRecord.gateway_ip[1],
									newRecord.gateway_ip[2],newRecord.gateway_ip[3] );
		}
		else
		{
			recordipAddress.SetString("연결됨");
		}
		sprintf(szText,"%s",recordipAddress);
		levItem.pszText=(LPSTR)szText;
		m_staticIPTable.SetItem(&levItem);
	
		levItem.iSubItem = 4;
		sprintf(szText,"%s",newRecord.flag_string);
		levItem.pszText=(LPSTR)szText;
		m_staticIPTable.SetItem(&levItem);
	
		levItem.iSubItem = 5;
		sprintf(szText,"%s",newRecord.interface_info);
		levItem.pszText=(LPSTR)szText;
		m_staticIPTable.SetItem(&levItem);
	
		levItem.iSubItem = 6;
		sprintf(szText,"%s",newRecord.metric);
		levItem.pszText=(LPSTR)szText;
		m_staticIPTable.SetItem(&levItem);

		m_IP->routingTable.push_back(newRecord);
		m_IP->device_list.assign(device_list.begin(), device_list.end());
		m_ETH->device_list.assign(device_list.begin(), device_list.end());
		m_IP->routingTable.sort();
	}
}


void CDC_ARP_01Dlg::OnBnClickedRoutingAddButton()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString recordtext;
	CString recordipAddress;
	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CStaticRoutingDlg dlg(this);
	int result = dlg.DoModal();
	if(result == IDOK)
	{
		char szText[50] = "";
		UpdateData(TRUE);

		LVITEM levItem;

		levItem.mask = LVIF_TEXT;
		levItem.iItem = 0;
		CIPLayer::STATIC_IP_ROUTING_RECORD newRecord;
		
		memcpy(newRecord.destination_ip,dlg.dstIPAddress,4);
		memcpy(newRecord.netmask_ip,dlg.netmaskIPAddress,4);
		memcpy(newRecord.gateway_ip,dlg.gatewayIPAddress,4);
		memcpy(newRecord.own_ip,dlg.srcIPAddress,4);
		newRecord.flag = dlg.flag;
		newRecord.flag_string = dlg.flag_string;
		newRecord.interface_info = dlg.interface_info;
		newRecord.metric = dlg.metric;
		newRecord.netmask_length = dlg.netmaskLength;
		
		int nIndex = dlg.selectedRow;
		BOOL isDeviceNotOpened = TRUE;
		list<CIPLayer::INTERFACE_STRUCT>::iterator iter = device_list.begin();
		for(; iter != device_list.end(); iter++)
		{
			if((*iter).device_number == nIndex)
			{
				isDeviceNotOpened = FALSE;
				break;
			}
		}
		if(isDeviceNotOpened == TRUE)
		{
			m_NI->SetAdapterNumber(nIndex);
			m_NI->PacketStartDriver();

			CIPLayer::INTERFACE_STRUCT newDevice;
			unsigned char dst_mac[12];

			memset(newDevice.device_ip, 0, 4);
			memset(newDevice.device_mac, 0, 6);
			memcpy(newDevice.device_ip, dlg.srcIPAddress, 4);
			sscanf(dlg.srcMACAddress, "%02x%02x%02x%02x%02x%02x", &dst_mac[0],&dst_mac[1],&dst_mac[2],&dst_mac[3],&dst_mac[4],&dst_mac[5]);
			memcpy(newDevice.device_mac, dst_mac, 6);
			
			newDevice.device_number = nIndex;
			device_list.push_back(newDevice);
		}
		
		newRecord.device_number = nIndex;

		levItem.iSubItem = 0;
		sprintf(szText,"%s"," ");
		levItem.pszText=(LPSTR)szText;
		m_staticIPTable.InsertItem(&levItem);
		
		levItem.iSubItem = 1;
		recordipAddress.Format(" %3d.%3d.%3d.%3d  ", newRecord.destination_ip[0],newRecord.destination_ip[1],
									newRecord.destination_ip[2],newRecord.destination_ip[3] );
		sprintf(szText,"%s",recordipAddress);
		levItem.pszText=(LPSTR)szText;
		m_staticIPTable.SetItem(&levItem);
		
		levItem.iSubItem = 2;
		recordipAddress.Format(" %3d.%3d.%3d.%3d  ", newRecord.netmask_ip[0],newRecord.netmask_ip[1],
									newRecord.netmask_ip[2],newRecord.netmask_ip[3] );
		sprintf(szText,"%s",recordipAddress);
		levItem.pszText=(LPSTR)szText;
		m_staticIPTable.SetItem(&levItem);
		
		levItem.iSubItem = 3;
		if(IS_FLAG_GATEWAY(newRecord.flag))
		{
			recordipAddress.Format(" %3d.%3d.%3d.%3d  ", newRecord.gateway_ip[0],newRecord.gateway_ip[1],
									newRecord.gateway_ip[2],newRecord.gateway_ip[3] );
		}
		else
		{
			recordipAddress.SetString("연결됨");
		}
		sprintf(szText,"%s",recordipAddress);
		levItem.pszText=(LPSTR)szText;
		m_staticIPTable.SetItem(&levItem);
		
		levItem.iSubItem = 4;
		sprintf(szText,"%s",newRecord.flag_string);
		levItem.pszText=(LPSTR)szText;
		m_staticIPTable.SetItem(&levItem);
		
		levItem.iSubItem = 5;
		sprintf(szText,"%s",newRecord.interface_info);
		levItem.pszText=(LPSTR)szText;
		m_staticIPTable.SetItem(&levItem);
		
		levItem.iSubItem = 6;
		sprintf(szText,"%s",newRecord.metric);
		levItem.pszText=(LPSTR)szText;
		m_staticIPTable.SetItem(&levItem);

		m_IP->routingTable.push_back(newRecord);
		m_IP->device_list.assign(device_list.begin(), device_list.end());
		m_ETH->device_list.assign(device_list.begin(), device_list.end());
		m_IP->routingTable.sort();
	}
}


void CDC_ARP_01Dlg::OnBnClickedRoutingDeleteButton()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int i=0;
	int index = m_staticIPTable.GetNextItem( -1, LVNI_SELECTED );
	if(index != LB_ERR) {
		list<CIPLayer::STATIC_IP_ROUTING_RECORD>::iterator cacheIter = m_IP->routingTable.begin();
		for(cacheIter, i = 0; cacheIter != m_IP->routingTable.end(); cacheIter++, i++)
		{
			if(i == index){
 				cacheIter = m_IP->routingTable.erase(cacheIter);
				break;
			}
		}
		m_staticIPTable.DeleteItem(index);
		m_proxyARPEntry.UpdateData(TRUE);
	}
}