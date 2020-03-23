#include <iostream>
#include "InterfaceApiImp.h"
#include "ConfigManager.h"
#include "Log4cxxWrapper.h"
#include "DataShow.h"
#include "json/json.h"
#include <execinfo.h> 
#include <string.h>
#include "AppResultSubmit.h"



using namespace cdy;



SessionManagerImp::SessionManagerImp(/*SessionManager &wrapper*/)
	//:m_appEventListener(NULL)
	//,m_sessionUser(NULL)
	:m_idleConnectionChecker(-1)
	,m_connectionTimeout(-1)
	//,m_timerHandler(NULL)
	,m_dataShow(NULL)
{
	
	struct timeval tv1={8, 0};
	m_idleConnectionChecker = EventLooper::GetInstance().ScheduleTimer(&tv1, TF_FIRE_PERIODICALLY, this);


	struct timeval tv2={1, 0};
	m_connectionTimeout = EventLooper::GetInstance().ScheduleTimer(&tv2, TF_FIRE_PERIODICALLY, this);

	
}

/*
SessionManagerImp::SessionManagerImp()
{
	//m_appSendFail = 
}
*/

SessionManagerImp::~SessionManagerImp()
{
	if(m_appListener){
		m_appListener->Stop();
		delete  m_appListener;
	}

	if(m_tsListener){
		m_tsListener->Stop();
		delete m_tsListener;
	}

	EventLooper::GetInstance().CancelTimer(m_idleConnectionChecker);
	EventLooper::GetInstance().CancelTimer(m_connectionTimeout);
}

TimerID SessionManagerImp::NewTimer(unsigned int dwInterval,TimerHandler pTimerHander )
{
	TimerID timerForApp;
	struct timeval tv={dwInterval, 0};
	
	timerForApp = EventLooper::GetInstance().ScheduleTimer(&tv, TF_FIRE_PERIODICALLY, this);

	m_timerMap[timerForApp] = pTimerHander;

	printf("timerID: %d     %s:%d\n",timerForApp,__FUNCTION__,__LINE__);
	//if(m_timerHandler == NULL)
	//	m_timerHandler = pTimerHander;
	return timerForApp;
}
	
void SessionManagerImp::KillTimer(TimerID id)
{
	EventLooper::GetInstance().CancelTimer(id);

	map<TimerID,TimerHandler>::iterator iTimer = m_timerMap.find(id);
	if(iTimer !=  m_timerMap.end())
		m_timerMap.erase(iTimer);
	//if(m_timerHandler != NULL)
	//	m_timerHandler = NULL;
}


SessionManagerImp& SessionManagerImp::GetInstance()
{
	static  SessionManagerImp instance_;
	return instance_; 

}

bool SessionManagerImp::StartListenForAppServer(IAppNewFrame &appNewFrame)
{
	LoggerWrapper &loggerUser = LoggerWrapper::GetInstance();
	short port;
	//IAppEventListener listener;
	
	//m_appEventListener = &listener;

	
	m_appNewFrame = &appNewFrame;
	m_appResultSubmit = &DocAppResultSubmit::GetInstance();//&appResultSubmit;
	m_appResultSubmit->add(&VideoAppResultSubmit::GetInstance());
	m_appResultSubmit->add(&DpptAppResultSubmit::GetInstance());
	m_appResultSubmit->add(&NoUseTsAppResultSubmit::GetInstance());

	

	const string& addr = ConfigManager::GetIp();//appInfo.GetAppListenIp();
	port =  (short)ConfigManager::GetAppPort();//(short)appInfo.GetAppListenPort();

	m_appListener = new  Acceptor(addr, port,this);
	
	return m_appListener->StartListen() == 0;
}


bool SessionManagerImp::StartListenForTS(ITsNewFrame &tsNewFrame,ITsServerLeave &tsServerLeave)
{
	m_tsNewFrame = &tsNewFrame;

	m_tsServerLeave = &tsServerLeave;
	
	m_tsListener = new  Acceptor(ConfigManager::GetIp(), (short)ConfigManager::GetTsPort(),this);
	
	
	return m_tsListener->StartListen() == 0;
}



bool SessionManagerImp::DestoryConnectToServer(string ip,unsigned int port)
{
	map<TargetAttributeT *,IConnectorAcceptor *>::iterator it;
	

	 for(it = m_longConnectors.begin();it != m_longConnectors.end();it ++){
	 	if(((TargetAttributeT *)it->first)->ip == ip && ((TargetAttributeT *)it->first)->port == port){
			// TODO: close this connection;
			if(it->first != NULL)
				delete it->first;
			
			if(it->second)
				delete it->second;
			
			m_longConnectors.erase(it);
			
			break;
	 	} 
	}
}


