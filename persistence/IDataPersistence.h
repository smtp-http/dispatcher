#ifndef __IDATA_PERSISTENCE_H__
#define __IDATA_PERSISTENCE_H__
#include <string>
#include <map>
#include <vector>
#include "TaskComm.h"
#include "json/json.h"

using namespace std;

/*
typedef enum{
	DocTask = 0,
	VideoTask,
	DpptTask
}TaskType;
*/
typedef enum{
	QueueWait = 0,
	QueueRuning
}QueueType;

typedef Json::Value DpMessageType;
typedef vector<DpMessageType> DpMessageArryType;


class IQueueUser{
public:
	virtual bool GetDocWaitQueueMessage(DpMessageArryType &dpMsgArry) = 0;
	virtual bool GetDpptWaitQueueMessage(DpMessageArryType &dpMsgArry) = 0;
	virtual bool GetVideoWaitQueueMessage(DpMessageArryType &dpMsgArry) = 0;

	virtual bool GetDocRunQueueMessage(DpMessageArryType &dpMsgArry) = 0;
	virtual bool GetDpptRunQueueMessage(DpMessageArryType &dpMsgArry) = 0;
	virtual bool GetVideoRunQueueMessage(DpMessageArryType &dpMsgArry) = 0;
};

class IGetResultReTransMsg{
public:
	virtual bool GetResultReTransMsg(DpMessageArryType &dpMsgArry) = 0;
};

class IDataPresistence{
public:
	//static IDataPresistence &CreatePresistence(IQueueUser *user);
	virtual bool DpGetMessage(DpMessageArryType &dpMsg,TaskType taskType) = 0;
	virtual bool DpGetWaitMessage(DpMessageArryType &dpMsg,TaskType taskType) = 0;
	virtual bool DpGetRunMessage(DpMessageArryType &dpMsg,TaskType taskType) = 0;
	virtual bool CreateDpMsgFile(TaskType taskType) = 0;
	virtual bool CreateDpMsgFile(const DpMessageType &dpMsg,TaskType taskType) = 0;
	//virtual bool CreateDpMsgFile(QueueType qt,const DpMessageType &dpMsg,TaskType taskType) = 0;
	virtual bool CreateWaitMsgFile(TaskType taskType) = 0;
	virtual bool CreateRunMsgFile(TaskType taskType) = 0;
	
};

#endif
