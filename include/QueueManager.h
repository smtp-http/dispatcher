#ifndef _CODYY_QUEUE_H
#define _CODYY_QUEUE_H
#include <queue>  
#include <list>

#include <iostream>
#include <string>
#include <TsNode.h>

#include <kdvtype.h>
#include "InterfaceApi.h"
#include "TaskStatusMap.h"
#include "MessageAssemble.h"
#include "IDataPersistence.h"

const unsigned int g_dwMaxMessageCount = 10000;
using namespace std;

class AppTaskMessageCmp{
public:
	bool operator()(AppTaskMessage*,AppTaskMessage*);

};

typedef list<AppTaskMessage*> TaskMessageList;
//typedef priority_queue<AppTaskMessage*> TaskMessagePriorityQueue;
//typedef priority_queue<AppTaskMessage*,vector<AppTaskMessage*>,AppTaskMessage> TaskMessagePriorityQueue;
typedef priority_queue<AppTaskMessage*,vector<AppTaskMessage*>,AppTaskMessageCmp> TaskMessagePriorityQueue;



#if 0
class MessageQueue
{
public:
	 MessageQueue();

private:
//	AppMessage* m_ptNext; 

	unsigned int m_dwCount;
	bool m_bOrder;
	AppTaskMessage* m_ptListFirst;
	AppTaskMessage* m_ptListLast;

  //     TransServer* pTs;
//  	SEMHANDLE  m_mutex;
public:
	bool Initialize(bool bOrder);
	AppTaskMessage* Pop ( );
	void Insert (AppTaskMessage* ptAppMessage );
	bool Find(AppTaskMessage* ptAppMessage );
	bool Delete (AppTaskMessage* ptMessage);
	unsigned int GetSize();
};
#endif

class DsManage:public ITsSessionUser ,public IAppEventListener,public IQueueUser
{
    AppTaskMessage* m_patAppMess;  /*开辟的应用消息数组*/

    unsigned int m_dwAppMessCount;      /*应用消息总个数*/

    bool m_bInited;
    bool m_bQuit;
    
    string m_strTsListenIp;
    unsigned int m_dwTsListenPort;
    string m_strAppListenIp;
    unsigned int m_dwAppListenPort;
	unsigned int m_dwMaxVideoTaskTimedOutMinutes;
	unsigned int m_dwMaxDynamicPptTaskTimedOutMinutes;
	unsigned int m_dwMaxDocTaskTimedOutMinutes;
	string 		m_strTsPriority[MAX_DOCUMENT_SEVER_TYPE_NUM];
	string      m_strTsErrorIndex[MAX_DOCUMENT_SEVER_TYPE_NUM];
	TaskStatusMap taskStatusMap;

public:	 
    TaskMessagePriorityQueue m_TransDocWaitQueue;
    TaskMessageList m_TransDocLinuxRunQueue;
    TaskMessageList m_TransDocOfficeRunQueue;
	TaskMessageList m_TransDocWpsRunQueue;

    TaskMessagePriorityQueue m_VideoWaitQueue;
    TaskMessageList m_VideoRunQueue;

    TaskMessagePriorityQueue m_TransDynamicPptWaitQueue;
    TaskMessageList m_TransDynamicPptRunQueue;

    TaskMessageList m_FreeMessageQueue;      
    TransServer   DocLinuxTs[MAX_DOC_LINUX_TRANS_SERVER_NUM];
	TransServer   DocOfficeTs[MAX_DOC_OFFICE_TRANS_SERVER_NUM];
	TransServer   DocWpsTs[MAX_DOC_WPS_TRANS_SERVER_NUM];
    TransServer   VideoTs[MAX_VIDEO_TRANS_SERVER_NUM];
    TransServer   PPTTs[MAX_DANAMIC_PPT_TRANS_SERVER_NUM];

	IDataPresistence *m_dataPresistence;

	//Added by wwz 160419
	TaskMessageList m_lastDocRunList;
	TaskMessageList m_lastVideoRunList;
	TaskMessageList m_lastDpptRunList;

public:
	virtual bool GetDocWaitQueueMessage(DpMessageArryType &dpMsgArry);
	virtual bool GetDpptWaitQueueMessage(DpMessageArryType &dpMsgArry);
	virtual bool GetVideoWaitQueueMessage(DpMessageArryType &dpMsgArry);

	virtual bool GetDocRunQueueMessage(DpMessageArryType &dpMsgArry);
	virtual bool GetDpptRunQueueMessage(DpMessageArryType &dpMsgArry);
	virtual bool GetVideoRunQueueMessage(DpMessageArryType &dpMsgArry);

