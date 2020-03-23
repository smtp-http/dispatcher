#ifndef __TS_MANAGER_H__
#define __TS_MANAGER_H__

#include "Message.h"
#include "TsInterface.h"

extern int tsLeave_times;

typedef enum
{

	TS_DOCUMENT_LINUX = 0,

	TS_DOCUMENT_OFFICE = 1,

	TS_DOCUMENT_WPS = 2,

	TS_VIDEO = 3,

	TS_DYNAMIC_PPT =4,

	TS_DOC = 5

} TsType;

/*
typedef enum
{
	TS_HEAVY = 0,
	TS_LIGHT = 1
}TsProperty;
*/

class AddTransServer;

class TsRegister : public ITsRegister
{
	AddTransServer *m_addTransServer;
public:
	TsRegister();
	virtual void OnRegister(Json::Value& value,TsSession *session);
};

class AddTransServer
{
	AddTransServer *next;
protected:
	TsQueue *m_tsQueue;
	string m_serverType;
	TransServerDb *m_tsDb;
	TaskWaitQueue * m_taskWaitQueue;
	Disptacher *m_disptacher;

public:
	AddTransServer();
	virtual void AddTsTransaction(Json::Value& value,TsSession*);
	void setNext(AddTransServer *n)
	{
		next = n;
	}
	void add(AddTransServer *n)
	{
		if (next)
			next->add(n);
		else
			next = n;
	}
	virtual void AddSpacificTransServer(Json::Value& value,TsSession*session)
	{
		if(next != NULL)
			next->AddSpacificTransServer(value,session);
		else {   // this command does not exist;
			printf("error");
			// TODO: log
		}
	}

};

class AddDocTransServer : public AddTransServer
{
	map<string,DocTsType> m_docTsTypeMap;
public:
	AddDocTransServer();
	//static AddDocTransServer& GetInstance();
	virtual void AddSpacificTransServer(Json::Value& value,TsSession*TsSession);
	virtual void AddTsTransaction(Json::Value & value, TsSession* session);
};

class AddOpenofficeTransServer : public AddDocTransServer
{
	AddOpenofficeTransServer();
public:
	static AddOpenofficeTransServer& GetInstance();
	virtual void AddSpacificTransServer(Json::Value& value,TsSession*TsSession);
};


class AddWpsTransServer : public AddDocTransServer
{
	AddWpsTransServer();
public:
	static AddWpsTransServer& GetInstance();
	virtual void AddSpacificTransServer(Json::Value& value,TsSession*TsSession);
};

class AddOfficeTransServer : public AddDocTransServer
{
	AddOfficeTransServer();
public:
	static AddOfficeTransServer& GetInstance();
	virtual void AddSpacificTransServer(Json::Value& value,TsSession*TsSession);
};


class AddVideoTransServer : public AddTransServer
{
	AddVideoTransServer();
public:
	static AddVideoTransServer& GetInstance();
	virtual void AddTsTransaction(Json::Value& value,TsSession*);
	virtual void AddSpacificTransServer(Json::Value& value,TsSession*TsSession);
};


class AddDpptTransServer : public AddTransServer
{
	AddDpptTransServer();
public:
	static AddDpptTransServer& GetInstance();
	virtual void AddSpacificTransServer(Json::Value& value,TsSession*TsSession);
};

class TsLeaveHandler;

class TransServerLeave : public ITsServerLeave
{
	TransServerLeave();
	TsLeaveHandler* m_tsLeaveHandler;
public:
	static TransServerLeave& GetInstance();
	virtual void OnServerLeave(TsSession &session);
};

class TsLeaveHandler
{
	TsLeaveHandler* next;
protected:
	TsQueue* m_tsQueue;
	TransServerDb* m_tsDb;
	Disptacher* m_disptacher;
	TaskRunQueue * m_taskRunQueue;
	TaskWaitQueue *m_taskWaitQueue;
public:
	TransServer* FindSession(TsSession* session);
	void setNext(TsLeaveHandler *n)
	{
		next = n;
	}
	void add(TsLeaveHandler *n)
	{
		if (next)
			next->add(n);
		else
			next = n;
	}
	virtual void SpacificTsLeave(TsSession* TsSession)
	{
		if(next != NULL)
			next->SpacificTsLeave(TsSession);
		else {   // this command does not exist;
			printf("error");
			// TODO: log
		}
	}

	void ProcessTsAndTask(TransServer *ts)
	{
		LoggerWrapper dsLog= LoggerWrapper::GetInstance();
		if(DocTrans == ts->GetServerType())
		{
			tsLeave_times +=1;
		}
		
		Task* task;
		
		task = ts->GetTask();

		if(task)
		{
			dsLog.Log(true,LOGGER_ERROR,"%s:%s:%d  ts leave id: %s , task id: %s\n",__FILE__,__PRETTY_FUNCTION__,__LINE__,(ts->GetId()).c_str(),(task->GetTaskId()).c_str());
		}
		m_tsQueue->DelTs(ts);
		//ts->SetState(TsIdle);
		//ts->SetSession(NULL);
		//ts->SetTask(NULL);
		ts->Release();

		/*
		if(m_tsDb->GiveBackTransServer(ts)){

		} else {
			// TODO: log .  Duplicate ts
		}
		*/
		if(NULL != task)
		{	
			m_taskRunQueue->DelTask(task->GetTaskId());
			task->SetPriority((task->GetPriority())+61);
			//m_taskWaitQueue->PushTask(task);
			m_disptacher->Submit(task);
		}
	}

};

class TsDocLeaveHandler : public TsLeaveHandler
{
public:
	TsDocLeaveHandler();
};

class TsOpenofficeLeaveHandler : public TsDocLeaveHandler
{
	TsOpenofficeLeaveHandler();
public:
	static TsOpenofficeLeaveHandler& GetInstance();

	virtual void SpacificTsLeave(TsSession* session);
};

class TsWpsLeaveHandler : public TsDocLeaveHandler
{
	TsWpsLeaveHandler();
public:
	static TsWpsLeaveHandler& GetInstance();

	virtual void SpacificTsLeave(TsSession* session);
};

class TsOfficeLeaveHandler : public TsDocLeaveHandler
{
	TsOfficeLeaveHandler();
public:
	static TsOfficeLeaveHandler& GetInstance();

	virtual void SpacificTsLeave(TsSession* session);
};


class TsVideoLeaveHandler : public TsLeaveHandler
{
	TsVideoLeaveHandler();
public:
	static TsVideoLeaveHandler& GetInstance();

	virtual void SpacificTsLeave(TsSession* session);
};



class TsDpptLeaveHandler : public TsLeaveHandler
{
	TsDpptLeaveHandler();
public:
	static TsDpptLeaveHandler& GetInstance();

	virtual void SpacificTsLeave(TsSession* session);
};


#endif

