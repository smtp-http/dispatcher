#include "AppResultReport.h"


AppResultReport& AppResultReport::GetInstance()
{
	static  AppResultReport instance_;
	return instance_; 

}

void AppResultReport::AddInfoToQueue(const string ip,const unsigned int port,const string msg)
{

}

bool AppResultReport::GetAnInfoFromeQueue(string &ip,unsigned int &port,string &msg)
{

}


int AppResultReSendTimerHandle()
{
	
}

AppResultReport::AppResultReport()
{
	unsigned int dwInterval; // TODO: init
	m_reSendTimerHandle = AppResultReSendTimerHandle;
	SessionManager::GetInstance().NewTimer(dwInterval, m_reSendTimerHandle);
}

void AppResultReport::OnAppSendMsgFail(const string ip, const unsigned int port, const string msg)
{

}

bool AppResultReport::ProsessDpFileInfoInSysInit()
{

}




