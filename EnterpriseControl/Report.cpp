#include "StdAfx.h"
#include "Report.h"


CReport::CReport(void)
{
	InitializeCriticalSection(&m_crit_User);
	InitializeCriticalSection(&m_cs_CloseUser);
	CGroup group;
	CUser user(1,"123.45.34.1","127.0.0.1");
	group.m_mapUser.insert(pair<unsigned long,CUser>(user.m_nConnID,user));
	CUser user1(2,"223.45.34.1","227.0.0.1");
	group.m_mapUser.insert(pair<unsigned long,CUser>(user1.m_nConnID,user1));
	m_mapGroup.insert(pair<string,CGroup>("默认分组",group));

	m_mapGroup.insert(pair<string,CGroup>("好友",group));
}


CReport::~CReport(void)
{
}


void  CReport::GroupAddUser(const string strGroupName,CUser user)
{
	EnterCriticalSection(&m_crit_User);
	map<string,CGroup>::iterator itor=m_mapGroup.find(strGroupName);
	if( itor == m_mapGroup.end() )
	{
		itor=m_mapGroup.find("默认分组");		
	}
	itor->second.m_mapUser.insert(pair<unsigned long,CUser>(user.m_nConnID,user));	
	LeaveCriticalSection(&m_crit_User);
}

CUser  CReport::GroupDelUser(unsigned long dwConnID)
{
	EnterCriticalSection(&m_crit_User);
	CUser user;
	map<string,CGroup>::iterator itor=m_mapGroup.begin();
	for( ; itor != m_mapGroup.end() ;itor++ )
	{
		map<unsigned long,CUser>::iterator itoruser=itor->second.m_mapUser.find(dwConnID);
		if( itoruser != itor->second.m_mapUser.end() )
		{
			user=itoruser->second;
			itor->second.m_mapUser.erase(itoruser);
			break;
		}
	}
	LeaveCriticalSection(&m_crit_User);
	return user;
}

void CReport::GetAllGroup(map<string,CGroup>&mapAllGroup)
{
	EnterCriticalSection(&m_crit_User);
	mapAllGroup=m_mapGroup;
	LeaveCriticalSection(&m_crit_User);
}


void  CReport::AddCloseUser(const unsigned long nConnID)
{		
	EnterCriticalSection(&m_cs_CloseUser);	
	m_listCloseUser.push_back(nConnID);	
	LeaveCriticalSection(&m_cs_CloseUser);
}

void  CReport::DelCloseUser(const unsigned long nConnID)
{
	EnterCriticalSection(&m_cs_CloseUser);	
	list<int>::iterator itor=m_listCloseUser.begin();
	for( ; itor != m_listCloseUser.end() ; itor++ )
	{
		if( nConnID == (*itor) )
		{
			m_listCloseUser.erase(itor);
			break;
		}
	}
	LeaveCriticalSection(&m_cs_CloseUser);
}

void  CReport::GetCloseUser(list<int>&listCloseUser)
{
	EnterCriticalSection(&m_cs_CloseUser);
	listCloseUser=m_listCloseUser;
	LeaveCriticalSection(&m_cs_CloseUser);
}


string  CReport::GetGroupName(CUser user)
{
	string strName;
	map<string,CGroup>mapGroup;
	GetAllGroup(mapGroup);
	for( map<string,CGroup>::iterator itor=mapGroup.begin(); itor != mapGroup.end() ; itor++ )
	{
		CGroup group=itor->second;
		map<unsigned long,CUser>::iterator itoruser=group.m_mapUser.find(user.m_nConnID);
		if(  itoruser !=  group.m_mapUser.end())
		{
			strName=itor->first;
			break;
		}
	}
	return strName;
}

int  CReport::GetGroupUserSize(string strName)
{
	map<string,CGroup>mapGroup;
	GetAllGroup(mapGroup);
	map<string,CGroup>::iterator itor=mapGroup.find(strName);
	if( itor != mapGroup.end() )
	{
		return itor->second.m_mapUser.size();
	}
	return 0;
}

int    CReport::GetAllGroupUserSize()
{
	int nCount=0;
	map<string,CGroup>mapGroup;
	GetAllGroup(mapGroup);
	map<string,CGroup>::iterator itor=mapGroup.begin();
	for( ; itor != mapGroup.end() ; itor++ )
	{
		nCount += itor->second.m_mapUser.size();
	}
	return nCount;
}

void   CReport::SetUserUpdate(CUser user)
{
	m_UserUpdate=user;
}

CUser CReport::GetUserUpdate()
{
	return m_UserUpdate;
}

void   CReport::SetCurSelUser(unsigned long dwConnID)
{
	map<string,CGroup>mapAllGroup;
	GetAllGroup(mapAllGroup);
	map<string,CGroup>::iterator itor=mapAllGroup.begin();
	for( ; itor != mapAllGroup.end() ;itor++ )
	{
		map<unsigned long,CUser>::iterator itoruser=itor->second.m_mapUser.find(dwConnID);
		if( itoruser != itor->second.m_mapUser.end() )
		{
			m_curSelUser=itoruser->second;
		}
	}
}

CUser CReport::GetCurSelUser()
{
	return m_curSelUser;
}