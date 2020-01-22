#include "Public.h"
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <sstream>
#include <windows.h>
#include <fstream>

string g_strIP;
int    g_nPort;

void EncapsulationSendData(string &strSendData,int ndataheadlen)
{	
	//在数据包前面填充8位包长度，不够8位补0
	unsigned long datelen=strSendData.size()+ndataheadlen;
	std::stringstream streamDatalen;
	streamDatalen<<datelen;
	string strDatalen=streamDatalen.str();
	if( strDatalen.size() < ndataheadlen)
	{
		string strZero;
		for(int i = 0 ; i < (ndataheadlen-strDatalen.size()) ;i++)
		{
			strZero+="0";
		}
		strDatalen=strZero+strDatalen;
	}
	strSendData=strDatalen+strSendData;
}

string CombinationArray(vector<string>vecData)
{
	string strdata="[";
	int nsize=vecData.size();	
	for( int i = 0 ; i < nsize;i++)
	{
		strdata += "\""+vecData.at(i)+"\"";
		if( i < nsize-1 )
			strdata += ",";
	}
	strdata += "]";
	return strdata;
}

string  CombinationInsertData(string strdata)
{
	vector<string>vecData;
	ParseCommaString(strdata,vecData);
	string strInsertdata;
	int nsize=vecData.size();
	for(int i = 0 ; i < nsize ; i++ )
	{
		strInsertdata += "'";
		strInsertdata+=vecData.at(i);
		strInsertdata += "'";
		if( i < nsize -1 )
			strInsertdata += ",";
	}
	return strInsertdata;
}

void ParseCommaString(string strdata,vector<string>&vecstrdata,string strSeparator)
{
	string strinfo=strdata;			
	int nfind=strinfo.find(strSeparator);
	if( nfind == -1 )
	{		
		if (strinfo.size() > 0 )
			vecstrdata.push_back(strinfo);
		return;
	}
	while( nfind != -1)
	{
		string strvalue=strinfo.substr(0,nfind);
		vecstrdata.push_back(strvalue);			
		strinfo=strinfo.substr(nfind+1,strinfo.size()-nfind-1);
		nfind=strinfo.find(strSeparator);
		if( nfind == -1 )
		{	
			vecstrdata.push_back(strinfo);					
		}
	}	
}

void ParseVectorToString(const vector<string>&vecstrdata,string& strdata,string strSeparator)
{
	for( int i = 0 ; i < vecstrdata.size() ; i++ )
	{
		strdata += vecstrdata.at(i);
		if( i < (vecstrdata.size()-1) )
			strdata += strSeparator;
	}
}

string GBKToUTF8(const std::string & strGBK)
{
	string strOutUTF8 = "";
	WCHAR * str1;
	int n = MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, NULL, 0);
	str1 = new WCHAR[n];
	MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, str1, n);
	n = WideCharToMultiByte(CP_UTF8, 0, str1, -1, NULL, 0, NULL, NULL);
	char * str2 = new char[n];
	WideCharToMultiByte(CP_UTF8, 0, str1, -1, str2, n, NULL, NULL);
	strOutUTF8 = str2;
	delete[]str1;
	str1 = NULL;
	delete[]str2;
	str2 = NULL;
	return strOutUTF8;
}

string UTF8ToGBK(const std::string & strUTF8)
{
	string strOutGBK = "";
	WCHAR * str1;
	int n = MultiByteToWideChar(CP_UTF8, 0, strUTF8.c_str(), -1, NULL, 0);
	str1 = new WCHAR[n];
	MultiByteToWideChar(CP_UTF8, 0, strUTF8.c_str(), -1, str1, n);
	n = WideCharToMultiByte(CP_ACP, 0, str1, -1, NULL, 0, NULL, NULL);
	char * str2 = new char[n];
	WideCharToMultiByte(CP_ACP, 0, str1, -1, str2, n, NULL, NULL);
	strOutGBK = str2;
	delete[]str1;
	str1 = NULL;
	delete[]str2;
	str2 = NULL;
	return strOutGBK;
}