void SessionManagerImp::SetDataShowProvider(IDataShowProvider &dataShow)
{
	m_dataShow = &dataShow;
	printf("####################### datashow: 0x%x\n",m_dataShow);
}


void print_trace(void)   
{   
    int i;   
    const int MAX_CALLSTACK_DEPTH = 32;    /* 需要打印堆栈的最大深度 */  
    void *traceback[MAX_CALLSTACK_DEPTH];  /* 用来存储调用堆栈中的地址 */  
    /* 利用 addr2line 命令可以打印出一个函数地址所在的源代码位置   
     * 调用格式为： addr2line -f -e /tmp/a.out 0x400618  
     * 使用前，源代码编译时要加上 -rdynamic -g 选项  
     */  
    char cmd[512] = "addr2line -f -e ";   
    char *prog = cmd + strlen(cmd);   
    /* 得到当前可执行程序的路径和文件名 */  
    int r = readlink("/proc/self/exe",prog,sizeof(cmd)-(prog-cmd)-1);   
    /* popen会fork出一个子进程来调用/bin/sh, 并执行cmd字符串中的命令，  
     * 同时，会创建一个管道，由于参数是'w', 管道将与标准输入相连接，  
     * 并返回一个FILE的指针fp指向所创建的管道，以后只要用fp往管理里写任何内容，  
     * 内容都会被送往到标准输入，  
     * 在下面的代码中，会将调用堆栈中的函数地址写入管道中，  
     * addr2line程序会从标准输入中得到该函数地址，然后根据地址打印出源代码位置和函数名。  
     */  
    FILE *fp = popen(cmd, "w");   
    /* 得到当前调用堆栈中的所有函数地址，放到traceback数组中 */  
    int depth = backtrace(traceback, MAX_CALLSTACK_DEPTH);   
    for (i = 0; i < depth; i++)   
    {   
        /* 得到调用堆栈中的函数的地址，然后将地址发送给 addr2line */  
        fprintf(fp, "%p\n", traceback[i]);   
        /* addr2line 命令在收到地址后，会将函数地址所在的源代码位置打印到标准输出 */  
    }   
    fclose(fp);   
} 



void SessionManagerImp::SendAppMessage(const string addr,const unsigned int port,const string msg,void* pTaskMessage)
{//连接失败后尝试三次间隔时间暂定5分钟

	static unsigned int ConnectTimes = 0;
	Connector *connector=NULL;
	static unsigned int connectErrorTimes = 0;

	struct ConnectorInfo *connectorInfo;

	LoggerWrapper& dsLog = LoggerWrapper::GetInstance();

	if(addr == "" || msg == "" || port < 0){
		printf("addr: %s           msg: %s     port:%d\n",addr.c_str(),msg.c_str(),port);
//		print_trace();
		dsLog.Log(true,LOGGER_ERROR," parameter error! line:%d\n",__LINE__);
		return;
	}
	
	if (!connector)
		connector = new Connector(addr, port, this);

try_connect:
	
	if (connector->Connect(300) == 0){
		connectorInfo = new struct ConnectorInfo;//(struct ConnectorInfo *)malloc(sizeof(struct ConnectorInfo));
		if(connectorInfo == NULL){
			dsLog.Log(true,LOGGER_ERROR," new struct ConnectorInfo error! \n");
			return;
		}

		connectorInfo->ip = addr;
		connectorInfo->port = port;
		connectorInfo->msg = msg;
		connectorInfo->re_connect_times = 0;
		connectorInfo->pTaskMessage = pTaskMessage;
		m_connectors[connector] = connectorInfo;

		connectErrorTimes = 0;
		
	} else {
	
		delete connector;

		connectErrorTimes ++;
		
		if (connectErrorTimes < 3){
			connector = new Connector(addr, port, this);
			goto try_connect;
		}

		connectErrorTimes = 0;
		
	}

	return;

}

void SessionManagerImp::OnNewSession(SessionImp &session)
{
	list<SessionImp*>::iterator alit;
	
	if(session.m_sessionType == SESSION_APP_CLIENT){
		
	} else if(session.m_sessionType == SESSION_APP_SERVER){

	}
}

void SessionManagerImp::OnNewSession(TsSession &session)
{
	list<struct TsSessionInfo*>::iterator tlit;
	
}