	//Added by wwz 160408
	bool DocTaskMapEncode(DpMessageType &dpMessage,AppTaskMessage * taskMessage);
	bool VideoTaskMapEncode(DpMessageType &dpMessage,AppTaskMessage * taskMessage);
	bool DpptTaskMapEncode(DpMessageType &dpMessage,AppTaskMessage * taskMessage);
	bool DocTaskMapDecode(DpMessageArryType &dpMessageArray,bool runTask);
	bool VideoTaskMapDecode(DpMessageArryType &dpMessageArray,bool runTask);
	bool DpptTaskMapDecode(DpMessageArryType &dpMessageArray,bool runTask);

	//Added by wwz 160421
	
	//For disp restart or transerver reconnect
	bool LinuxDocReconnRegProcess(RegisterInfo &regInfo,TsSession &tsSession);
	bool OfficeDocReconnRegProcess(RegisterInfo &regInfo,TsSession &tsSession);
	bool WpsDocReconnRegProcess(RegisterInfo &regInfo,TsSession &tsSession);
	bool VideoReconnRegProcess(RegisterInfo &regInfo,TsSession &tsSession);
	bool DpptReconnRegProcess(RegisterInfo &regInfo,TsSession &tsSession);
	//For register timeout after disp restart 
	static int OnReconnRegisterTimeOut();
	void CheckLastDocRunList();
	void CheckLastVideoRunList();
	void CheckLastDpptRunList();
	
	virtual void OnNewFrame(AppServerSession *appServerSession,const string frame);
	virtual void OnNewFrame(TsSession* tsSession,const string frame);
	void BuildJsonMsg(TsMessageBase &base,string &json);
	void BuildJsonMsg(AppMessageBase &base,string &json);
	int GetTransWaitQueuePosition(TaskMessagePriorityQueue* const queue,string taskId,TaskMessagePriorityQueue *witeQueue);
    DsManage();
    ~DsManage();    
    bool Initialize();
    unsigned int MessageNew(AppTaskMessage** pptNewMessage);

    bool IsQuitingDs();
	void OnRegister(RegisterInfo &regInfo, TsSession &session) ;
	void OnNewProgressReq(AppTaskProgressReq &taskInfo,AppServerSession *appServerSion);
	void OnTaskResult(TaskResult &taskResult, TsSession &session) ;
	void OnNewProgressInfo(TaskResult &taskResult,TsSession &session) ;
    virtual void OnServerLeave(TsSession &session);    //服务器掉线等事件,网络断开与心跳检测失败； 
    void OnSessionError(TsSession &session);  //协议解析失败等，一般仅仅需要关闭session   

	virtual void OnResultSubmit(int result,void *pTaskMessage);
 
 	void OnNewTask(AppTaskMessage &taskInfo,AppServerSession *appServerSession) ;
	void OnDocLinuxSvrRegister(RegisterInfo &regInfo, TsSession &session);
	void OnDocOfficeSvrRegister(RegisterInfo &regInfo, TsSession &session);	
	void OnDocWpsSvrRegister(RegisterInfo &regInfo, TsSession &session);		
	void OnPptSvrRegister(RegisterInfo &regInfo, TsSession &session);
	void OnVideoSvrRegister(RegisterInfo &regInfo, TsSession &session);
	void OnNewDocTask(AppTaskMessage &taskInfo,AppServerSession *appServerSession) ;	
	void OnNewPptTask(AppTaskMessage &taskInfo,AppServerSession *appServerSession) ;	
	void OnNewVideoTask(AppTaskMessage &taskInfo,AppServerSession *appServerSession) ;	
	void OnDocCommonTaskResult(TaskResult &taskResult, TsSession &session,TaskMessageList& taskRunlist,TransServer* pTs,unsigned int dwTsNo,DocTransServerType enumTsType);
	void OnDocLinuxTaskResult(TaskResult &taskResult, TsSession &session);
	void OnDocOfficeTaskResult(TaskResult &taskResult, TsSession &session);
	void OnDocWpsTaskResult(TaskResult &taskResult, TsSession &session);
	void OnPptTaskResult(TaskResult &taskResult, TsSession &session);
	void OnVideoTaskResult(TaskResult &taskResult, TsSession &session);
	bool OnDocCommonServerLeave(TsSession &session,TaskMessageList& taskRunlist,TransServer* pTs,unsigned int dwTsCount);
	bool OnDocLinuxServerLeave(TsSession &session);   
	bool OnDocOfficeServerLeave(TsSession &session);
	bool OnDocWpsServerLeave(TsSession &session);
	bool OnPptServerLeave(TsSession &session);
	bool OnVideoServerLeave(TsSession &session);	
	static int OnTimer();
	static int OnReconnectTimeOut();
	int DocLinuxRunQueueCheck();
	int DocOfficeRunQueueCheck();
	int DocWpsRunQueueCheck();
	int DocWaitQueueCheck();
	int PptWaitQueueCheck();
	int PptRunQueueCheck();	
	int VideoWaitQueueCheck();
	int VideoRunQueueCheck();
	void HandleNewDocTaskForAutoTsType(AppTaskMessage* pNewAppTaskMessage);
	bool HandleNewDocTaskForCommonTsType(AppTaskMessage* pNewAppTaskMessage,TaskMessageList& taskRunlist,TransServer* pTs,unsigned int dwTsCount);
	void HandleNewDocTaskForAppointedTsType(AppTaskMessage* pNewAppTaskMessage,TaskMessageList& taskRunlist,TransServer* pTs,unsigned int dwTsCount);
	void HandleVideoWaitQueue(TsSession &session);
	void HandleDocWaitQueue(TsSession &session);
	void HandlePptWaitQueue(TsSession &session);

