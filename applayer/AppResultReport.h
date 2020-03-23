#ifndef __APP_RESULT_REPORT_H__
#define __APP_RESULT_SUBMIT_H__
#include "InterfaceApi.h"

class AppResultReport : public IAppSendMsgFail
{
	AppResultReport();
	void AddInfoToQueue(const string ip,const unsigned int port,const string msg);
	bool GetAnInfoFromeQueue(string &ip,unsigned int &port,string &msg);
	bool WriteAnInfoToDpFile(const string ip,const unsigned int port,const string msg);
	
public:
	virtual void OnAppSendMsgFail(const string ip,const unsigned int port,const string msg);
	bool ProsessDpFileInfoInSysInit();
	AppResultReport& GetInstance();
private:
	TimerHandler m_reSendTimerHandle;
	// TODO:
	
};


#endif