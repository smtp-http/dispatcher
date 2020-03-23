#ifndef __TASK_RESULT_H__
#define __TASK_RESULT_H__

#include <iostream>
#include <string>
#include <vector>
#include "TsInterface.h"
#include "json/json.h"
#include "InterfaceApi.h"
#include "Disptacher.h"
#include "Log4cxxWrapper.h"
//#include "CommandDb.h"
#include "Command.h"

class CommandDb;

extern TsQueue GVideoTsQueue;
extern TsQueue GDocOpenofficeTsQueue;
extern TsQueue GDocWpsTsQueue;
extern TsQueue GDocOfficeTsQueue;
extern TsQueue GDpptTsQueue;



class TaskResult// : public ITaskResult
{
	TaskResult *next; // 1. "next" pointer in the base class
	
protected:
	vector<string> m_resultNames;
	TaskDb *m_taskDb;
	Disptacher *m_disptacher;
	//CommandDb* m_commandDb;
	AppCommandDb* m_appCmdDb;
	TaskRunQueue* m_taskRunQueue;
	TaskWaitQueue* m_taskWaitQueue;
	TsQueue * m_tsqueue;

public:
	TaskResult();
	friend class TaskState;
	friend class TaskVideoTransCheck;
	
	bool IsThisTaskType(const string resultName);

	virtual Task* PopTask();
	virtual void PunishTsPriority(Task *task){}
	
		
	void setNext(TaskResult *n)
	{
		next = n;
	}
	void add(TaskResult *n)
	{
		if (next)
			next->add(n);
		else
			next = n;
	}
	virtual void SpacificTaskResult(Json::Value& value,TsSession* tsServerSion)
	{
		if(next != NULL)
			next->SpacificTaskResult(value,tsServerSion);
		else {   // this command does not exist;
			printf("error");
			// TODO: log
		}
	}

	virtual void DoTaskRresult(Json::Value& value,TsSession* session);
	

};

class VideoTaskResult : public TaskResult
{
	VideoTaskResult();
public:

	static VideoTaskResult& GetInstance();
	virtual void SpacificTaskResult(Json::Value& value,TsSession* tsServerSion);	
};

class DocTaskResult : public TaskResult
{
	DocTaskResult();
	DocTsType m_docTsType;
public:

	static DocTaskResult& GetInstance();
	virtual void SpacificTaskResult(Json::Value& value,TsSession* tsServerSion);
	void DoTaskRresult(Json::Value& value,TsSession* session);
	DocTsType GetDocTsType(){return m_docTsType;}
	virtual Task* PopTask();
	virtual void PunishTsPriority(Task *task);
};

class DpptTaskResult : public TaskResult
{
	DpptTaskResult();
public:

	static DpptTaskResult& GetInstance();
	virtual void SpacificTaskResult(Json::Value& value,TsSession* tsServerSion);
};




#endif

