#ifndef __INTERFACEAPIIMP_H__
#define __INTERFACEAPIIMP_H__

#include "ConnBase.h"
#include "EventLooper.h"
#include "InterfaceApi.h"
#include <vector>
#include <list>
#include <map>
#include <stdarg.h> 

#include <log4cxx/logger.h>
#include <log4cxx/logstring.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/exception.h>  

using namespace cdy;
using namespace std;
using namespace log4cxx;

class SessionImp;

#define LONG_CONNECT_RETRY_INTERFAL 2

typedef enum
{
	SESSION_TS = 0,
		
	SESSION_APP_SERVER = 1,

	SESSION_APP_CLIENT = 2,

	SESSION_TS_CLIENT = 3

} SessionType;



class SessionImp : public IConnectionSink
{
public:

	int SendMessage(string &str);
	
	virtual void Close() = 0;


	virtual void SetConnection(Connection *connection) = 0;

protected:
	void ShowReadBuf();

	void ProcessFrame(const char *buf, int length);

	virtual bool OnFrame(string &frame) = 0;
	
	Connection *m_connection;
	
	SessionType m_sessionType;
	
	vector<char> m_rdBuffer; //initial size set 1024.
	
	long m_idleTime;

	TimerID m_timeout;

	friend class SessionManagerImp;

};


class TsSessionImp : public SessionImp
{
public:
	TsSessionImp(/*ITsSessionUser *sessionUser,*/TsSession *tssession);
	
	//void SetSessionUser(ITsSessionUser *sessionUser);
	
	virtual void Close();

//	virtual void SetSessionType(){m_sessionType = SESSION_TS;}
	
	virtual void SetConnection(Connection *connection){m_connection = connection;}
protected:
	virtual void OnData(const char *buf, int length, Connection *conn);
	// called when user can write data to network.
	virtual void OnWrite(Connection *conn);
	// called when the connection is broken, if user call Connection::Close(), //&&&
	// will not get this callback.
	virtual void OnDisconnect(int reason, Connection *conn);  //

	virtual bool OnFrame(string &frame);
private:
	//ITsSessionUser *m_sessionUser;

	class TsSession* m_tssession;

};


class TsClientSessionImp : public SessionImp
{
public:
	TsClientSessionImp(Connection *connection,TsClientSession *tsClientSession)
	{
		if(connection != NULL)
			m_connection = connection;
			m_sessionType = SESSION_TS_CLIENT;
			//m_session = new class  TsClientSession(this);
			m_session = tsClientSession;
	}


	virtual void Close();

	//virtual void SetSessionType(){m_sessionType = SESSION_APP_CONNECT;}

	virtual void SetConnection(Connection *connection){m_connection = connection;}
protected:
	virtual void OnData(const char *buf, int length, Connection *conn);
	// called when user can write data to network.
	virtual void OnWrite(Connection *conn);
	// called when the connection is broken, if user call Connection::Close(), //&&&
	// will not get this callback.
	virtual void OnDisconnect(int reason, Connection *conn);  //

	virtual bool OnFrame(string &frame);
private:
	class TsClientSession *m_session;
};


class AppServerSessionImp : public SessionImp
{
public:
	AppServerSessionImp(Connection *connection/*,IAppEventListener *event_listener*/)
		//: m_eventListener(event_listener)
	{
		m_connection = connection;
		m_sessionType = SESSION_APP_SERVER;
		m_session = new class AppServerSession(this);
	}

	~AppServerSessionImp()
	{
		delete m_session;
		m_session = NULL;
	}
	

	virtual void Close();

	//virtual void SetSessionType(){m_sessionType = SESSION_APP_LISTEN;}

	virtual void SetConnection(Connection *connection){m_connection = connection;}
protected:
	virtual void OnData(const char *buf, int length, Connection *conn);
	// called when user can write data to network.
	virtual void OnWrite(Connection *conn);
	// called when the connection is broken, if user call Connection::Close(), //&&&
	// will not get this callback.
	virtual void OnDisconnect(int reason, Connection *conn);  //

	virtual bool OnFrame(string &frame);
private:
//	IAppEventListener *m_eventListener;
	class AppServerSession *m_session;

};


class AppClientSessionImp : public SessionImp
{
public:
	AppClientSessionImp(Connection *connection)
	{
		if(connection != NULL)
			m_connection = connection;
			m_sessionType = SESSION_APP_CLIENT;
			m_session = new class AppClientSession(this);
	}


