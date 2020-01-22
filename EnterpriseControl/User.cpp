#include "StdAfx.h"
#include "User.h"


CUser::CUser(void)
{
}


CUser::~CUser(void)
{
}

CUser::CUser(unsigned long nConnID,string strIPNet,string strIPLocal,string strRemark,
		string strSYS,string strFilter,string strDelay,string strSetupTime,
		string strMac,string strCpu,string strFree,string strActTime)
{
	m_nConnID=nConnID;
	m_strIPNet=strIPNet;//外网地址
	m_strIPLocal=strIPLocal;//本地地址
	m_strRemark=strRemark;//计算机名/备注
	m_strSYS=strSYS;//操作系统
	m_strFilter=strFilter;//筛选
	m_strDelay=strDelay;//延迟
	m_strSetupTime=strSetupTime;//安装时间
	m_strMac=strMac;//物理地址
	m_strCpu=strCpu;//Cpu/Mhz
	m_strFree=strFree;//空闲
	m_strActTime=strActTime;//活动时间
}