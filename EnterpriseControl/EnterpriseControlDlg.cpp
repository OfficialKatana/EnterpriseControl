
// EnterpriseControlDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EnterpriseControl.h"
#include "EnterpriseControlDlg.h"
#include "afxdialogex.h"
#include "sortdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 额外的上下线处理Hook等等。。。。。。
class UStatusHook
{
public:
	static UStatusHook* hInstance;
	static INT WorkMode;
	BOOL isLocked;
	CRITICAL_SECTION theCrSec;
public:
	// Controller
	struct UserUtil
	{
		CUser theUser;
		time_t theTime;
		// Reserved
		DWORD dwHironFlag;
		// NewFlag
		ULONG newFlag;
		BOOL is_Online;
	};
	enum {
		NATIVEMODE,
		TGMODE,
		EXTRAMODE,
		COCOLHOOKMODE
	};
	union SchNum
	{
		char one;
		short two;
		long four;
		long long eight;
		unsigned char uone;
		unsigned short utwo;
		unsigned int ufour;
		unsigned long long int ueight;
	};
	static list<UserUtil> SortUserDown;
	static UStatusHook* GetInstance()
	{
		if (hInstance == NULL)
		{
			hInstance = new UStatusHook;
			hInstance->isLocked = FALSE;
			InitializeCriticalSection(&(hInstance->theCrSec));
		}
		return hInstance;
	}
public:
	// Logic
	BOOL isEqual(CUser UserOne, CUser UserTwo)
	{
		if (UserOne.m_strIPNet != UserTwo.m_strIPNet)return FALSE;
		if (UserOne.m_strIPLocal != UserTwo.m_strIPLocal)return FALSE;
		// User Eq.
		return TRUE;
	}
	BOOL isUserInList(CUser theUser)
	{
		if (SortUserDown.empty())return FALSE;
		BOOL bFlag = FALSE;
		for (auto dist = SortUserDown.begin(); dist != SortUserDown.end(); dist++)
		{
			if (isEqual(dist->theUser, theUser))
			{
				bFlag = TRUE;
				break;
			}
		}
		return bFlag;
	}
	BOOL RemoveUser(CUser theUser)
	{
		if (SortUserDown.empty())return FALSE;
		auto dist = SortUserDown.begin();
		while (dist != SortUserDown.end())
		{
			if (isEqual(theUser, dist->theUser))
				break;
			dist++;
		}
		if (dist == SortUserDown.end())return FALSE;
		SortUserDown.erase(dist);
		return TRUE;
	}
	BOOL UpdateUserInfo(CUser newUser, BOOL is_Online)
	{
		if (SortUserDown.empty())return FALSE;
		while (isLocked)
		{
			Yield();
		}
		isLocked = TRUE;
		auto dist = SortUserDown.begin();
		while (dist != SortUserDown.end())
		{
			if (isEqual(newUser, dist->theUser))
				break;
			dist++;
		}
		if (dist == SortUserDown.end())
		{
			isLocked = FALSE;
			return FALSE;
		}
		dist->newFlag = newUser.m_nConnID;
		dist->theTime = time(0);
		dist->is_Online = is_Online;
		isLocked = FALSE;
		return TRUE;
	}
	BOOL AppendUserToSuspicious(CUser theUser)
	{
		if (isUserInList(theUser))
		{
			return FALSE;
		}
		UserUtil Util;
		Util.theTime = time(0);
		Util.theUser = theUser;
		Util.dwHironFlag = 0x8000;
		Util.newFlag = theUser.m_nConnID;
		SortUserDown.push_back(Util);
		return TRUE;
	}
	BOOL ResetToLongConn(ULONG ConnID)
	{
		auto pBusiness = CMainBusiness::GetInstance();
		if (!pBusiness)return FALSE;
		auto ReportBuffer = pBusiness->GetReport();
		if (ReportBuffer.GetAllGroupUserSize())
			theWorkThread(pBusiness);
		else
			theWorkThread(0);
		SchNum XNUM;
		XNUM.ufour = (ULONG)pBusiness;
		XNUM.two = MAKEWORD(1, 0);
		if ((ULONG)pBusiness&XNUM.ufour == 0x8120) // MAGIC
			return FALSE;
		return TRUE;
	}
	VOID DoClearOld(CEnterpriseControlDlg* theDlg)
	{
		if (isLocked)return;
		isLocked = TRUE;
		while (SortUserDown.size() > 200ULL)SortUserDown.pop_back();

		time_t nowTime = time(0);

		for (auto nBegin = SortUserDown.begin(); nBegin != SortUserDown.end(); nBegin++)
		{
			if (nowTime - nBegin->theTime > 180LL)
			{
				if (!nBegin->is_Online)
					theDlg->DelItemUserReal(nBegin->theUser);
				if (RemoveUser(nBegin->theUser))
					nBegin = SortUserDown.begin();
			}
		}
		isLocked = FALSE;
	}
	DWORD theWorkThread(void FAR* lpArg)
	{
		if (!lpArg&&!hInstance)WorkMode = TGMODE;
		if (lpArg&&hInstance)WorkMode = NATIVEMODE;
		if (!lpArg&&hInstance)WorkMode = EXTRAMODE;
		if (lpArg&&!hInstance)WorkMode = COCOLHOOKMODE;
		return 0;
	}
	BOOL DoHookAddUser(CUser theUser, CEnterpriseControlDlg* theDlg)
	{
		auto bRet = FALSE;
		if (UpdateUserInfo(theUser, TRUE))bRet = TRUE;
		DoClearOld(theDlg);
		return bRet;
	}
	VOID DoHookRemUser(CUser theUser, CEnterpriseControlDlg* theDlg)
	{
		if (!AppendUserToSuspicious(theUser))
		{
			// 网络不稳定
			if (!UpdateUserInfo(theUser, FALSE))
			{
				theDlg->AddLog("警告，程序逻辑出现严重问题，可能导致难以预料的后果。");
			}
		}
		DoClearOld(theDlg);
		theDlg->OnRemoveUser(theUser);
		theDlg->UpdateConnCount();
		return;
	}
	// 映射。
	static ULONG GetMappedConnID(ULONG theConnId)
	{
		for (auto findOp = SortUserDown.begin(); findOp != SortUserDown.end(); findOp++)
		{
			if (findOp->newFlag == theConnId)
				return findOp->theUser.m_nConnID;
		}
		return theConnId;
	}
	static BOOL RemoveInstance()
	{
		SortUserDown.clear();
		if (hInstance)
		{
			DeleteCriticalSection(&(hInstance->theCrSec));
			delete hInstance;
			hInstance = NULL;
			return TRUE;
		}
		return FALSE;
	}
};
UStatusHook* UStatusHook::hInstance = NULL;
INT UStatusHook::WorkMode = UStatusHook::NATIVEMODE;
list<UStatusHook::UserUtil> UStatusHook::SortUserDown;