	virtual void Close();

	//virtual void SetSessionType(){m_sessionType = SESSION_APP_CONNECT;}

	virtual void SetConnection(Connection *connection){m_connection = connection;}
protected:
	virtual void OnData(const char *buf, int length, Connection *conn);
	// called when user can write data to network.
	virtual void OnWrite(Connection *conn);
	// called when the connection is broken, if user call Connection::Close(), //&&&
	// will not get this callback.
	virtual void OnDisconnect(int reason, Connection *conn);  //

	virtual bool OnFrame(string &frame);
private:
	class AppClientSession *m_session;
};




struct TsSessionInfo{
	TsSession *session;
	bool timeOut;
};

typedef struct TsClientSessionInfo{
	TsClientSession *session;
	string ip;
	unsigned int port;
}TsClientSessionInfoT;


typedef struct TsTargetAttribute
{
	string ip;
	unsigned int port;
	unsigned int reconnectInterval;
}TargetAttributeT;

class AppResultSubmit;

class SessionManagerImp : public IConnectorAcceptorSink, public ITimerUserSink
{
	//SessionManager &sessionmanager;
	SessionManagerImp(/*SessionManager &wrapper*/);
	
public:
	
	~SessionManagerImp();
//-------- for test ----------------	
	void ShowTsSessionList();
	void ShowConnectors();
	void ShowAppServerSessionList();
	void ShowAppClientSessionList();
//-------------------------------


	 static SessionManagerImp& GetInstance();

	bool StartListenForAppServer(IAppNewFrame &appNewFrame);

	bool StartListenForTS(ITsNewFrame &tsNewFrame,ITsServerLeave &tsServerLeave);

	bool DestoryConnectToServer(string ip,unsigned int port);

	void SetDataShowProvider(IDataShowProvider &dataShow);

//	void SendAppMessage(AppMessageBase &appMessage);
	
	void SendAppMessage(const string ip,const unsigned int port,const string msg,void* pTaskMessage);

	void OnNewSession(SessionImp &session);

	void OnNewSession(TsSession &session);

	void OnSessionClose(SessionImp &session);

	void OnSessionClose(TsSession &session);

	//IAppEventListener* GetAppEventListener() const;
//	ITsSessionUser* GetSessionUser() const;
	ITsNewFrame* GetTsNewFrame() const;
	IAppNewFrame *GetAppNewFrame() const;

	TimerID NewTimer(unsigned int dwInterval,TimerHandler pTimerHander );
	
	void KillTimer(TimerID id);	
protected:
	virtual void OnConnection(Connection *conn, IConnectorAcceptor *ca);
	virtual void OnTimer(TimerID tid);
private:
	
	//IAppEventListener *m_appEventListener;
	IAppSendMsgFail *m_appSendFail;
	IAppNewFrame *m_appNewFrame;
	AppResultSubmit *m_appResultSubmit;
	
	//ITsSessionUser *m_sessionUser;

	ITsNewFrame *m_tsNewFrame;
	ITsServerLeave *m_tsServerLeave;
	IDataShowProvider *m_dataShow;

	Acceptor *m_appListener;
	Acceptor *m_tsListener;
	
	//list<SessionImp*> m_sessions;

	struct ConnectorInfo{
		string ip;
		unsigned int port;
		string msg;
		unsigned int re_connect_times;
		void* pTaskMessage;
	};


	map<IConnectorAcceptor*,struct ConnectorInfo *> m_connectors;

	map<TargetAttributeT *,IConnectorAcceptor *> m_longConnectors;

	//struct ServerInfo{
	//	string ip;
	//	unsigned int port;
	//};
	//map<struct ServerInfo*,bool>  m_clientTsConnectors;

	typedef list<struct TsSessionInfo*>TsSessionList;

	TsSessionList m_tsSessionList;

	list <TsClientSessionInfo*> m_tsClinetSessionList;

	typedef list<SessionImp*> SessionImpList;
	
	SessionImpList m_appServerSessionList;

	SessionImpList m_appClientSessionList;

	
	//timer
	TimerID m_idleConnectionChecker;

	TimerID m_connectionTimeout;

	//TimerID m_timerForApp;

	map<TimerID,TimerHandler> m_timerMap;

	//TimerHandler m_timerHandler;

	friend class TsSessionImp;

};

typedef enum{
	DISCON_RECV_RESPONSE = 0,
	DISCON_APP_CLOSE = 1
}DisconnectReason;

void print_trace(void);


#endif