wstring UTF8ToGBKW(const std::string & strUTF8)
{
	wstring wstrOutGBK = L"";
	WCHAR * str1;
	int n = MultiByteToWideChar(CP_UTF8, 0, strUTF8.c_str(), -1, NULL, 0);
	str1 = new WCHAR[n];
	MultiByteToWideChar(CP_UTF8, 0, strUTF8.c_str(), -1, str1, n);
	n = WideCharToMultiByte(CP_ACP, 0, str1, -1, NULL, 0, NULL, NULL);	
	wstrOutGBK = str1;
	delete[]str1;
	str1 = NULL;
	return wstrOutGBK;
}

int hexCharToInt(char c)  
{   
	if (c >= '0' && c <= '9') return (c - '0');  
	if (c >= 'A' && c <= 'F') return (c - 'A' + 10);  
	if (c >= 'a' && c <= 'f') return (c - 'a' + 10);  
	return 0;  
} 

bool ReadFileData(const char* cFilePath,string& strdata)
{
	filebuf *pbuf=NULL;  
	ifstream filestr;  
	long size=0;  	   
	filestr.open (cFilePath, ios::binary);
	if( filestr.is_open() )
	{
		//获取文件大小  
		pbuf=filestr.rdbuf();  	
		size=pbuf->pubseekoff (0,ios::end,ios::in);  
		pbuf->pubseekpos (0,ios::in);  
		// 获取文件内容  
		size+=1;
		char * pdata=new char[size];  
		memset(pdata,0,size);	
		streamsize result=pbuf->sgetn (pdata,size); 
		filestr.close();	
		strdata=string(pdata);		
		//utf-8 txt格式头0xef,0xbb,0xbf
		char cHead1=pdata[0];
		char cHead2=pdata[1];
		char cHead3=pdata[2];
		char cHeadUtf81=0xef;
		char cHeadUtf82=0xbb;
		char cHeadUtf83=0xbf;
		if( size >= 3 &&  cHead1 == cHeadUtf81 && cHead2 == cHeadUtf82 && cHead3 == cHeadUtf83 ) 
		{
			string strUtf8=strdata;
			strdata=strUtf8.substr(3,strUtf8.size()-3);	
		}		
		delete []pdata; 
		return true;
	}
	return false;
}

long ReadFileData(const char* cFilePath,char *&pdata)
{
	filebuf *pbuf=NULL;  
	ifstream filestr;  
	long lsize=0;  	   
	filestr.open (cFilePath, ios::binary);  
	if( filestr.is_open() )
	{		 
		pbuf=filestr.rdbuf();  
		lsize=pbuf->pubseekoff (0,ios::end,ios::in);  
		pbuf->pubseekpos (0,ios::in); 
		pdata=new char[lsize];  
		memset(pdata,0,lsize);
		streamsize result=pbuf->sgetn (pdata,lsize); 
		filestr.close();
	}
	return lsize;
}

std::wstring StringToWString(const std::string &str)
{
	std::wstring wstr(str.length(),L' ');
	std::copy(str.begin(), str.end(), wstr.begin());
	return wstr; 
}

std::string WStringToString(const std::wstring &wstr)
{
	std::string str(wstr.length(), ' ');
	std::copy(wstr.begin(), wstr.end(), str.begin());
	return str; 
}

string ws2s(const std::wstring& ws)
{
	std::string curLocale = setlocale(LC_ALL, NULL);        // curLocale = "C";
	setlocale(LC_ALL, "chs");
	const wchar_t* _Source = ws.c_str();
	size_t _Dsize = 2 * ws.size() + 1;
	char *_Dest = new char[_Dsize];
	memset(_Dest,0,_Dsize);
	wcstombs(_Dest,_Source,_Dsize);
	std::string result = _Dest;
	delete []_Dest;
	setlocale(LC_ALL, curLocale.c_str());
	return result;
}

std::wstring s2ws(const std::string& s)
{
	setlocale(LC_ALL, "chs");
	const char* _Source = s.c_str();
	size_t _Dsize = s.size() + 1;
	wchar_t *_Dest = new wchar_t[_Dsize];
	wmemset(_Dest, 0, _Dsize);
	mbstowcs(_Dest,_Source,_Dsize);
	std::wstring result = _Dest;
	delete []_Dest;
	setlocale(LC_ALL, "C");
	return result;
}

