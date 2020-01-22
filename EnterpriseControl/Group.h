#pragma once
//组
#include "User.h"
#include<map>
class CGroup
{
public:
	CGroup(void);
	~CGroup(void);
public:
	map<unsigned long,CUser>m_mapUser;//key为id也为通信的connid
};

