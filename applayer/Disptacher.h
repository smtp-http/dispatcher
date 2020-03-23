#ifndef __DISPTACHER_H__
#define __DISPTACHER_H__

#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include "json/json.h"
//#include "InterfaceApi.h"
#include "TsQueue.h"
#include "TaskQueue.h"


class Disptacher {
public:
	Disptacher(){}

	bool DuplicateTaskCheck(string id)
	{
		return !(m_taskRunQueue->CheckSameTaskId(id) || m_taskWaitQueue->CheckSameTaskId(id));
	}

	virtual bool Submit(Task *task) = 0;

protected:
	virtual bool SubmitTaskToSpecTs(Task*,TransServer *ts);
	
	TaskWaitQueue* m_taskWaitQueue;
	TaskRunQueue* m_taskRunQueue;
	map<string,TsQueue*>m_tsQueues;
	//vector<string> m_commandNames;

};

class DocDisptacher : public Disptacher
{
	DocDisptacher();
public:
	map<int,string> m_tsPriority;
	
	static DocDisptacher& GetInstance();
	virtual bool Submit(Task *task);

	bool GetDocTsPriority();
};

class VideoDisptacher : public Disptacher
{
	VideoDisptacher();
public:
	static VideoDisptacher& GetInstance();
	virtual bool Submit(Task *task);
};

class DpptDisptacher : public Disptacher
{
	DpptDisptacher();
public:
	static DpptDisptacher& GetInstance();
	virtual bool Submit(Task *task);
};

#endif