void CEnterpriseControlDlg::DelItemUserReal(CUser User2Rem)
{
	CString csLog;
	auto user = User2Rem;
	bool bDel = false;
	for (int i = 0; i < m_wndReport.GetRecords()->GetCount(); i++)
	{
		CXTPReportRecord  *pRecord = m_wndReport.GetRecords()->GetAt(i);
		CXTPReportRecordItemText *pGroupItem = reinterpret_cast<CXTPReportRecordItemText*>(pRecord->GetItem(0));
		CString csGroupName = pGroupItem->GetValue();
		for (int j = 0; j < pRecord->GetChilds()->GetCount(); j++)
		{
			CXTPReportRecord  *pChildRecord = pRecord->GetChilds()->GetAt(j);
			CXTPReportRecordItemText *pChildItem = reinterpret_cast<CXTPReportRecordItemText*>(pChildRecord->GetItem(0));
			auto pItemNet = reinterpret_cast<CXTPReportRecordItemText*>(pChildRecord->GetItem(1));
			auto pItemLocNet = reinterpret_cast<CXTPReportRecordItemText*>(pChildRecord->GetItem(2));

			if (
				pChildItem->GetItemData() == user.m_nConnID ||
				pChildItem->GetItemData() == UStatusHook::GetMappedConnID(user.m_nConnID) ||
				(pItemNet->GetValue() == CString(user.m_strIPNet.c_str()) &&
					pItemLocNet->GetValue() == CString(user.m_strIPLocal.c_str()))
				)
			{
				pRecord->GetChilds()->RemoveAt(j);
				int nFind = csGroupName.Find(')');
				if (nFind != -1)
				{
					csGroupName = csGroupName.Right(csGroupName.GetLength() - nFind - 1);
				}
				string strGroupName = CStringA(csGroupName);
				CString csGroup;

				if (CMainBusiness::GetInstance()->GetReport().GetGroupUserSize(strGroupName) == 0)
				{
					csGroup.Format(_T("%s"), CString(strGroupName.c_str()));
				}
				else
				{
					csGroup.Format(_T("(%d)%s"),
						CMainBusiness::GetInstance()->GetReport().GetGroupUserSize(strGroupName), CString(strGroupName.c_str()));
				}
				pGroupItem->SetValue(csGroup);
				bDel = true;
				break;
			}
		}
		if (bDel)
		{
			//PopulateReport();
			m_wndReport.Populate();
			break;
		}
	}
	csLog.Format(_T("下线 %s (%s)[%d]"), CString(user.m_strIPNet.c_str()),
		CString(user.m_strRemark.c_str()), user.m_nConnID);
	AddLog(csLog);
	UpdateConnCount();
}

VOID CEnterpriseControlDlg::OnRemoveUser(CUser User2Rem)
{
	CString csLog;
	auto user = User2Rem;
	bool bDel = false;
	for (int i = 0; i < m_wndReport.GetRecords()->GetCount(); i++)
	{
		CXTPReportRecord  *pRecord = m_wndReport.GetRecords()->GetAt(i);
		CXTPReportRecordItemText *pGroupItem = reinterpret_cast<CXTPReportRecordItemText*>(pRecord->GetItem(0));
		CString csGroupName = pGroupItem->GetValue();
		for (int j = 0; j < pRecord->GetChilds()->GetCount(); j++)
		{
			CXTPReportRecord  *pChildRecord = pRecord->GetChilds()->GetAt(j);
			CXTPReportRecordItemText *pChildItem = reinterpret_cast<CXTPReportRecordItemText*>(pChildRecord->GetItem(0));
			if (pChildItem->GetItemData() == user.m_nConnID || pChildItem->GetItemData() == UStatusHook::GetMappedConnID(user.m_nConnID))
			{
				pRecord->GetChilds()->RemoveAt(j);
				int nFind = csGroupName.Find(')');
				if (nFind != -1)
				{
					csGroupName = csGroupName.Right(csGroupName.GetLength() - nFind - 1);
				}
				string strGroupName = CStringA(csGroupName);
				CString csGroup;

				if (CMainBusiness::GetInstance()->GetReport().GetGroupUserSize(strGroupName) == 0)
				{
					csGroup.Format(_T("%s"), CString(strGroupName.c_str()));
				}
				else
				{
					csGroup.Format(_T("(%d)%s"),
						CMainBusiness::GetInstance()->GetReport().GetGroupUserSize(strGroupName), CString(strGroupName.c_str()));
				}
				pGroupItem->SetValue(csGroup);
				bDel = true;
			}
		}
		if (bDel)
		{
			//PopulateReport();
			m_wndReport.Populate();
			break;
		}
	}
	return;
}


// CAboutDlg dialog used for App About
#define BoolType(b) b?true:false
#define ID_COLUMN_SHOW      500

class CAboutDlg : public CDialogSampleDlgBase
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogSampleDlgBase(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogSampleDlgBase::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogSampleDlgBase)
END_MESSAGE_MAP()


// CEnterpriseControlDlg dialog


#define ID_THEME_OFFICE2000             32793
#define ID_THEME_OFFICEXP               32794
#define ID_THEME_OFFICE2003             32795
#define ID_THEME_NATIVEXP               32796
#define ID_THEME_WHIDBEY                32797

CEnterpriseControlDlg::CEnterpriseControlDlg(CWnd* pParent /*=NULL*/)
	: CDialogSampleDlgBase(CEnterpriseControlDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bInRepositionControls = FALSE;
	m_bInitDone = FALSE;
}

void CEnterpriseControlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogSampleDlgBase::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CEnterpriseControlDlg, CDialogSampleDlgBase)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
	ON_MESSAGE(XTPWM_DOCKINGPANE_NOTIFY, OnDockingPaneNotify)
	ON_COMMAND_RANGE(ID_EXIT, ID_BUTTON_SYS_SET, OnMenuAndToolBarCommandRange)
	ON_MESSAGE(WM_UPDATEPORT,OnUpdatePort)
	ON_MESSAGE(WM_UPDATEREPORT,OnUpdateReport)
	ON_MESSAGE(WM_ADDLOGNEW,OnAddLogNew)
	ON_NOTIFY(XTP_NM_REPORT_HEADER_RCLICK, IDC_REPORT, OnReportColumnRClick)
	ON_NOTIFY(XTP_NM_REPORT_SORTORDERCHANGED, IDC_REPORT, OnReportItemClick)
	ON_NOTIFY(NM_RCLICK, IDC_REPORT, OnReportItemRClick)
	ON_NOTIFY(NM_CLICK, IDC_REPORT, OnReportItemClick)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	//ID_INDICATOR_SCRL,
};
// CEnterpriseControlDlg message handlers

