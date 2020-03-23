#ifndef __TS_UPDATE_H__
#define __TS_UPDATE_H__

#include "stdio.h"
#include "TsInterface.h"
#include "TaskQueue.h"

class TsUpdate : public ITsUpdateInfo
{
	TsUpdate* next;
protected:
	string m_taskType;
	TaskRunQueue* m_taskRunQueue;
public:
	bool isThis(string taskType)
	{
		if(m_taskType == taskType)
			return true;
		else
			return false;
	}
	void setNext(TsUpdate *n)
	{
		next = n;
	}
	void add(TsUpdate *n)
	{
		if (next)
			next->add(n);
		else
			next = n;
	}
	
	virtual void OnUpdateInfo(Json::Value& value,TsSession *session)
	{
		if(next != NULL)
			next->OnUpdateInfo(value,session);
		else {  
			printf("error");
			// TODO: log
		}
	}
};

class TsVideoTransProgress : public TsUpdate
{
	TsVideoTransProgress();
public:
	static TsVideoTransProgress& GetInstance();
	virtual void OnUpdateInfo(Json::Value& value,TsSession *session);
};




#endif
