// sortdlg.cpp: 实现文件
//

#include "stdafx.h"
#include "sortdlg.h"
#include "afxdialogex.h"
#include "resource.h"

#define ID_MYBUT1 200
#define ID_MYBUT2 201
#define ID_MYBUT3 202
#define ID_MYBUT4 203
#define ID_MYBUT5 204
#define ID_MYBUT6 205
#define ID_MYBUT7 206
#define ID_MYBUT8 207
#define ID_MYBUT9 208
#define ID_MYBUT10 209
#define ID_MYBUT11 210
#define ID_MYBUT12 211

// sortdlg 对话框

IMPLEMENT_DYNAMIC(sortdlg, CDialogEx)

sortdlg::sortdlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG1, pParent)
{
	SortMethod = NULL;
}

sortdlg::~sortdlg()
{
}


BEGIN_MESSAGE_MAP(sortdlg, CDialogEx)
	ON_BN_CLICKED(ID_MYBUT1, OnMybut1)
	ON_BN_CLICKED(ID_MYBUT2, OnMybut2)
	ON_BN_CLICKED(ID_MYBUT3, OnMybut3)
	ON_BN_CLICKED(ID_MYBUT4, OnMybut4)
	ON_BN_CLICKED(ID_MYBUT5, OnMybut5)
	ON_BN_CLICKED(ID_MYBUT6, OnMybut6)
	ON_BN_CLICKED(ID_MYBUT7, OnMybut7)
	ON_BN_CLICKED(ID_MYBUT8, OnMybut8)
	ON_BN_CLICKED(ID_MYBUT9, OnMybut9)
	ON_BN_CLICKED(ID_MYBUT10, OnMybut10)
	ON_BN_CLICKED(ID_MYBUT11, OnMybut11)
	ON_BN_CLICKED(ID_MYBUT12, OnMybut12)
END_MESSAGE_MAP()


void sortdlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BOOL sortdlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	p_MyBut[0] = NewMyButton(_TEXT("分组/ID"), ID_MYBUT1, CRect(10, 0, 110, 100), 0);
	p_MyBut[1] = NewMyButton(_TEXT("外网地址"), ID_MYBUT2, CRect(10, 100, 110, 200), 0);
	p_MyBut[2] = NewMyButton(_TEXT("内网地址"), ID_MYBUT3, CRect(10, 200, 110, 300), 0);
	p_MyBut[3] = NewMyButton(_TEXT("计算机名"), ID_MYBUT4, CRect(150, 0, 250, 100), 0);
	p_MyBut[4] = NewMyButton(_TEXT("操作系统"), ID_MYBUT5, CRect(150, 100, 250, 200), 0);
	p_MyBut[5] = NewMyButton(_TEXT("筛选"), ID_MYBUT6, CRect(150, 200, 250, 300), 0);
	p_MyBut[6] = NewMyButton(_TEXT("延迟"), ID_MYBUT7, CRect(300, 0, 400, 100), 0);
	p_MyBut[7] = NewMyButton(_TEXT("安装时间"), ID_MYBUT8, CRect(300, 100, 400, 200), 0);
	p_MyBut[8] = NewMyButton(_TEXT("物理地址"), ID_MYBUT9, CRect(300, 200, 400, 300), 0);
	p_MyBut[9] = NewMyButton(_TEXT("Cpu/Mhz"), ID_MYBUT10, CRect(450, 0, 550, 100), 0);
	p_MyBut[10] = NewMyButton(_TEXT("空闲"), ID_MYBUT11, CRect(450, 100, 550, 200), 0);
	p_MyBut[11] = NewMyButton(_TEXT("活动时间"), ID_MYBUT12, CRect(450, 200, 550, 300), 0);

	return TRUE;
}

CButton* sortdlg::NewMyButton(CString cstr, int nID, CRect rect, int nStyle)
{
	CButton *p_Button = new CButton();
	ASSERT_VALID(p_Button);
	p_Button->Create(cstr, WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | nStyle, rect, this, nID); //创建按钮
	return p_Button;
}

void sortdlg::OnMybut1()
{
	if (!SortMethod)return;
	*SortMethod = 0;
}

void sortdlg::OnMybut2()
{
	if (!SortMethod)return;
	*SortMethod = 1;
}

void sortdlg::OnMybut3()
{
	if (!SortMethod)return;
	*SortMethod = 2;
}

void sortdlg::OnMybut4()
{
	if (!SortMethod)return;
	*SortMethod = 3;
}

void sortdlg::OnMybut5()
{
	if (!SortMethod)return;
	*SortMethod = 4;
}

void sortdlg::OnMybut6()
{
	if (!SortMethod)return;
	*SortMethod = 5;
}

void sortdlg::OnMybut7()
{
	if (!SortMethod)return;
	*SortMethod = 6;
}

void sortdlg::OnMybut8()
{
	if (!SortMethod)return;
	*SortMethod = 7;
}

void sortdlg::OnMybut9()
{
	if (!SortMethod)return;
	*SortMethod = 8;
}

void sortdlg::OnMybut10()
{
	if (!SortMethod)return;
	*SortMethod = 9;
}

void sortdlg::OnMybut11()
{
	if (!SortMethod)return;
	*SortMethod = 10;
}

void sortdlg::OnMybut12()
{
	if (!SortMethod)return;
	*SortMethod = 11;
}




// sortdlg 消息处理程序