void Utf8ToString(string strUtf8, string & strAnsi)
{
	wstring wstr;
	int len = MultiByteToWideChar(CP_UTF8,0,strUtf8.c_str(),strUtf8.size(),NULL,0);
	wstr.resize(len);
	MultiByteToWideChar(CP_UTF8,0,strUtf8.c_str(),strUtf8.size(),&wstr[0],len);
	len=WideCharToMultiByte(CP_ACP,0,wstr.c_str(),wstr.size(),NULL,0,NULL,NULL);	
	strAnsi.resize(len);
	WideCharToMultiByte(CP_ACP,0,wstr.c_str(),wstr.size(),&strAnsi[0],len,NULL,NULL);
}

void Utf8ToWString(wstring& wstr,const char* pUTF8Text)  
{  
	if (NULL == pUTF8Text)  
	{  
		return;  
	}  
	int  unicodeLen = ::MultiByteToWideChar( CP_UTF8,  0,  pUTF8Text,  -1,  NULL,  0 ); 

	wchar_t*  pUnicode = new  wchar_t[unicodeLen+1];  
	if (NULL == pUnicode)  
	{  
		return;  
	}  
	MultiByteToWideChar( CP_UTF8, 0, pUTF8Text, -1, (LPWSTR)pUnicode, unicodeLen ); 
	wstr = pUnicode;  
	delete []pUnicode;  
}  

char* WStringToUtf8(const wstring& wstr)  
{  
	char *buffer = NULL;  
	int  length = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);  
	if (length <= 0)  
	{  
		return NULL;  
	}  
	buffer = new char[length];  
	if (buffer == NULL)  
	{  
		return NULL;  
	}  
	ZeroMemory(buffer, length);  
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, buffer, length, NULL, NULL);  
	return buffer;  
}  


wchar_t * ucTowc(const unsigned char * pUC)  
{  
	// get the length  
	int nCharacters = 0; int nIndex = 0;  
	while (pUC[nIndex] != 0)  
	{  
		if ((pUC[nIndex] & 0x80) == 0)  
			nIndex += 1;  
		else if ((pUC[nIndex] & 0xE0) == 0xE0)  
			nIndex += 3;  
		else  
			nIndex += 2;  

		nCharacters += 1;  
	}  
	// make a UTF-16 string  
	wchar_t * pWC = new wchar_t [nCharacters + 1];  
	nIndex = 0; nCharacters = 0;  
	while (pUC[nIndex] != 0)  
	{  
		if ((pUC[nIndex] & 0x80) == 0)  
		{  
			pWC[nCharacters] = pUC[nIndex];  
			nIndex += 1;  
		}  
		else if ((pUC[nIndex] & 0xE0) == 0xE0)  
		{  
			pWC[nCharacters] = ((pUC[nIndex] & 0x1F) << 12) | ((pUC[nIndex + 1] & 0x3F) << 6) | (pUC[nIndex + 2] & 0x3F);  
			nIndex += 3;  
		}  
		else  
		{  
			pWC[nCharacters] = ((pUC[nIndex] & 0x3F) << 6) | (pUC[nIndex + 1] & 0x3F);  
			nIndex += 2;  
		}  

		nCharacters += 1;  
	}  
	pWC[nCharacters] = 0;  

	return pWC;   
}

string replace(const char *pszSrc, const char *pszOld, const char *pszNew)
{
	std::string strContent, strTemp;
	strContent.assign( pszSrc );
	std::string::size_type nPos = 0;
	while( true )
	{
		nPos = strContent.find(pszOld, nPos);
		strTemp = strContent.substr(nPos+strlen(pszOld), strContent.length());
		if ( nPos == std::string::npos )
		{
			break;
		}
		strContent.replace(nPos,strContent.length(), pszNew );
		strContent.append(strTemp);
		nPos +=strlen(pszNew) - strlen(pszOld)+1; //防止重复替换 避免死循环
	}
	return strContent;
}