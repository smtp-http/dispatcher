#ifndef __TASK_H__
#define __TASK_H__
#include <string>
#include <list>
#include "json/json.h"
#include <vector>
//#include "CommandDb.h"
#include "TaskComm.h"
#include <sys/time.h>
#include <sys/times.h>


using namespace std;

//class Command;
class TransServer;
class Disptacher;
class TsSession;
class AppCommand;


typedef struct 
{

}ErrorInfo;


class TaskState;
class Task 
{
private:
	TaskState* m_taskState;
protected:
	string m_taskId;
	bool m_isUseTs;
	unsigned int m_taskPriority;
	
	Json::Value m_cmdValue;
	
	//Command* m_command;
	AppCommand* m_appCmd;
	TransServer* m_transServer;
	Disptacher* m_disptacher;

	int m_progress;

	unsigned int m_taskBrithTime;
	unsigned int m_taskStartRunTime;


	
public:
	vector<int> m_alreadyTriedTsType;
	vector<string> m_alreadyTriedTsId;

	int m_triedTimes;
	string m_appTargetIp;
	unsigned int m_appTargetPort;

	int m_maxTryTimes;
	TsSession* m_tsSession;
	TaskType m_tasktype;
private:
	//friend class TaskState;
	friend class TaskVideoTransCheck;
	void ChangeState(TaskState *state);//{InitTaskState(state,this);}
	void InitTaskState(TaskState* taskState,Task* task);//{m_taskState = taskState;taskState->SetTask(task);}
public:
	virtual ~Task(){}
	Task()
	{
		//m_command = NULL;
		m_appCmd = NULL;
		m_transServer = NULL;
		m_taskState = NULL;
	}

	void Init()
	{
		m_triedTimes = 0;
		m_taskState = NULL;
	}

	void Release()
	{
		m_alreadyTriedTsType.clear();
		vector<int>(m_alreadyTriedTsType).swap(m_alreadyTriedTsType);

		m_alreadyTriedTsId.clear();
		vector<string>(m_alreadyTriedTsId).swap(m_alreadyTriedTsId);

		delete m_taskState;
	}
	
	unsigned int GetPriority()
	{
		return m_taskPriority;
	}

	void SetDisptacher(Disptacher* disp)
	{
		m_disptacher = disp;
	}

	void SetProgress(int progress)
	{
		m_progress = progress;
	}

	int GetProgress()
	{
		return m_progress;
	}

	void SetPriority(unsigned int priority)
	{
		m_taskPriority = priority;
	}

	string GetTaskId()
	{
		return m_taskId;
	}

	void SetTaskId(string id)
	{
		m_taskId = id;
	}

	void SetTransServer(TransServer * transserver)
	{
		m_transServer = transserver;
	}

	TransServer * GetTransServer()
	{
		return m_transServer;
	}

	//Command* GetCommand()
	//{
	//	return m_command;
	//}
	AppCommand* GetAppCommand(){return m_appCmd;}

	//void SetCommand(Command* cmd)
	//{
	//	m_command = cmd;
	//}
	void SetAppCommand(AppCommand* appCmd);
	//{
	//	m_appCmd = appCmd;
	//	InitTaskState(appCmd->CreatTaskState());
	//}

	void SetCmdValue(Json::Value& value)
	{
		m_cmdValue = value;
	}

	Json::Value& GetCmdValue()
	{
		return m_cmdValue;
	}

	bool IsExceedMaxTryTimes()
	{
		return (this->m_triedTimes >= this->m_maxTryTimes);
	}

	void ResultReport(Json::Value& value);

	void SetTaskBrithTime()
	{
		m_taskBrithTime = (unsigned int)times(NULL);
	}

	unsigned int GetTaskBrithTime()
	{
		return m_taskBrithTime;
	}

	void SetTaskStartRunTime()
	{
		m_taskStartRunTime = (unsigned int)times(NULL);;
	}

	unsigned int GetTaskStartRunTime()
	{
		return m_taskStartRunTime;
	}