BOOL CEnterpriseControlDlg::OnInitDialog()
{	
	SetFlag(xtpResizeNoSizeIcon);//去掉对话框右下角的默认拉伸块
	CDialogSampleDlgBase::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon	
	//初始化界面风格
	CXTPPaintManager::SetTheme(xtpThemeOfficeXP);	
	InitStatusBar();
	IntMenuAndToolBar();	
	InitLog();
	InitReport();			
	m_bInitDone = TRUE;
	MoveWindow(0,0,1024,700);
	RepositionControls();
	CMainBusiness::GetInstance()->Init(m_hWnd);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

LRESULT CEnterpriseControlDlg::OnKickIdle(WPARAM, LPARAM)
{
	static bool bFirst=true;
	if( bFirst )
	{
		bFirst=false;
		m_wndEdit.SetFocus();
	}
	return 0;
}

void CEnterpriseControlDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogSampleDlgBase::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CEnterpriseControlDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogSampleDlgBase::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CEnterpriseControlDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CEnterpriseControlDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialogSampleDlgBase::OnSize(nType, cx, cy);
	RepositionControls();
}

LRESULT CEnterpriseControlDlg::OnDockingPaneNotify(WPARAM wParam, LPARAM lParam)
{
	if (wParam == XTP_DPN_SHOWWINDOW)
	{
		// get a pointer to the docking pane being shown.
		CXTPDockingPane* pPane = (CXTPDockingPane*)lParam;
		if (!pPane->IsValid())
		{
			int nIndex = pPane->GetID() - IDR_PANELOG;
			CreateLogEdit(pPane);
		}
		return TRUE; // handled
	}
	return FALSE;
}

BOOL CEnterpriseControlDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	HD_NOTIFY *pHDNotify = (HD_NOTIFY*)lParam;
	return CDialogSampleDlgBase::OnNotify(wParam, lParam, pResult);
}

void   CEnterpriseControlDlg::InitStatusBar()
{
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return;      // fail to create
	}
	m_wndStatusBar.SetPaneInfo(0, ID_SEPARATOR, SBPS_STRETCH, 800);
	m_wndStatusBar.SetPaneInfo(1, ID_INDICATOR_CAPS, SBPS_STRETCH, 80);
	m_wndStatusBar.SetPaneInfo(2, ID_INDICATOR_NUM, SBPS_STRETCH, 80);

	CString csIPArray;
	char hostname[256]; 
	memset(hostname,0,256);
	gethostname(hostname, sizeof(hostname));
	HOSTENT *host = gethostbyname(hostname);

	if (host != NULL)
	{
		for ( int i=0; ; i++ )
		{ 
			csIPArray += inet_ntoa(*(IN_ADDR*)host->h_addr_list[i]);
			if ( host->h_addr_list[i] + host->h_length >= host->h_name )
				break;
			csIPArray += "/";
		}
	}	
	else
		csIPArray = _T("127.0.0.1");
	m_wndStatusBar.SetPaneText(0,csIPArray);	
	m_wndStatusBar.SetPaneText(2,_T("连接:0"));
}


LRESULT CEnterpriseControlDlg::OnUpdatePort(WPARAM wParam, LPARAM lParam)
{
	CString csData;
	csData.Format(_T("监听:%d"),lParam);
	m_wndStatusBar.SetPaneText(1,csData);
	AddLog(csData);
	return 0;
}

void CEnterpriseControlDlg::UpdateConnCount()
{
	CString csData;
	csData.Format(_T("连接:%d"),CMainBusiness::GetInstance()->GetReport().GetAllGroupUserSize());
	m_wndStatusBar.SetPaneText(2,csData);
}


void   CEnterpriseControlDlg::IntMenuAndToolBar()
{
	VERIFY(InitCommandBars());	
	// TODO: Add extra initialization here
	CXTPCommandBars* pCommandBars = GetCommandBars();
	pCommandBars->SetMenu(_T("Menu Bar"), IDR_MENU);

	CXTPToolBar* pToolBar = pCommandBars->Add(_T("Standard"), xtpBarTop);
	pToolBar->LoadToolBar(IDR_TOOLBAR);
	pToolBar->GetControls()->CreateOriginalControls();

	pCommandBars->GetCommandBarsOptions()->ShowKeyboardCues(xtpKeyboardCuesShowWindowsDefault);
	pCommandBars->GetToolTipContext()->SetStyle(xtpToolTipOffice);

	LoadCommandBars(_T("CommandBars"));
}

void   CEnterpriseControlDlg::OnMenuAndToolBarCommandRange(UINT nID)
{
}

void   CEnterpriseControlDlg::InitLog()
{
	VERIFY(m_paneManager.InstallDockingPanes(this));
	m_paneManager.SetTheme(xtpPaneThemeVisualStudio2005Beta1);
	m_paneManager.UseSplitterTracker(FALSE);
	m_paneManager.SetShowContentsWhileDragging(TRUE);
	m_paneManager.SetAlphaDockingContext(TRUE);

	// Create docking panes.
	CXTPDockingPane* pwndPaneLog = m_paneManager.CreatePane(
		IDR_PANELOG, CRect(0, 0,200, 120), xtpPaneDockBottom );
	pwndPaneLog->SetOptions(xtpPaneNoCloseable);
	LOGFONT lfIcon;
	VERIFY( ::SystemParametersInfo( SPI_GETICONTITLELOGFONT, sizeof( lfIcon ), &lfIcon, 0 ) );
	m_fntEdit.CreateFontIndirect(&lfIcon);

	CXTPDockingPaneLayout layout(&m_paneManager);
	if (layout.Load(_T("NormalLayout"))) 
	{
		m_paneManager.SetLayout(&layout);
	}
}