void SessionManagerImp::OnSessionClose(SessionImp &session)
{
	LoggerWrapper& dsLog = LoggerWrapper::GetInstance();
	
	list<SessionImp*>::iterator alit;
	SessionImp *session_imp;

	
	if(session.m_sessionType == SESSION_APP_CLIENT){
		
		for (alit = m_appClientSessionList.begin();alit != m_appClientSessionList.end();alit ++) {
			session_imp = *alit;
			if(session_imp == &session){
				m_appClientSessionList.erase(alit);  
				
				break;
			}
		}
		
	} else if(session.m_sessionType == SESSION_APP_SERVER){
		for (alit = m_appServerSessionList.begin();alit != m_appServerSessionList.end();alit ++) {
			session_imp = *alit;
			if(session_imp == &session){
				m_appServerSessionList.erase(alit);   
				
				break;
			}
		}

	}
}

void SessionManagerImp::OnSessionClose(TsSession &session)
{
	struct TsSessionInfo *tsSessionInfo;
	list<struct TsSessionInfo*>::iterator tlit;

	if(m_tsServerLeave != NULL)
		m_tsServerLeave->OnServerLeave(session);
	
	for (tlit = m_tsSessionList.begin();tlit != m_tsSessionList.end();tlit ++) {//printf("@@@@@@@@@@@ leng:%d     0x%x     (%d)\n",m_tsSessionList.size(),tlit,__LINE__);
		tsSessionInfo = *tlit;
		if(tsSessionInfo->session == &session){
			
			m_tsSessionList.erase(tlit);   

			delete tsSessionInfo;
			
			break;
		}
	}
}

void SessionManagerImp::OnConnection(Connection *conn, IConnectorAcceptor *ca)
{
	int ret;
	
	TsSession *ts_session;
	struct TsSessionInfo *tsSessionInfo;
	//TsSessionImp *ts_session_imp;
	TsSessionImp *ts_session_imp;
	SessionImp *session_imp;
	TsClientSession *tsClientSession;
	//TsClientSessionImp *tsClientSessionImp;


	map<IConnectorAcceptor*,struct ConnectorInfo *>::iterator it;

	map<TargetAttributeT *,IConnectorAcceptor *>::iterator lcit;

	LoggerWrapper& dsLog = LoggerWrapper::GetInstance();

	if(!ca){
		return;
	}

	if (conn){
		if (ca->IsConnector() == false){//accepter
			if (ca == m_tsListener) {//ts

				ts_session = new TsSession();
				if(ts_session == NULL){
					dsLog.Log(true,LOGGER_ERROR," new TsSession error! \n");
					return;
				}
				
				conn->SetConnectionSink(ts_session->m_imp);

				tsSessionInfo = new struct TsSessionInfo;
				if (tsSessionInfo == NULL){
					
					dsLog.Log(true,LOGGER_ERROR,"malloc error!\n");
				}

				tsSessionInfo->session = ts_session;
				
				ts_session_imp = (TsSessionImp *)ts_session->m_imp;
				//ts_session_imp->SetSessionUser(this->m_sessionUser);
				ts_session_imp->SetConnection(conn);

				tsSessionInfo->timeOut = 0;
				
				m_tsSessionList.push_back(tsSessionInfo);
				OnNewSession(*ts_session);
				
			} else if (ca == m_appListener){ //app

				session_imp = new AppServerSessionImp(conn/*,this->m_appEventListener*/);
				if(session_imp == NULL){
					dsLog.Log(true,LOGGER_ERROR," new AppServerSessionImp error! \n");
					return;
				}
				
				conn->SetConnectionSink(session_imp);
				session_imp->SetConnection(conn);

				m_appServerSessionList.push_back(session_imp);
				OnNewSession(*session_imp);
				
			} else {
				
			}
			
		} else {//connector

			 for(lcit = m_longConnectors.begin();lcit != m_longConnectors.end();lcit ++){
			 	if((IConnectorAcceptor *)lcit->second == ca){
					tsClientSession = new TsClientSession();
					if(session_imp == NULL){
						dsLog.Log(true,LOGGER_ERROR," new AppClientSessionImp error! \n");
						return;
					}

					//conn->SetConnectionSink(session_imp);
					//session_imp->SetConnection(conn);
					conn->SetConnectionSink(tsClientSession->m_imp);
					tsClientSession->m_imp->SetConnection(conn);

					TsClientSessionInfoT *info = new TsClientSessionInfoT;
					info->session =tsClientSession;//->m_imp->m_session;
					info->ip = ((TargetAttributeT *)lcit->first)->ip;
					info->port = ((TargetAttributeT *)lcit->first)->port;
					
					m_tsClinetSessionList.push_back(info);
					// TODO:OnNewSession(*session_imp);
					
					return;
			 	} 
			}
 
			it = m_connectors.find(ca);
			if(it != m_connectors.end()){  //

				session_imp = new AppClientSessionImp(conn);
				if(session_imp == NULL){
					dsLog.Log(true,LOGGER_ERROR," new AppClientSessionImp error! \n");
					return;
				}
				
				conn->SetConnectionSink(session_imp);
				session_imp->SetConnection(conn);

				m_appClientSessionList.push_back(session_imp);
				OnNewSession(*session_imp);

				if(it->second != NULL){
					ret = session_imp->SendMessage(it->second->msg);
					m_appResultSubmit->OnResultSubmit(ret,it->second->pTaskMessage);
				}
				dsLog.Log(true,LOGGER_INFO,"Send app message:%s, %s:%s:%d",it->second->msg.c_str(),__FILE__,__PRETTY_FUNCTION__,__LINE__);
					
				delete ca;

				if(it->second != NULL)
					delete it->second;
				m_connectors.erase(it);
				
			} else {  // error ,Can not find connector
				dsLog.Log(true,LOGGER_ERROR,"Can not find connector! :%d",__LINE__);
				delete ca;
			}
		}
	} else {   
		//Added by wwz 160424 for test
		printf("conn is NULL\n");
    
//		cout << "not connected, try again!" << endl;
		if (ca->IsConnector() == true){//connector
			it = m_connectors.find(ca);
			if(it == m_connectors.end()){  //This connector does not exist
				cout << "This connector does not exist :%d" << endl;
			} else {
				if(it->second != NULL){
					if(it->second->re_connect_times == 0){
						it->second->re_connect_times = 3;    //Join the Error handling map
					} else {
						if(it->second->re_connect_times == 1){  //Connection failed three times
							//Added by wwz 160424 for test,for connection failure
							printf("connection failure,send task result fail!\n");
							m_appResultSubmit->OnResultSubmit(ret,it->second->pTaskMessage);
						
							delete ca;
							delete it->second;
							m_connectors.erase(it); //it->second
						} else {
							it->second->re_connect_times -= 1;
						}
					}
				}
			}
		}
	}
}


