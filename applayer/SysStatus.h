#ifndef __SYS_STATUS_H__
#define __SYS_STATUS_H__
#include "json/json.h"
#include "DataPersistence.h"
#include "EventLooper.h"
#include "task.h"
#include "Disptacher.h"

using namespace cdy;

class TaskWaitQueue;
class TaskRunQueue;

#define CopyMessageToValue(dpDocMessageArr,type)   {\ 
	for (iter = dpDocMessageArr.begin();iter!=dpDocMessageArr.end();iter++) {\
		  	Json::Value cmdValue;\
			for(it = (*iter).begin();it != (*iter).end();++ it) {\
				cmdValue[it->first] = it->second;\
			}\
		  	task = AddTask::BuildBasicTask(cmdValue,type);\
			m_taskWaitQueues[type]->PushTask(task);\
		  }\
	}
class SysStatus:public IQueueUser,public IGetResultReTransMsg
{
	SysStatus();
	IDataPresistence *m_dataPresistence;
	map<TaskType,TaskWaitQueue *> m_taskWaitQueues;
	map<TaskType,TaskRunQueue*> m_taskRunQueues;
	map<TaskType,TaskDb*> m_taskDb;
	map<TaskType,Disptacher*> m_disptacher;
	unsigned int m_DocTaskTimedOutMinutes;
	unsigned int m_DynamicPptTaskTimedOutMinutes;
	unsigned int m_VideoTaskTimedOutMinutes;

	map<string,DocTsType> m_docTaskAppointedTsType;

public:
	static SysStatus& GetInstance();
	void SetDataPresistence(IDataPresistence *dataPresistence);
	bool GetWaitQueueMessage(DpMessageArryType &dpMsgArry,TaskWaitQueue &waitQ);

	virtual bool GetDocWaitQueueMessage(DpMessageArryType &dpMsgArry);
	virtual bool GetDpptWaitQueueMessage(DpMessageArryType &dpMsgArry);
	virtual bool GetVideoWaitQueueMessage(DpMessageArryType &dpMsgArry);
	
	virtual bool GetDocRunQueueMessage(DpMessageArryType &dpMsgArry);
	virtual bool GetDpptRunQueueMessage(DpMessageArryType &dpMsgArry);
	virtual bool GetVideoRunQueueMessage(DpMessageArryType &dpMsgArry);

	virtual bool GetResultReTransMsg(DpMessageArryType &dpMsgArry);

	void SysStatusRestore();

	int RunQueueCheck(TaskType type, unsigned int taskTimeOutMins);
	int DocRunQueueCheck();
    int VideoRunQueueCheck();
    int DpptRunQueueCheck();

	int WaitQueueCheck(TaskType type);
    int DocWaitQueueCheck();
    int VideoWaitQueueCheck();
    int DpptWaitQueueCheck();
	static int OnTimer();
private:
	//bool CopyJsonvalueToMap(Json::Value &json,DpMessageType &message);
	bool GetRunQueueMessage(DpMessageArryType &dpMsgArry,TaskRunQueue &runQ);

	void CopyMessageToQueue(TaskType type);
	
};









#endif