void  CEnterpriseControlDlg::CreateLogEdit(CXTPDockingPane* pPane)
{
	CEdit& wndEdit = m_wndEdit;
	// create and attach the edit control for this pane.
	if (!::IsWindow(wndEdit.m_hWnd))
	{
		if (!wndEdit.CreateEx(WS_EX_STATICEDGE, _T("EDIT"), _T(""),
			WS_CHILD|WS_VSCROLL|ES_AUTOVSCROLL|ES_MULTILINE|ES_WANTRETURN, CRect(0, 0,0, 0), this, pPane->GetID()))
		{
			TRACE0( "Error creating pane edit control.\n" );
		}
		wndEdit.SetFont(&m_fntEdit);		
	}

	pPane->Attach(&wndEdit);
}

void   CEnterpriseControlDlg::AddLog(CString csData)
{
	CTime t=CTime::GetCurrentTime();
	CString csTime=t.Format("[%Y-%m-%d %H:%M:%S]      ");
	CString csOldText;
	m_wndEdit.GetWindowText(csOldText);
	m_wndEdit.SetSel(csOldText.GetLength(),csOldText.GetLength());  //插入光标放于最后
	m_wndEdit.ReplaceSel(csTime+csData+_T("\r\n"));
	m_wndEdit.ScrollWindow(0,0);  //滚动到插入点
}