void SessionManagerImp::OnTimer(TimerID tid)
{
	struct TsSessionInfo *tsSessionInfo;
	string json;
	Json::Value root;
	struct ConnectorInfo *info;

	root["type"] = "SvrLinkCheck";

	map<TimerID,TimerHandler>::iterator iTimer;

	//root.toStyledString();
	json = root.toStyledString();
	
	//SvrLinkCheck svrLinkCheck;
	map<IConnectorAcceptor*,struct ConnectorInfo *>::iterator mit;
	list<struct TsSessionInfo *>::iterator it;
	map<TargetAttributeT *,IConnectorAcceptor *>::iterator lcit;
	//TsMessageBase *msg;
	TsSession *tsSession;
	LoggerWrapper& dsLog = LoggerWrapper::GetInstance();
	Connector *connector=NULL;
	
	if (tid == m_connectionTimeout){
		 for(mit = m_connectors.begin();mit != m_connectors.end();){
		 	if(mit->second != NULL){
				if(mit->second->re_connect_times >= 1){
					if(((Connector *)(mit->first))->Connect(300) != 0){// Remove from map directly 
						if(mit->first != NULL)
							delete mit->first;
						delete mit->second;
						m_connectors.erase(mit ++); 
						info = (struct ConnectorInfo *)(mit->second);
						m_appSendFail->OnAppSendMsgFail(info->ip,info->port,info->msg);
					} else {
						mit ++;
					}
					
				} else {
					mit ++;
				}
		 	}
            else
            {
                mit ++;
            }
		 }

		for(lcit = m_longConnectors.begin();lcit != m_longConnectors.end();lcit ++){
			if (lcit->second == NULL){
				if(((TargetAttributeT *)lcit->first)->reconnectInterval >= LONG_CONNECT_RETRY_INTERFAL){
					if (!connector)
						connector = new Connector(((TargetAttributeT *)lcit->first)->ip,((TargetAttributeT *)lcit->first)->port, this);
					if (connector->Connect(300) == 0){
						lcit->second = connector;
					} else {
						((TargetAttributeT *)lcit->first)->reconnectInterval = 0;
					}
				} else {
					((TargetAttributeT *)lcit->first)->reconnectInterval += 1;
				}
			}
		}
	} else if(tid == m_idleConnectionChecker) {
		for(it = m_tsSessionList.begin();it != m_tsSessionList.end();) {//printf("@@@@@@@@@@@ leng:%d     0x%x     (%d)\n",m_tsSessionList.size(),it,__LINE__);
			
			tsSessionInfo = *(it++);
			if(tsSessionInfo == NULL){
				dsLog.Log(true,LOGGER_ERROR," tsSessionInfo is NUL!");
				return;
			}
			
			tsSession = tsSessionInfo->session;
			if(tsSession == NULL){
				dsLog.Log(true,LOGGER_ERROR," tsSession is NUL!");
				return;
			}

			if(tsSessionInfo->timeOut == 1) {  // Timeout handling
				dsLog.Log(true,LOGGER_DEBUG,"Timeout handling!  session:0x%x   sessionimp:0x%x\n",tsSessionInfo->session,tsSessionInfo->session->m_imp);	
				OnSessionClose(*tsSessionInfo->session);
				tsSession->Close();

			} else {
					
				tsSessionInfo->session->SendMessage(/**msg*/json);
				tsSessionInfo->timeOut = 1;

			}
		}
	//}else if(tid == m_timerForApp){ 
	//		m_timerHandler();		
	} else { //app timer handle
		
		for(iTimer = m_timerMap.begin();iTimer != m_timerMap.end();++ iTimer){
			if(tid == iTimer->first){
				((TimerHandler)iTimer->second)();
				break;
			}
		}
	}

}

