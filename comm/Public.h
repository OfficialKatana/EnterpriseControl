#pragma once
//公共功能
#include <fstream>
#include <string>
#include<queue>
using namespace std;

extern   string g_strIP;
extern   int    g_nPort;

#define  SOFTCODE    "DianYinMusicPC"
//封装发送的数据
extern void EncapsulationSendData(string &strSendData,int ndataheadlen); 

//解析逗号格式的字符串:,,,,,,成每个元素
extern void ParseCommaString(string strdata, vector<string>&vecstrdata,string strSeparator=",");

//解析vector,成逗号格式的字符串：,,,,,,
extern void ParseVectorToString(const vector<string>&vecstrdata,string& strdata,string strSeparator=",");

//将数据组合数组 解析内容格式:,,,,,,  解析后的内容格式:[",,,,",",,,,",",,,,"]
extern  string CombinationArray(vector<string>vecData);

//使用在数据库插入数据时,解析内容格式:,,,,,,  解析后的内容格式:'','',''
extern string CombinationInsertData(string strdata);

//汉字成utf-8
extern string GBKToUTF8(const std::string & strGBK);
//utf-8成汉字
extern string UTF8ToGBK(const std::string & strUTF8);
extern wstring UTF8ToGBKW(const std::string & strUTF8);
//获取文件内容
extern bool ReadFileData(const char* cFilePath,string& strdata);
//获取文件内容
extern long ReadFileData(const char* cFilePath,char *&pdata);
//string to wstring
extern std::wstring StringToWString(const std::string &str);
//wstring to string
extern std::string WStringToString(const std::wstring &wstr);
//带中文的路径wstring to string
extern string ws2s(const std::wstring& ws);
extern std::string WStringToString(const std::wstring &wstr);
//带中文的路径string to wstring
extern std::wstring s2ws(const std::string& s);
//Utf8转string
extern void Utf8ToString(string strUtf8, string & strAnsi);
//Utf8转wstring
extern void Utf8ToWString(wstring& wstr,const char* pUTF8Text);
//wstring转Utf8
extern char* WStringToUtf8(const wstring& wstr);
//unsigned char转wchar_t
extern wchar_t * ucTowc(const unsigned char * pUC);
//string替换
extern string replace(const char *pszSrc, const char *pszOld, const char *pszNew);