	TaskState* GetTaskState()
	{
		return m_taskState;
	}
	
};

class TaskDocument : public Task
{
public:
	TaskDocument();
	TaskDocument(string taskId,unsigned int priority){m_taskId = taskId;m_taskPriority = priority;
		m_appointedServerType = SERVER_NOAPPOINTED;}

	void SetIsSpecific(bool is){m_isSpecificTsType = is;}
	bool GetIsSpecific(){return m_isSpecificTsType;}

	
	void SetAppiontedTS(DocTsType appointedTS)
	{
		m_appointedServerType = appointedTS;
	}

	DocTsType GetAppiontedTS()
	{
		return m_appointedServerType;
	}

	void PunishTsPriority(DocTsType ts_type);
	void SetTsPriority(map<int,string> & ts_priority);
	map<int,DocTsType> & GetTsPriority();
private:
	bool m_isSpecificTsType;
	
	DocTsType m_appointedServerType;

	map<string,DocTsType> ts_typepair;
	
	map<int,DocTsType> m_tsPriority;
};

class TaskVideo : public Task
{
	TaskProperty m_property;
public:
	TaskVideo();
	TaskVideo(string taskId,unsigned int priority){m_taskId = taskId;m_taskPriority = priority;}
};

class TaskDppt : public Task
{
public:
	TaskDppt();
	TaskDppt(string taskId,unsigned int priority){m_taskId = taskId;m_taskPriority = priority;}
};

class TaskNoUseTs : public Task
{
public:
	TaskNoUseTs(){}
	TaskNoUseTs(string taskId,unsigned int priority){m_taskId = taskId;m_taskPriority = priority;}
};



class TaskCmp
{
public:
	bool operator()(Task* a,Task* b)
	{
		if(a->GetPriority() < b->GetPriority())
			return true;
		else if(a->GetPriority() == b->GetPriority())
			return a > b;
		else
			return false;
	}

};

class TaskDb
{
public:
	virtual Task* GetTask() = 0;
	void TaskInit(){}

	void GiveBackTask(Task *task)
	{
		task->Release();
		m_taskBuf.push_back(task);
	}
protected:
	list<Task*> m_taskBuf;
};

class TaskVideoDb : public TaskDb
{
	TaskVideoDb();
public:
	static TaskVideoDb& GetInstance();
	
	virtual Task* GetTask()
	{
		Task * tmp_task;
		if(!m_taskBuf.empty())
		{
			tmp_task = m_taskBuf.front();
			m_taskBuf.pop_front();
		}	
		else
		{
			tmp_task = new TaskVideo;
		}
		return tmp_task;
	}
	
	
};

class TaskDocumentDb : public TaskDb
{
	TaskDocumentDb();
public:
	static TaskDocumentDb& GetInstance();
	virtual Task* GetTask()
	{
		Task * tmp_task;
		if(!m_taskBuf.empty())
		{
			tmp_task = m_taskBuf.front();
			m_taskBuf.pop_front();
		}	
		else
		{
			tmp_task = new TaskDocument;
		}
		
		return tmp_task;
		
	}
};

class TaskDpptDb : public TaskDb
{
	TaskDpptDb();
public:
	static TaskDpptDb& GetInstance();
	virtual Task* GetTask()
	{
		Task * tmp_task;
		if(!m_taskBuf.empty())
		{
			tmp_task = m_taskBuf.front();
			m_taskBuf.pop_front();
		}	
		else
		{
			tmp_task = new TaskDppt;
		}
		
		return tmp_task;
		
	}
};


class TaskNoUseTsDb : public TaskDb
{
	TaskNoUseTsDb();
public:
	static TaskNoUseTsDb& GetInstance();
	virtual Task* GetTask()
	{
		Task * tmp_task;
		if(!m_taskBuf.empty())
		{
			tmp_task = m_taskBuf.front();
			m_taskBuf.pop_front();
		}	
		else
		{
			tmp_task = new TaskNoUseTs;
		}
		
		return tmp_task;
		
	}
};



#endif

