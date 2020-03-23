#include "ConnBase.h"
#include "InterfaceApiImp.h"


using namespace cdy;
using namespace std;


SessionManager::SessionManager()
{

}


SessionManager::~SessionManager()
{

}


SessionManager& SessionManager::GetInstance()
{
	
	static SessionManager instance_;     
	SessionManagerImp &instance_imp = SessionManagerImp::GetInstance();
	instance_.m_imp = &instance_imp;
	return instance_; 
}


bool SessionManager::StartListenForAppServer(IAppNewFrame &appNewFrame)
{
	if(m_imp)
		return m_imp->StartListenForAppServer(appNewFrame);
	else
		return 0;
}


bool SessionManager::StartListenForTS(ITsNewFrame &tsNewFrame,ITsServerLeave &tsServerLeave)
{
	if(m_imp)
		return m_imp->StartListenForTS(tsNewFrame,tsServerLeave);
	else
		return 0;
}


bool SessionManager::DestoryConnectToServer(string ip, unsigned int port)
{
	if(m_imp)
		return m_imp->DestoryConnectToServer(ip, port);
	else
		return 0;
}


void SessionManager::SetDataShowProvider(IDataShowProvider &dataShow)
{
	if(m_imp)
		return m_imp->SetDataShowProvider(dataShow);
}


//void SessionManager::SendAppMessage(AppMessageBase& appMessage)
//{
//	if(m_imp)
//		m_imp->SendAppMessage(appMessage);
//}

void SessionManager::SendAppMessage(const string ip,const unsigned int port,const string msg,void * pTaskMessage)
{
	if(m_imp)
		m_imp->SendAppMessage(ip,port,msg,pTaskMessage);
}

TimerID SessionManager::NewTimer(unsigned int dwInterval, TimerHandler pTimerHander)
{
	if(m_imp){
		return m_imp->NewTimer(dwInterval,pTimerHander);
	}
}

void SessionManager::KillTimer(TimerID id)
{
	if(m_imp)
		m_imp->KillTimer(id);
}



void SessionManager::ShowConnectors()
{
	if(m_imp)
		m_imp->ShowConnectors();
}


void SessionManager::ShowTsSessionList()
{
	if(m_imp)
		m_imp->ShowTsSessionList();
}

void SessionManager::ShowAppServerSessionList()
{
	if(m_imp)
		m_imp->ShowAppServerSessionList();
}
void SessionManager::ShowAppClientSessionList()
{
	if(m_imp)
		m_imp->ShowAppClientSessionList();
}


