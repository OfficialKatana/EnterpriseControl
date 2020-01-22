#pragma once
#include "HPSocket.h"
#include "Protocol.h"

struct STPACKETBUF
{
	int    nStartLen;
	LPBYTE pBuf;   
	STPACKETBUF()
	{
		nStartLen=0;
		pBuf = new BYTE[MAX_RECV_BUFFER];
		memset(pBuf,0,MAX_RECV_BUFFER);
	}
};

class CServer:public  CTcpServerListener
{
public:
	CServer(void);
	~CServer(void);
public:
	void            Start(USHORT usPort);
	EnHandleResult  SendData(CONNID dwConnID, const BYTE* pData, int iLength);
private:
	virtual EnHandleResult OnPrepareListen(ITcpServer* pSender, SOCKET soListen);
	virtual EnHandleResult OnAccept(ITcpServer* pSender, CONNID dwConnID, SOCKET soClient);
	virtual EnHandleResult OnSend(ITcpServer* pSender, CONNID dwConnID, const BYTE* pData, int iLength);
	virtual EnHandleResult OnReceive(ITcpServer* pSender, CONNID dwConnID, const BYTE* pData, int iLength);
	virtual EnHandleResult OnClose(ITcpServer* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode);
	virtual EnHandleResult OnShutdown(ITcpServer* pSender);
private:
	CTcpServerPtr m_pServer;	
	STPACKETBUF   m_stPackBuf;
};

