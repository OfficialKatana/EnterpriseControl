#pragma once
//列表
#include "Group.h"
#include<list>
class CReport
{
public:
	CReport(void);
	~CReport(void);
private:
	CRITICAL_SECTION  m_crit_User;
	map<string,CGroup>m_mapGroup;//key为组名
	CRITICAL_SECTION  m_cs_CloseUser;
	list<int>         m_listCloseUser;//关闭用户
	CUser             m_UserUpdate;//当前添加/删除/修改的用户
	CUser             m_curSelUser;	//当前选择的用户
public:
	string          GetGroupName(CUser user);//获取组名
	int             GetGroupUserSize(string strName);//获取组用户大小
	int             GetAllGroupUserSize();//获取所有组用户大小
	void            GroupAddUser(const string strGroupName,CUser user);//组添加用户
	CUser           GroupDelUser(unsigned long dwConnID);//组删除用户	
	void            GetAllGroup(map<string,CGroup>&mapAllGroup);//获取所有组数据
	void            AddCloseUser(const unsigned long nConnID);//添加关闭的用户
	void            GetCloseUser(list<int>&listCloseUser);//获取关闭的用户
	void            DelCloseUser(const unsigned long nConnID);//删除关闭的用户
    void            SetUserUpdate(CUser user);//设置当前添加/删除/修改的用户
	CUser           GetUserUpdate();//获取当前添加/删除/修改的用户
	void            SetCurSelUser(unsigned long dwConnID);//设置当前选择的用户
	CUser           GetCurSelUser();//获取当前选择的用户	
};

