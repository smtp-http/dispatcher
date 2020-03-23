#ifndef _INTERFACEAPI_H
#define _INTERFACEAPI_H

//#include "MessageAssemble.h"
#include <string>
#include "EventLooper.h"

using namespace std;
using namespace cdy;

typedef int (*TimerHandler)(void);


class TsSession
{

public:
	TsSession();
	
	~TsSession();
	

	void SendMessage(string msg);
	
	void Close();
	
private:

	class SessionImp *m_imp;

	 friend class SessionManagerImp;
};

class IDataShowProvider
{
public:
	//virtual bool DataShowInString(string str)= 0;
	virtual bool DataPrint(char *szFormat, ...)=0;
};

class AppServerSession
{
public:
	AppServerSession(class AppServerSessionImp *imp);
	~AppServerSession();

	void SendMessage(string msg);
	
	

private:

	SessionImp *m_imp;
	
};

class TsClientSession
{
public:
	TsClientSession();
	~TsClientSession();

	void SendMessage(string msg);
	
private:
	SessionImp *m_imp;
	friend class SessionManagerImp;
};

class AppClientSession
{
public:
	AppClientSession(class AppClientSessionImp *imp);
	~AppClientSession();

	void SendMessage(string msg);
	
private:
	
	SessionImp *m_imp;
};


class ITsNewFrame
{
public:
	virtual void OnNewFrame(TsSession* tsServerSion,const string frame) = 0;
};

class ITsServerLeave
{
public:
	virtual void OnServerLeave(TsSession &session) = 0;
};

class ITsGetInfo
{
public:
	virtual string GetTsListenIp()= 0;
	virtual unsigned int GetTsListenPort()= 0;
};

class IAppSendMsgFail
{
public:
	virtual void OnAppSendMsgFail(const string ip,const unsigned int port,const string msg) = 0;
};



class IAppNewFrame
{
public:
	virtual void OnNewFrame(AppServerSession *appServerSion,string frame) =0;
};
/*
class IAppGetInfo
{
public:
	virtual string GetAppListenIp()= 0;
	virtual unsigned int GetAppListenPort()= 0;
};
*/
class IAppResultSubmit
{
public:
	virtual void OnResultSubmit(int result,void *pTaskMessage) = 0;
};

class SessionManager  
{
	SessionManager();
	~SessionManager();
public:
	static SessionManager& GetInstance();

	void SetDataShowProvider(IDataShowProvider &dataShow);
	
	bool StartListenForAppServer(IAppNewFrame &appNewFrame);

	bool StartListenForTS(ITsNewFrame &tsNewFrame,ITsServerLeave &tsServerLeave);

	bool DestoryConnectToServer(string ip, unsigned int port);

//	void SendAppMessage(AppMessageBase& appMessage);

	void SendAppMessage(const string ip,const unsigned int port,const string msg,void* pTaskMessage);
	
	TimerID NewTimer(/*unsigned int& dwTimeId,*/unsigned int dwInterval,TimerHandler pTimerHander );  /*dwInterval  是秒计数的*/
	void KillTimer(/*unsigned int dwTimeId*/TimerID id);

	//-------- for test ----------------	
	void ShowTsSessionList();
	void ShowConnectors();
	void ShowAppServerSessionList();
	void ShowAppClientSessionList();
//-------------------------------

private:
	class SessionManagerImp *m_imp;
};


#endif//_INTERFACEAPI_H


