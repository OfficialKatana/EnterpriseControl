#pragma once
//用户信息
#include<string>
using namespace std;
class CUser
{
public:
	CUser(void);
	CUser(unsigned long nConnID,string strIPNet="",string strIPLocal="",string strRemark="",
		string strSYS="",string strFilter="",string strDelay="",string strSetupTime="",
		string strMac="",string strCpu="",string strFree="",string strActTime="");
	~CUser(void);
public:	
	unsigned long m_nConnID;
	string        m_strIPNet;//外网地址
	string        m_strIPLocal;//本地地址
	string        m_strRemark;//计算机名/备注
	string        m_strSYS;//操作系统
	string        m_strFilter;//筛选
	string        m_strDelay;//延迟
	string        m_strSetupTime;//安装时间
	string        m_strMac;//物理地址
	string        m_strCpu;//Cpu/Mhz
	string        m_strFree;//空闲
	string        m_strActTime;//活动时间
};

