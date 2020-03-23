#ifndef __TASK_QUEUE_H__
#define __TASK_QUEUE_H__
#include <queue>  
#include <list>
#include "Task.h"
#include "TaskComm.h"
#include "IDataPersistence.h"

//class CommandDb;
class AppCommandDb;
typedef priority_queue<Task*,vector<Task*>,TaskCmp> TaskPriorityQueue;


class TaskQueue
{
protected:
	IDataPresistence *m_dataPresistence;
public:
	void SetDataPresistence(IDataPresistence *dataPresistence){m_dataPresistence = dataPresistence;}
};

class TaskWaitQueue : public TaskQueue
{
	TaskType m_taskType;
	
public:
	TaskWaitQueue(){}
	TaskWaitQueue(TaskType type);//
	Task* PopTask();

	Task* PopRightTaskForDocTs(DocTsType tsType);

	Task* GetTaskInSpecId(string taskId);
	Task* GetTaskInSpecId(string taskId,unsigned int &position);
	
	bool PushTask(Task *task);

	bool CheckSameTaskId(string TaskId);

	TaskPriorityQueue& GetTaskList();
private:
	TaskPriorityQueue m_queue;
};


class TaskRunQueue : public TaskQueue
{
	TaskType m_taskType;
	
public:
	TaskRunQueue(TaskType type);
	bool PushTask(Task *task,TsSession *session);
	list<Task *>& GetTaskList();
	Task* GetTaskInSpecId(string taskId);
	Task* GetTaskInSession(TsSession* session);
	bool DelTask(string taskId);
	bool CheckSameTaskId(string TaskId);
	unsigned int GetTaskNum()
	{
		return m_taskList.size();
	}
private:
	list<Task *> m_taskList;
};

class TaskQueueManager
{
	TaskQueueManager();
public:
	static TaskQueueManager& GetInstance();
	TaskRunQueue* GetRunQueue(string serverType);
	TaskWaitQueue* GetWaitQueue(string serverType);
private:
	map<string,TaskRunQueue *> m_taskRunQList;
	map<string,TaskWaitQueue *> m_taskWaitQList;
	//CommandDb* m_commandDb;
	AppCommandDb* m_appCmdDb;
};


#endif
