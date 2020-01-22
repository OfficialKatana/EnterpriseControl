#pragma once
#include "Report.h"
#include "Server.h"
//主业务
class CMainBusiness
{
private:  
	CMainBusiness();
	virtual ~CMainBusiness(void);
	static CMainBusiness *m_pInstance;  
public:  
	static CMainBusiness * GetInstance()  
	{  
		if(m_pInstance == NULL)  //判断是否第一次调用  
		{
			m_pInstance = new CMainBusiness(); 
		}
		return m_pInstance;  
	} 
public:
	void                  Init(HWND hwnd);
	void                  SetPort(unsigned short nPort);//设置端口，并开始监听
	void                  DisposeBusiness(ITcpServer* pSender, CONNID dwConnID, const BYTE* pRecvData);//处理业务	
	void                  Close(CONNID dwConnID);	//关闭用户
	void                  AddThread();//添加一条线程数
	void                  AddExitThread();//添加一条退出线程数	
	bool                  GetExit();//获取是否退出
	void                  SetExit();//设置退出
	bool                  WaitAllThreadExit();//等待所有线程退出	
	CReport               GetReport();//获取列表
private:
	CReport               m_Report;   //列表
	CServer               m_Server;   //通信
	HWND                  m_hWnd;
	unsigned short        m_nPort;
	bool                  m_bExit;
	int                   m_nThreadTotal;//线程总线
	int                   m_nExitThreadTotal;//退出线程总线	
private:
	void                  Login(CONNID dwConnID,ITcpServer* pSender,const BYTE* pData);//登陆
	string                GetUserSYS(void*	pLoginInfo);	
	static DWORD WINAPI   CloseUserThread(LPVOID lpParam);//用户退出线程
};