	void HandleDocWaitQueueForSingleTs(TaskMessageList& taskRunlist,TransServer* pTs,DocTransServerType enumTsType,unsigned int dwTsNo);
	void HandlePptWaitQueueForSingleTs(unsigned int dwTsNo);
	void HandleVideoWaitQueueForSingleTs(unsigned int dwTsNo);
	void HandleCommonWaitQueueForSingleTs(TaskMessagePriorityQueue& taskWaitQueue, TaskMessageList& taskRunlist,TransServer* pTs,unsigned int dwTsNo);
	void RegisterPreProcess(RegisterInfo &regInfo, TsSession &session,TransServer* pTs, unsigned int dwTsCount, bool& bFind, bool& bResult,unsigned int& dwTsFreeIndex);
	void RegisterVideoProcess(RegisterInfo &regInfo, TsSession &session,TransServer* pTs, unsigned int dwTsCount, bool& bFind, bool& bResult,unsigned int& dwTsFreeIndex);
	void SetTsListenIp(string strTsListenIp);
    virtual string GetTsListenIp();
    void SetTsListenPort(unsigned int dwTsListenPort);
    virtual unsigned int GetTsListenPort();
    void SetAppListenIp(string strAppListenIp);
    virtual string GetAppListenIp();
    void SetAppListenPort(unsigned int dwAppListenPort);
    virtual unsigned int GetAppListenPort();    
	bool SetTsPriority(string* strArray, int count);
	void SetDataPresistence(IDataPresistence *dataPresistence);
	string* GetTsPriority();
	string* GetTsErrorIndex();
	void SetTimedOutProperty(unsigned int dwDocTaskTimedOutMinutes,unsigned int dwVideoTaskTimedOutMinutes,unsigned int dwDynamicPptTaskTimedOutMinutes);
	unsigned int  GetDocTaskTimedOutMinutes();
	unsigned int  GetVideoTaskTimedOutMinutes();

	bool VideoHeavyTaskDisp(AppTaskMessage* pNewAppTaskMessage,bool bCanDispOriginalTs);
	bool VideoLightTaskDisp(AppTaskMessage* pNewAppTaskMessage,bool bCanDispOriginalTs);	
	unsigned int  GetDynamicPptTaskTimedOutMinutes();
	bool IsOnlyOneTypeServerRegistered(string strTsType);	
	bool IsPresentRegisteredTsTypeAllFinished(unsigned int* dwErrorCount);
	bool HandleAppDocTaskToOtherSameTypeTs(AppTaskMessage* pAppTaskMessage, string strTsType);
	bool HandleCommonAppDocTaskToOtherTs(AppTaskMessage* pAppTaskMessage, TaskMessageList& taskRunlist,TransServer* pTs,unsigned int dwTsNo);
	void MoveTaskFromRunQueueToWaitQueue(AppTaskMessage* pAppTaskMessage, TsType enumTsType);
	void CommonMoveTaskFromRunQueueToWaitQueue(AppTaskMessage* pAppTaskMessage, TaskMessagePriorityQueue& taskWaitQueue,TaskMessageList& taskRunlist,TransServer* pTs,unsigned int dwTsNo);
 
	void GetServerStatus(AppServerStatusResult &taskInfo);
	int CommonDocAndPptResultReport(AppTaskMessage* pTaskMessage,bool bResult,unsigned int dwErrorCode, unsigned int dwPageCount);
	int CommonDocAndPptTimedOutReport(AppTaskMessage* pTaskMessage);
	int CommonDocAndPptRunQueueCheck(TaskMessagePriorityQueue& taskWaitQueue,TaskMessageList& taskRunlist, unsigned int dwTimedOutMinutes, TransServer* pTs,unsigned int dwTsCount,TsType enumTsType);
	int CommonVideoTaskResultReport( AppTaskMessage* pTaskMessage, 	bool bResult, unsigned int dwErrorCode, string strTargetFileName = "", unsigned int dwPageCount = 0,TVideoInfoResultParm* pVideoResultParam = NULL);
	int VideoTaskTimedOutReport( AppTaskMessage* pTaskMessage);
	bool HandleCommonAppVideoTaskToOtherTs(AppTaskMessage* pTaskMessage);
	void CommonVideoTaskSend(AppTaskMessage* pTaskMessage, unsigned int dwVideoTsIndex);
	void HandleVideoTsReconnectTimer();
};



#endif//_QUEUE_H
