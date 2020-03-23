#ifndef __TASK_STATUS_MAP_H__
#define __TASK_STATUS_MAP_H__

#include <map>
#include <string> 
#include <string.h>
#include <time.h>
#include "TaskComm.h"

using namespace std;

#define TASK_PROGRESS_TIMEOUT (100*60)

class TaskStatus{
	string m_taskId;
	string m_taskType;
	unsigned int m_timeFlag;
	string m_progress;
public:
	TaskStatus(string taskId,string taskType)
		:m_taskId(taskId)
		,m_taskType(taskType)
	{
		m_timeFlag = (unsigned int)time(NULL);
	}

	~TaskStatus();


	void SetProgress(string progress);
	string GetTaskType();
	string GetTaskId();
	string GetProgress();
	unsigned int GetTimeFlag();
};

class TaskStatusMap{

	map<string,TaskStatus *> m_taskStatusMap;
	
public:
	TaskStatusMap();
	bool TaskStatusInsert(TaskStatus *);
	bool TaskStatusDel(TaskStatus *);
	bool TaskStatusDel(string,string);
	bool TaskStatusMapTraversal();
	bool TaskStatusMapTimeoutTraversal();
	bool TaskProgressUpdate(const string &taskId,const string &taskType,const string &progress);
	//bool TaskStatusUpdate(TaskStatus *);
	TaskStatus *GetTaskStatusInMap(string taskId,string taskType);
};


#endif