BOOL CEnterpriseControlDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if( m_wndEdit.m_hWnd )
	{
		if( m_wndEdit.m_hWnd == pMsg->hwnd && pMsg->message == WM_KEYDOWN )		
		{
			return   TRUE; 
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void   CEnterpriseControlDlg::InitReport()
{
	SortMethod = 1;
	pRecordSort = NULL;
	Sort_Down = 0;
	m_wndReport.Create(WS_CHILD|WS_TABSTOP|WS_VISIBLE|WM_VSCROLL, CRect(0, 0, 0, 0), this, IDC_REPORT );
	m_wndReport.SetGridStyle(TRUE, xtpReportLineStyleSolid);
	m_wndReport.GetPaintManager()->m_treeStructureStyle=xtpReportTreeStructureDots;
	m_wndReport.GetPaintManager()->m_nTreeIndent = 10;

	VERIFY(m_imageList.Create(16,16, ILC_COLOR32|ILC_MASK, 0, 1));
	m_imageList.Add(ExtractIcon(AfxGetInstanceHandle(), TEXT("shell32.dll"), 18));
	m_imageList.Add(ExtractIcon(AfxGetInstanceHandle(), TEXT("shell32.dll"), 15));
	m_wndReport.SetImageList(&m_imageList);

	CXTPReportColumn* Column=m_wndReport.AddColumn(new CXTPReportColumn(0, _T(""), 120));
	Column->SetVisible(FALSE);
	m_wndReport.GetColumns()->GetGroupsOrder()->Add(Column);
	Column=m_wndReport.AddColumn(new CXTPReportColumn(1, _T("分组/ID"), 100));
	Column->SetIconID(0);
	Column=m_wndReport.AddColumn(new CXTPReportColumn(2, _T("外网地址"), 100));
	Column->SetIconID(1);
	Column=m_wndReport.AddColumn(new CXTPReportColumn(3, _T("内网地址"), 100));
	Column->SetIconID(2);
	Column=m_wndReport.AddColumn(new CXTPReportColumn(4, _T("计算机名/备注"), 100));
	Column->SetIconID(3);
	Column=m_wndReport.AddColumn(new CXTPReportColumn(5, _T("操作系统"), 100));
	Column->SetIconID(4);
	Column=m_wndReport.AddColumn(new CXTPReportColumn(6, _T("筛选"), 100));
	Column->SetIconID(5);
	Column=m_wndReport.AddColumn(new CXTPReportColumn(7, _T("延迟"), 100));
	Column->SetIconID(6);
	Column=m_wndReport.AddColumn(new CXTPReportColumn(8, _T("安装时间"), 100));
	Column->SetIconID(7);
	Column=m_wndReport.AddColumn(new CXTPReportColumn(9, _T("物理地址"), 100));
	Column->SetIconID(8);
	Column=m_wndReport.AddColumn(new CXTPReportColumn(10, _T("Cpu/Mhz"), 100));
	Column->SetIconID(9);
	Column=m_wndReport.AddColumn(new CXTPReportColumn(11, _T("空闲"), 50));
	Column->SetIconID(10);
	Column=m_wndReport.AddColumn(new CXTPReportColumn(12, _T("活动时间"), 100));
	Column->SetIconID(11);

	map<string,CGroup>mapGroup;
	CMainBusiness::GetInstance()->GetReport().GetAllGroup(mapGroup);
	map<string,CGroup>::iterator itor=mapGroup.begin();
	for( ; itor != mapGroup.end() ; itor++ )
	{
		CString csGroup;
		csGroup.Format(_T("%s"),CString(itor->first.c_str()));
		CGroup group=itor->second;
		map<unsigned long,CUser>mapUser=group.m_mapUser;
		map<unsigned long,CUser>::iterator itoruser=mapUser.begin();
		for(  ; itoruser !=  mapUser.end() ; itoruser++ )
		{
			CUser user=itoruser->second;
			CXTPReportRecord* Record=m_wndReport.AddRecord(new CXTPReportRecord());	
			Record->AddItem(new CXTPReportRecordItemText(csGroup));
			CString csID;
			csID.Format(_T("%d"),user.m_nConnID);
			CXTPReportRecord *pChild=new CXTPReportRecord;
			CXTPReportRecordItemText* pItem=new CXTPReportRecordItemText(csID);
			pItem->SetIconIndex(1);
			Record->AddItem(pItem);
			Record->AddItem(new CXTPReportRecordItemText(CString(user.m_strIPNet.c_str())));
			Record->AddItem(new CXTPReportRecordItemText(CString(user.m_strIPLocal.c_str())));
			Record->AddItem(new CXTPReportRecordItemText(CString(user.m_strRemark.c_str())));
			Record->AddItem(new CXTPReportRecordItemText(CString(user.m_strSYS.c_str())));
			Record->AddItem(new CXTPReportRecordItemText(CString(user.m_strFilter.c_str())));
			Record->AddItem(new CXTPReportRecordItemText(CString(user.m_strDelay.c_str())));
			Record->AddItem(new CXTPReportRecordItemText(CString(user.m_strSetupTime.c_str())));
			Record->AddItem(new CXTPReportRecordItemText(CString(user.m_strMac.c_str())));
			Record->AddItem(new CXTPReportRecordItemText(CString(user.m_strCpu.c_str())));
			Record->AddItem(new CXTPReportRecordItemText(CString(user.m_strFree.c_str())));
			Record->AddItem(new CXTPReportRecordItemText(CString(user.m_strActTime.c_str())));
		}
		/*CGroup group=itor->second;
		CString csGroup;
		if( group.m_mapUser.size() == 0 )
			csGroup.Format(_T("%s"),CString(itor->first.c_str()));
		else
			csGroup.Format(_T("(%d)%s"),group.m_mapUser.size(),CString(itor->first.c_str()));
		CXTPReportRecord* Record=m_wndReport.AddRecord(new CXTPReportRecord());		
		CXTPReportRecordItemText* pGroupItemText=new CXTPReportRecordItemText(CString(csGroup));
		pGroupItemText->SetIconIndex(0);
		Record->AddItem(pGroupItemText);
		Record->AddItem(new CXTPReportRecordItemText(_T("")));
		Record->AddItem(new CXTPReportRecordItemText(_T("")));
		Record->AddItem(new CXTPReportRecordItemText(_T("")));
		Record->AddItem(new CXTPReportRecordItemText(_T("")));
		Record->AddItem(new CXTPReportRecordItemText(_T("")));
		Record->AddItem(new CXTPReportRecordItemText(_T("")));
		Record->AddItem(new CXTPReportRecordItemText(_T("")));
		Record->AddItem(new CXTPReportRecordItemText(_T("")));
		Record->AddItem(new CXTPReportRecordItemText(_T("")));
		Record->AddItem(new CXTPReportRecordItemText(_T("")));
		Record->AddItem(new CXTPReportRecordItemText(_T("")));
	
		map<unsigned long,CUser>mapUser=group.m_mapUser;
		map<unsigned long,CUser>::iterator itoruser=mapUser.begin();
		for(  ; itoruser !=  mapUser.end() ; itoruser++ )
		{
			ReportGroupAddChild(Record,itoruser->second);
		}*/
	}	
	m_wndReport.Populate();	

	for (int i=0; i<m_wndReport.GetRows()->GetCount(); i++)
	{
		CXTPReportRow *pRow = m_wndReport.GetRows()->GetAt(i);

		if (pRow->IsGroupRow())
		{
			CXTPReportGroupRow *pGroupRow = reinterpret_cast<CXTPReportGroupRow*>(pRow);
			CString csCaption=pGroupRow->GetCaption();
			csCaption.Replace(_T(":"),_T(""));
			CString csNewCaption;
			csNewCaption.Format(_T("(%d)%s"),pRow->GetChilds()->GetCount(),csCaption);			
			pGroupRow->SetCaption(csNewCaption);
		}
	}
}

void    CEnterpriseControlDlg::ReportGroupAddChild(CXTPReportRecord  *pRecord,CUser user)
{
	CString csID;
	csID.Format(_T("%d"),user.m_nConnID);
	CXTPReportRecord *pChild=new CXTPReportRecord;
	CXTPReportRecordItemText* pChildItem=new CXTPReportRecordItemText(csID);
	pChildItem->SetItemData(user.m_nConnID);
	pChildItem->SetIconIndex(1);
	pChild->AddItem(pChildItem);
	pChild->AddItem(new CXTPReportRecordItemText(CString(user.m_strIPNet.c_str())));
	pChild->AddItem(new CXTPReportRecordItemText(CString(user.m_strIPLocal.c_str())));
	pChild->AddItem(new CXTPReportRecordItemText(CString(user.m_strRemark.c_str())));
	pChild->AddItem(new CXTPReportRecordItemText(CString(user.m_strSYS.c_str())));
	pChild->AddItem(new CXTPReportRecordItemText(CString(user.m_strFilter.c_str())));
	pChild->AddItem(new CXTPReportRecordItemText(CString(user.m_strDelay.c_str())));
	pChild->AddItem(new CXTPReportRecordItemText(CString(user.m_strSetupTime.c_str())));
	pChild->AddItem(new CXTPReportRecordItemText(CString(user.m_strMac.c_str())));
	pChild->AddItem(new CXTPReportRecordItemText(CString(user.m_strCpu.c_str())));
	pChild->AddItem(new CXTPReportRecordItemText(CString(user.m_strFree.c_str())));
	pChild->AddItem(new CXTPReportRecordItemText(CString(user.m_strActTime.c_str())));
	pRecord->GetChilds()->Add(pChild);
	pRecordSort = pRecord;
	SortItemGroupEx();
}

class lpItemTextBox
{
public:
	CXTPReportRecordItemText *m_nConnID;
	CXTPReportRecordItemText *m_strIPNet;
	CXTPReportRecordItemText *m_strIPLocal;
	CXTPReportRecordItemText *m_strRemark;
	CXTPReportRecordItemText *m_strSYS;
	CXTPReportRecordItemText *m_strFilter;
	CXTPReportRecordItemText *m_strDelay;
	CXTPReportRecordItemText *m_strSetupTime;
	CXTPReportRecordItemText *m_strMac;
	CXTPReportRecordItemText *m_strCpu;
	CXTPReportRecordItemText *m_strFree;
	CXTPReportRecordItemText *m_strActTime;
};

CXTPReportRecord * GetChiByBuffer (lpItemTextBox toCover)
{
	CXTPReportRecord *pChild = new CXTPReportRecord;
	pChild->AddItem(toCover.m_nConnID);
	pChild->AddItem(toCover.m_strIPNet);
	pChild->AddItem(toCover.m_strIPLocal);
	pChild->AddItem(toCover.m_strRemark);
	pChild->AddItem(toCover.m_strSYS);
	pChild->AddItem(toCover.m_strFilter);
	pChild->AddItem(toCover.m_strDelay);
	pChild->AddItem(toCover.m_strSetupTime);
	pChild->AddItem(toCover.m_strMac);
	pChild->AddItem(toCover.m_strCpu);
	pChild->AddItem(toCover.m_strFree);
	pChild->AddItem(toCover.m_strActTime);
	return pChild;
};

BOOL CEnterpriseControlDlg::SortItemGroup(CXTPReportRecord * pRecord)
{
	list <lpItemTextBox> InfoToStore;
	auto nCount = pRecord->GetChilds()->GetCount();
	if (!nCount)return FALSE;
	auto StoreBuffer = [&](CXTPReportRecord* Rep)
	{
		lpItemTextBox Buffer;
		Buffer.m_nConnID = new CXTPReportRecordItemText;
		Buffer.m_strIPNet = new CXTPReportRecordItemText;
		Buffer.m_strIPLocal = new CXTPReportRecordItemText;
		Buffer.m_strRemark = new CXTPReportRecordItemText;
		Buffer.m_strSYS = new CXTPReportRecordItemText;
		Buffer.m_strFilter = new CXTPReportRecordItemText;
		Buffer.m_strDelay = new CXTPReportRecordItemText;
		Buffer.m_strSetupTime = new CXTPReportRecordItemText;
		Buffer.m_strMac = new CXTPReportRecordItemText;
		Buffer.m_strCpu = new CXTPReportRecordItemText;
		Buffer.m_strFree = new CXTPReportRecordItemText;
		Buffer.m_strActTime = new CXTPReportRecordItemText;

		Buffer.m_nConnID->SetItemData(
			atol((((CXTPReportRecordItemText*)Rep->GetItem(0))->GetValue()).GetString())
		);
		Buffer.m_nConnID->SetIconIndex(1);

		Buffer.m_nConnID->SetValue(((CXTPReportRecordItemText*)Rep->GetItem(0))->GetValue());
		Buffer.m_strIPNet->SetValue(((CXTPReportRecordItemText*)Rep->GetItem(1))->GetValue());
		Buffer.m_strIPLocal->SetValue(((CXTPReportRecordItemText*)Rep->GetItem(2))->GetValue());
		Buffer.m_strRemark->SetValue(((CXTPReportRecordItemText*)Rep->GetItem(3))->GetValue());
		Buffer.m_strSYS->SetValue(((CXTPReportRecordItemText*)Rep->GetItem(4))->GetValue());
		Buffer.m_strFilter->SetValue(((CXTPReportRecordItemText*)Rep->GetItem(5))->GetValue());
		Buffer.m_strDelay->SetValue(((CXTPReportRecordItemText*)Rep->GetItem(6))->GetValue());
		Buffer.m_strSetupTime->SetValue(((CXTPReportRecordItemText*)Rep->GetItem(7))->GetValue());
		Buffer.m_strMac->SetValue(((CXTPReportRecordItemText*)Rep->GetItem(8))->GetValue());
		Buffer.m_strCpu->SetValue(((CXTPReportRecordItemText*)Rep->GetItem(9))->GetValue());
		Buffer.m_strFree->SetValue(((CXTPReportRecordItemText*)Rep->GetItem(10))->GetValue());
		Buffer.m_strActTime->SetValue(((CXTPReportRecordItemText*)Rep->GetItem(11))->GetValue());
		InfoToStore.push_back(Buffer);
	};
	for (auto nAt = 0; nAt < nCount; nAt++)
	{
		auto RecordBuffer = pRecord->GetChilds()->GetAt(nAt);
		StoreBuffer(RecordBuffer);
	}
	auto sort_itor = InfoToStore.begin();
	while (sort_itor != InfoToStore.end())
	{
		auto privIrot = sort_itor++;
		if (sort_itor == InfoToStore.end())break;
		if (sort_itor->m_strSetupTime->GetValue() < privIrot->m_strSetupTime->GetValue())
		{
			lpItemTextBox bufferTextBox = *sort_itor;
			*sort_itor = *privIrot;
			*privIrot = bufferTextBox;
			sort_itor = InfoToStore.begin();
		}
	}
	pRecord->GetChilds()->RemoveAll();
	for (auto nItor = InfoToStore.begin(); nItor != InfoToStore.end(); nItor++)
	{
		pRecord->GetChilds()->Add(GetChiByBuffer(*nItor));
	}
	return 1;
}

BOOL CEnterpriseControlDlg::SortItemGroupEx()
{
	auto pRecord = pRecordSort;
	if (!pRecord)return FALSE;
	list <lpItemTextBox> InfoToStore;
	auto nCount = pRecord->GetChilds()->GetCount();
	if (!nCount)return FALSE;
	auto StoreBuffer = [&](CXTPReportRecord* Rep)
	{
		lpItemTextBox Buffer;
		Buffer.m_nConnID = new CXTPReportRecordItemText;
		Buffer.m_strIPNet = new CXTPReportRecordItemText;
		Buffer.m_strIPLocal = new CXTPReportRecordItemText;
		Buffer.m_strRemark = new CXTPReportRecordItemText;
		Buffer.m_strSYS = new CXTPReportRecordItemText;
		Buffer.m_strFilter = new CXTPReportRecordItemText;
		Buffer.m_strDelay = new CXTPReportRecordItemText;
		Buffer.m_strSetupTime = new CXTPReportRecordItemText;
		Buffer.m_strMac = new CXTPReportRecordItemText;
		Buffer.m_strCpu = new CXTPReportRecordItemText;
		Buffer.m_strFree = new CXTPReportRecordItemText;
		Buffer.m_strActTime = new CXTPReportRecordItemText;

		Buffer.m_nConnID->SetItemData(
			atol((((CXTPReportRecordItemText*)Rep->GetItem(0))->GetValue()).GetString())
		);
		Buffer.m_nConnID->SetIconIndex(1);

		Buffer.m_nConnID->SetValue(((CXTPReportRecordItemText*)Rep->GetItem(0))->GetValue());
		Buffer.m_strIPNet->SetValue(((CXTPReportRecordItemText*)Rep->GetItem(1))->GetValue());
		Buffer.m_strIPLocal->SetValue(((CXTPReportRecordItemText*)Rep->GetItem(2))->GetValue());
		Buffer.m_strRemark->SetValue(((CXTPReportRecordItemText*)Rep->GetItem(3))->GetValue());
		Buffer.m_strSYS->SetValue(((CXTPReportRecordItemText*)Rep->GetItem(4))->GetValue());
		Buffer.m_strFilter->SetValue(((CXTPReportRecordItemText*)Rep->GetItem(5))->GetValue());
		Buffer.m_strDelay->SetValue(((CXTPReportRecordItemText*)Rep->GetItem(6))->GetValue());
		Buffer.m_strSetupTime->SetValue(((CXTPReportRecordItemText*)Rep->GetItem(7))->GetValue());
		Buffer.m_strMac->SetValue(((CXTPReportRecordItemText*)Rep->GetItem(8))->GetValue());
		Buffer.m_strCpu->SetValue(((CXTPReportRecordItemText*)Rep->GetItem(9))->GetValue());
		Buffer.m_strFree->SetValue(((CXTPReportRecordItemText*)Rep->GetItem(10))->GetValue());
		Buffer.m_strActTime->SetValue(((CXTPReportRecordItemText*)Rep->GetItem(11))->GetValue());
		InfoToStore.push_back(Buffer);
	};
	for (auto nAt = 0; nAt < nCount; nAt++)
	{
		auto RecordBuffer = pRecord->GetChilds()->GetAt(nAt);
		StoreBuffer(RecordBuffer);
	}
	auto sort_itor = InfoToStore.begin();
	if(!SortMethod)
		while (sort_itor != InfoToStore.end())
		{
			auto privIrot = sort_itor++;
			if (sort_itor == InfoToStore.end())break;
			if (atol(sort_itor->m_nConnID->GetValue().GetString()) < atol(privIrot->m_nConnID->GetValue().GetString()))
			{
				lpItemTextBox bufferTextBox = *sort_itor;
				*sort_itor = *privIrot;
				*privIrot = bufferTextBox;
				sort_itor = InfoToStore.begin();
			}
		}
	else
		while (sort_itor != InfoToStore.end())
		{
			auto privIrot = sort_itor++;
			auto DetFx = [&]()->BOOL
			{
				auto retRst = [&](BOOL Arg)->BOOL
				{
					if (Sort_Down)
					{
						return Arg;
					}
					else
					{
						if (Arg) return FALSE;
						else return TRUE;
					}
				};
				switch (SortMethod)
				{
				case 1:
					if (sort_itor->m_strIPNet->GetValue() == privIrot->m_strIPNet->GetValue())return FALSE;
					if (sort_itor->m_strIPNet->GetValue() < privIrot->m_strIPNet->GetValue())return retRst(TRUE); else return retRst(FALSE);
				case 2:
					if (sort_itor->m_strIPLocal->GetValue() == privIrot->m_strIPLocal->GetValue())return FALSE;
					if (sort_itor->m_strIPLocal->GetValue() < privIrot->m_strIPLocal->GetValue())return retRst(TRUE); else return retRst(FALSE);
				case 3:
					if (sort_itor->m_strRemark->GetValue() == privIrot->m_strRemark->GetValue())return FALSE;
					if (sort_itor->m_strRemark->GetValue() < privIrot->m_strRemark->GetValue())return retRst(TRUE); else return retRst(FALSE);
				case 4:
					if (sort_itor->m_strSYS->GetValue() == privIrot->m_strSYS->GetValue())return FALSE;
					if (sort_itor->m_strSYS->GetValue() < privIrot->m_strSYS->GetValue())return retRst(TRUE); else return retRst(FALSE);
				case 5:
					if (sort_itor->m_strFilter->GetValue() == privIrot->m_strFilter->GetValue())return FALSE;
					if (sort_itor->m_strFilter->GetValue() < privIrot->m_strFilter->GetValue())return retRst(TRUE); else return retRst(FALSE);
				case 6:
					if (sort_itor->m_strDelay->GetValue() == privIrot->m_strDelay->GetValue())return FALSE;
					if (sort_itor->m_strDelay->GetValue() < privIrot->m_strDelay->GetValue())return retRst(TRUE); else return retRst(FALSE);
				case 7:
					if (sort_itor->m_strSetupTime->GetValue() == privIrot->m_strSetupTime->GetValue())return FALSE;
					if (sort_itor->m_strSetupTime->GetValue() < privIrot->m_strSetupTime->GetValue())return retRst(TRUE); else return retRst(FALSE);
				case 8:
					if (sort_itor->m_strMac->GetValue() == privIrot->m_strMac->GetValue())return FALSE;
					if (sort_itor->m_strMac->GetValue() < privIrot->m_strMac->GetValue())return retRst(TRUE); else return retRst(FALSE);
				case 9:
					if (sort_itor->m_strCpu->GetValue() == privIrot->m_strCpu->GetValue())return FALSE;
					if (sort_itor->m_strCpu->GetValue() < privIrot->m_strCpu->GetValue())return retRst(TRUE); else return retRst(FALSE);
				case 10:
					if (sort_itor->m_strFree->GetValue() == privIrot->m_strFree->GetValue())return FALSE;
					if (sort_itor->m_strFree->GetValue() < privIrot->m_strFree->GetValue())return retRst(TRUE); else return retRst(FALSE);
				default:
					if (sort_itor->m_strActTime->GetValue() == privIrot->m_strActTime->GetValue())return FALSE;
					if (sort_itor->m_strActTime->GetValue() < privIrot->m_strActTime->GetValue())return retRst(TRUE); else return retRst(FALSE);
				}
				return FALSE;
			};
			if (sort_itor == InfoToStore.end())break;
			if (
				DetFx()
				)
			{
				lpItemTextBox bufferTextBox = *sort_itor;
				*sort_itor = *privIrot;
				*privIrot = bufferTextBox;
				sort_itor = InfoToStore.begin();
			}
		}
	pRecord->GetChilds()->RemoveAll();
	for (auto nItor = InfoToStore.begin(); nItor != InfoToStore.end(); nItor++)
	{
		pRecord->GetChilds()->Add(GetChiByBuffer(*nItor));
	}
	return 1;
}

void    CEnterpriseControlDlg::AddItemUser()
{
	CString csLog;
	CUser user=CMainBusiness::GetInstance()->GetReport().GetUserUpdate();	
	auto hookInstance = UStatusHook::GetInstance();
	if (hookInstance->DoHookAddUser(user, this))return;
	string strGroupName=CMainBusiness::GetInstance()->GetReport().GetGroupName(user);
	for (int i=0; i<m_wndReport.GetRecords()->GetCount(); i++)
	{
		CXTPReportRecord  *pRecord  = m_wndReport.GetRecords()->GetAt(i);
		CXTPReportRecordItemText *pGroupItem = reinterpret_cast<CXTPReportRecordItemText*>(pRecord->GetItem(0));
		CString csValue=pGroupItem->GetValue();
		if( csValue.Find(CString(strGroupName.c_str())) != -1 )
		{
			CString csGroup;
			if( CMainBusiness::GetInstance()->GetReport().GetGroupUserSize(strGroupName) == 0 )
			{
				csGroup.Format(_T("%s"),CString(strGroupName.c_str()));
			}
			else
			{
				csGroup.Format(_T("(%d)%s"),
				CMainBusiness::GetInstance()->GetReport().GetGroupUserSize(strGroupName),
					CString(strGroupName.c_str()
				)
				);
			}
			pGroupItem->SetValue(csGroup);
			ReportGroupAddChild(pRecord,user);
			pRecord->SetExpanded(TRUE);
			m_wndReport.Populate();	
			break;
		}
	}
	csLog.Format(_T("上线 %s (%s)[%d]"),CString(user.m_strIPNet.c_str()),
		CString(user.m_strRemark.c_str()),user.m_nConnID);
	AddLog(csLog);
	UpdateConnCount();
}

void    CEnterpriseControlDlg::DelItemUser()
{
	CUser user=CMainBusiness::GetInstance()->GetReport().GetUserUpdate();
	auto hookInstance = UStatusHook::GetInstance();
	hookInstance->DoHookRemUser(user, this);
	return;
}

LRESULT CEnterpriseControlDlg::OnUpdateReport(WPARAM wParam, LPARAM lParam)
{	
	int nStatu=wParam;

	if( nStatu == STATU_ADD )
	{		
		AddItemUser();
	}
	else if(nStatu == STATU_DEL )
	{
		DelItemUser();
	}
	else if( nStatu == STATU_UPDATE )
	{
	}	
	return 0;
}

void CEnterpriseControlDlg::OnReportColumnRClick(NMHDR * pNotifyStruct, LRESULT * /*result*/)
{
	XTP_NM_REPORTRECORDITEM* pItemNotify = (XTP_NM_REPORTRECORDITEM*) pNotifyStruct;
	ASSERT(pItemNotify->pColumn);
	CPoint ptClick = pItemNotify->pt;
	
	CXTPReportColumns* pColumns = m_wndReport.GetColumns();
	CXTPReportColumn* pColumn = pItemNotify->pColumn;
	int nColumnCount = pColumns->GetCount();
	int nColumn;
	CMenu menuColumns;
	VERIFY(menuColumns.CreatePopupMenu());
	for (nColumn = 0; nColumn < nColumnCount; nColumn++)
	{
		CXTPReportColumn* pCol = pColumns->GetAt(nColumn);
		CString sCaption = pCol->GetCaption();
		menuColumns.AppendMenu(MF_STRING, ID_COLUMN_SHOW + nColumn, sCaption);
		menuColumns.CheckMenuItem(ID_COLUMN_SHOW + nColumn,MF_BYCOMMAND | (pCol->IsVisible() ? MF_CHECKED : MF_UNCHECKED) );
	}
	int nMenuResult = CXTPCommandBars::TrackPopupMenu(&menuColumns, TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTALIGN |TPM_RIGHTBUTTON, ptClick.x, ptClick.y, this, NULL);
	if (nMenuResult >= ID_COLUMN_SHOW)
	{
		CXTPReportColumn* pCol = pColumns->GetAt(nMenuResult - ID_COLUMN_SHOW);
		if (pCol)
		{
			pCol->SetVisible(!pCol->IsVisible());
		}
	}
}

void CEnterpriseControlDlg::OnReportItemClick(NMHDR * pNotifyStruct, LRESULT * /*result*/)
{
	
	XTP_NM_REPORTRECORDITEM* pItemNotify = (XTP_NM_REPORTRECORDITEM*)pNotifyStruct;
	sortdlg sdlg;
	if (Sort_Down)Sort_Down = FALSE;
	else Sort_Down = TRUE;
	sdlg.SortMethod = &SortMethod;
	sdlg.DoModal();
	for (int i = 0; i < m_wndReport.GetRecords()->GetCount(); i++)
	{
		CXTPReportRecord  *pRecord = m_wndReport.GetRecords()->GetAt(i);
		pRecordSort = pRecord;
		SortItemGroupEx();
		pRecord->SetExpanded(TRUE);
	}
	m_wndReport.Populate();
	// PopulateReport();
}

void CEnterpriseControlDlg::OnReportItemRClick(NMHDR * pNotifyStruct, LRESULT * /*result*/)
{
	XTP_NM_REPORTRECORDITEM* pItemNotify = (XTP_NM_REPORTRECORDITEM*) pNotifyStruct;

	if (!pItemNotify->pRow)
		return;

	/*CMenu menu;
	if (pItemNotify->pRow->IsGroupRow())
	{
		VERIFY(menu.LoadMenu(IDR_MENU_CONTEXT_GROUP));
	}
	else
	{
		VERIFY(menu.LoadMenu(IDR_MENU_CONTEXT_ROW));
	}

	// track menu
	int nMenuResult = CXTPCommandBars::TrackPopupMenu(menu.GetSubMenu(0), TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTALIGN |TPM_RIGHTBUTTON, pItemNotify->pt.x, pItemNotify->pt.y, this, NULL);

	// general items processing
	switch (nMenuResult)
	{
	case ID_POPUP_COLLAPSEALLGROUPS:
	{
		pItemNotify->pRow->GetControl()->CollapseAll();
		break;
	}
	case ID_POPUP_EXPANDALLGROUPS:
	{
		pItemNotify->pRow->GetControl()->ExpandAll();
		break;
	}

	case ID_POPUP_MARK_READ:
	{
		CMessageRecord *pRecord = DYNAMIC_DOWNCAST(CMessageRecord, pItemNotify->pRow->GetRecord());
		if (pRecord->SetRead(TRUE))
		{
			GetReportCtrl().Populate();
		}
		break;
	}

	case ID_POPUP_MARK_UNREAD:
	{
		CMessageRecord *pRecord = DYNAMIC_DOWNCAST(CMessageRecord, pItemNotify->pRow->GetRecord());
		if (pRecord->SetRead(FALSE))
		{
			GetReportCtrl().Populate();
		}
		break;
	}

	}*/
}


void CEnterpriseControlDlg::RepositionControls()
{
	if (m_bInRepositionControls || !m_bInitDone)
		return;
	CRect rcClientStart;
	CRect rcClientNow;
	GetClientRect(rcClientStart);
	if ((GetStyle() & WS_MINIMIZE) || (rcClientStart.IsRectEmpty()))
		return;
	m_bInRepositionControls = TRUE;		
	//切分窗口
	RepositionBars(0, 0xffff, AFX_IDW_PANE_FIRST, 0, 0, &rcClientStart);
	RepositionBars(0, 0xffff, AFX_IDW_PANE_FIRST, reposQuery, &rcClientStart, &rcClientStart);	
	//状态栏
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0, reposQuery, rcClientNow);				
	m_wndReport.MoveWindow(rcClientStart);
	m_bInRepositionControls = FALSE;
}

LRESULT CEnterpriseControlDlg::OnAddLogNew(WPARAM wParam, LPARAM lParam)
{	
	char* cData=(char*)lParam;
	AddLog(CString(cData));
	delete []cData;
	return 0;
}

void CEnterpriseControlDlg::OnClose()
{
	UStatusHook::RemoveInstance();
	CMainBusiness::GetInstance()->SetExit();
	CMainBusiness::GetInstance()->WaitAllThreadExit();
	CDialogSampleDlgBase::OnClose();
}