//IAppEventListener* SessionManagerImp::GetAppEventListener() const
//{
//	return m_appEventListener;
//}

IAppNewFrame *SessionManagerImp::GetAppNewFrame() const
{

	return m_appNewFrame;
}


//ITsSessionUser* SessionManagerImp::GetSessionUser() const
//{
//	return m_sessionUser;
//}

ITsNewFrame* SessionManagerImp::GetTsNewFrame() const
{
	return m_tsNewFrame;
}

void SessionManagerImp::ShowConnectors()
{
	int i;
	map<IConnectorAcceptor*,struct ConnectorInfo *>::iterator mit;
	
	if(m_dataShow == NULL)
		return;
	
	m_dataShow->DataPrint("\n\n\n++++++ connectors:\n");
	for(mit = m_connectors.begin(),i = 0;mit != m_connectors.end();mit ++,i ++){
		m_dataShow->DataPrint("----------------the %dth elem: \n",i);
		m_dataShow->DataPrint("connector: 0x%x\n",(Connector *)(mit->first));
		m_dataShow->DataPrint("msg: %s\n",((struct ConnectorInfo *)mit->second)->msg.data());
		m_dataShow->DataPrint("re_connect_times: %d\n",((struct ConnectorInfo *)mit->second)->re_connect_times);
		m_dataShow->DataPrint("\n\n\n");
	}
	
}


void SessionManagerImp::ShowTsSessionList()
{
	int i;
	struct TsSessionInfo *tsSessionInfo;
	list<struct TsSessionInfo *>::iterator it;
	
	if(m_dataShow == NULL)
		return;
	
	m_dataShow->DataPrint("\n\n\n++++++ ts_session_list:\n");
	for(it = m_tsSessionList.begin(),i = 0;it != m_tsSessionList.end();it ++,i ++) {//printf("@@@@@@@@@@@ leng:%d     0x%x     (%d)\n",m_tsSessionList.size(),it,__LINE__);
		tsSessionInfo = *it;
		m_dataShow->DataPrint("----------------the %dth elem:  \n",i);
		m_dataShow->DataPrint("session: 0x%x\n",tsSessionInfo->session);
		m_dataShow->DataPrint("timeOut:%d\n",tsSessionInfo->timeOut);
		m_dataShow->DataPrint("\n\n\n");
	}
}

void SessionManagerImp::ShowAppServerSessionList()
{
	int i;
	list<SessionImp*>::iterator alit;
	
	if(m_dataShow == NULL)
		return;

	m_dataShow->DataPrint("\n\n\n++++++ app_server_session_list:\n");
	for (alit = m_appServerSessionList.begin(),i = 0;alit != m_appServerSessionList.end();alit ++,i ++) {
		m_dataShow->DataPrint("----------------the %dth elem:   \n",i);
		m_dataShow->DataPrint("session_imp: 0x%x\n",*alit);
		m_dataShow->DataPrint("\n\n\n");
	}
		
}

void SessionManagerImp::ShowAppClientSessionList()
{
	int i;
	list<SessionImp*>::iterator alit;
	
	if(m_dataShow == NULL)
		return;
	
	m_dataShow->DataPrint("\n\n\n++++++ app_client_session_list:\n");
	for (alit = m_appClientSessionList.begin();alit != m_appClientSessionList.end();alit ++,i ++) {
		m_dataShow->DataPrint("----------------the %dth elem:    \n",i);
		m_dataShow->DataPrint("session_imp: 0x%x\n",*alit);
		m_dataShow->DataPrint("\n\n\n");
	}

}



