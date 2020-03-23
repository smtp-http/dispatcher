#ifndef _TSNODE_H
#define _TSNODE_H

#include <string>
//#include <sockLib.h>
#include "../../../10-common/include/system/kdvtype.h"
#include "MessageAssemble.h"
#include "InterfaceApi.h"
#define SOCKHANDLE			 int
#define MAX_DOC_LINUX_TRANS_SERVER_NUM   1000
#define MAX_DOC_OFFICE_TRANS_SERVER_NUM   1000
#define MAX_DOC_WPS_TRANS_SERVER_NUM   1000
#define MAX_DANAMIC_PPT_TRANS_SERVER_NUM   1000
#define MAX_VIDEO_TRANS_SERVER_NUM   1000
#define MAX_DOCUMENT_SEVER_TYPE_NUM 3
//extern class AppMessage;
//

typedef enum
{
    TS_UNREGISTERED = 0,

    TS_FREE = 1,

    TS_RUNNING = 2,

	TS_WAIT_RECONNECT = 3
    
//    TS_WAITING = 3,
        
//    TS_LOST = 4

} TsState;


class TransServer
{
private:
	TsType  m_Type; 
	TaskProperty m_Property;
	TsState m_State; 
	u32 m_dwTsNo;
//	u32 m_dwTsLinkCheckCount;
//	bool m_bResponse;
	void* m_pMessageInTs;
	string m_strIpaddr;
	string m_strTsId;
	TsSession* m_pTsSession;

	
public:
	int m_reconnectTimeoutCount;
	int m_max_reconnectTimeoutTimes;
        TransServer();
		
	TsState GetState();
	void SetState(TsState eState);

	bool Initialize();
	string GetIpAddr();
	void SetTsType(TsType tsType);
	TsType GetTsType();
	void SetTsProperty(TaskProperty property);
	TaskProperty GetTsProperty();
	void SetIpAddr(string strIpAddr);
	string GetTsId();

	void SetTsId(string strTsId);		
	TsSession* GetTsSession();
	void SetTsSession(TsSession* tsSession);	
	void* GetMessageInTs();
	void SetMessageInTs(void*  pAppMessage);
};



#endif//__TSNODE_H
