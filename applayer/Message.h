#ifndef __MESSAGE_H__
#define __MESSAGE_H__


#include "AddTask.h"
#include "TaskResult.h"
#include "TsUpdate.h"
//#include "TsInterface.h"

class TsSession;

typedef enum{
	TsRegistMsg = 0,
	TsResultMsg,
	TsUpdateMsg,
	TsLinkResponse
}TsMsgType;


class Message
{
public:
	Message(){}
	virtual ~Message(){}
};

class AppMessage : public Message
{
public:
	
};

class TsMessage : public Message
{
protected:
	string m_type;
	TsMsgType m_tsMsgType;
	string m_serverType;
public:
	virtual ~TsMessage(){}
	TsMessage(){}
	TsMsgType GetTsMsgType(){return m_tsMsgType;}

	string GetServerType(){return m_serverType;}
	
};


class TsResultMessage : public TsMessage
{

public:
	TsResultMessage(string serverType)
	{
		m_type = "TransResult";
		m_tsMsgType = TsUpdateMsg;
		m_serverType = serverType;
	}
public:
	//string m_serverType;
};

class TsRegistMessage : public TsMessage
{

public:
	TsRegistMessage(string serverType)
	{
		m_type = "SvrRegister";
		m_tsMsgType = TsRegistMsg;
		m_serverType = serverType;
	}

	bool BuildResponse(int errCode,const string& errorDetial,string& response);
};



class TsUpdateMessage : public TsMessage
{
public:
	TsUpdateMessage(string serverType)
	{
		m_type = "TaskProgress";
		m_tsMsgType = TsUpdateMsg;
		m_serverType = serverType;
	}
};

class TsMessageDb
{
	TsMessageDb();
public:
	static TsMessageDb& GetInstance()
	{
		static TsMessageDb instance_;
		return instance_;
	}
	bool AddMessage(TsMessage *msg);
	bool GetServerTypeInSpecificType(TsMsgType msgType,vector<string>& tsMessages);
	TsMessage* GetMessage(TsMsgType msgType,string serverType);
private:
	vector<TsMessage *> m_tsMessages;
};


class TsMessageHandle
{
	TsMessageHandle* next; 
public:
	TsMessageHandle(){}
	bool IsThisMsg(string type);
	void setNext(TsMessageHandle *n)
	{
		next = n;
	}
	void add(TsMessageHandle *n)
	{
		if (next)
			next->add(n);
		else
			next = n;
	}
	virtual void SpacificHandle(Json::Value& value,TsSession* tsSession)
	{
		if(next != NULL)
			next->SpacificHandle(value,tsSession);
		else {   // this command does not exist;
			printf("error");
			// TODO: log
		}
	}
protected:
	string m_type;
	virtual void BuildAndSendResultResponse(Json::Value& value,TsSession* tsSession);
};

class TsRegistMessageHandle : public TsMessageHandle
{
	ITsRegister* m_register;
	

	TsRegistMessageHandle(ITsRegister* reg)
	{
		m_register = reg;
		m_type = "SvrRegister";
	}
public:
	virtual void SpacificHandle(Json::Value& value,TsSession* tsSession);
	static TsRegistMessageHandle& GetInstance(ITsRegister* reg)
	{
		static  TsRegistMessageHandle instance_(reg);
		return instance_; 	
	}
};

class TsResultMessageHandle : public TsMessageHandle
{
	//ITaskResult* m_taskResult;
	TaskResult* m_taskResult;
	TsResultMessageHandle(/*ITaskResult* tr*/) 
	{
		//m_taskResult = tr;
		m_type = "TransResult";
		m_taskResult = &DocTaskResult::GetInstance();

		VideoTaskResult &videoTaskResult = VideoTaskResult::GetInstance();
		DpptTaskResult &dpptTaskResult = DpptTaskResult::GetInstance();

		m_taskResult->add(&videoTaskResult);
		m_taskResult->add(&dpptTaskResult);
	}
public:
	virtual void SpacificHandle(Json::Value& value,TsSession* tsSession);
	static TsResultMessageHandle& GetInstance()
	{
		static  TsResultMessageHandle instance_;
		return instance_; 	
	}
};

class TsUpdateMessageHandle : public TsMessageHandle
{
	ITsUpdateInfo* m_updataInfo;
	TsUpdateMessageHandle()
	{
		m_type = "TaskProgress";
		m_updataInfo = &TsVideoTransProgress::GetInstance();
	}
public:
	virtual void SpacificHandle(Json::Value& value,TsSession* tsSession);
	static TsUpdateMessageHandle& GetInstance()
	{
		static  TsUpdateMessageHandle instance_;
		return instance_; 	
	}
};

class MessageAnalyzer
{
public:
	
};

class TsMessageAnalyzer : public MessageAnalyzer,public ITsNewFrame
{
	TsMessageAnalyzer();
	//TaskResult* m_taskResult;
	TsMessageHandle* m_messageHandle;
public:
	~TsMessageAnalyzer();
	static TsMessageAnalyzer& GetInstance();
	virtual void OnNewFrame(TsSession* tsServerSion,const string frame);
};

class AppMessageAnalyzer : public MessageAnalyzer,public IAppNewFrame
{
	AppMessageAnalyzer();
	AddTask* m_addTask;
public:
	~AppMessageAnalyzer();
	static AppMessageAnalyzer& GetInstance();
	virtual void OnNewFrame(AppServerSession *appServerSion,string frame);
};


class testITsRegister : public ITsRegister
{
public:
	virtual void OnRegister(Json::Value& value,TsSession *session){}
};

#endif
