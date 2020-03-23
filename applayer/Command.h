#ifndef __COMMAND_H__
#define __COMMAND_H__

#include <string>
#include "json/json.h"
#include "TsManager.h"
//#include "TaskComm.h"
//#include "TransServer.h"

#define _common_build_app_result(appResult,tsResult,code)  { \
	appResult["id"] = tsResult["id"]; \
	if(!tsResult["errorcode"].isNull() && tsResult["errorcode"].isString()){ \
		code = atoi(tsResult["errorcode"].asString().c_str()); \
		appResult["code"] = code; \
	} else { \
		dsLog.Log(false,LOGGER_ERROR,"error message from ts result. No \"errorcode\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__); \
		return false; \
	} \
	if(tsResult["result"].asString() == "Success"){ \
		appResult["status"] = "ture"; \
	} else { \
		appResult["status"] = "false"; \
	} \
}

class TsCommand;
class Task;
class TaskResult;

class TaskState
{

public:
	TaskState(){}
	TaskState(TsCommand* tsCmd){m_tsCmd = tsCmd;}
	virtual bool Handle(Json::Value& value,TaskResult* taskResult);

protected:
	void ChangeState(Task *task,TaskState *state);
	Task* GetTask(){return m_task;}
protected:
	TsCommand* m_tsCmd;
public:
	TsCommand* GetTsCommand(){return m_tsCmd;}
	void SetTsCommand(TsCommand* tsCmd){m_tsCmd = tsCmd;}
	friend class Task;
private:
	void SetTask(Task* task){m_task = task;}
	
	Task* m_task;
};

/*
class TaskDocTransState : public TaskState
{
public:
	TaskDocTransState(){}
	virtual bool Handle(Json::Value& value,TaskResult* taskResult);
};
*/


//class TaskVideoTransState : public TaskState
//{

//public:
//	TaskVideoTransState(){}
//	TaskVideoTransState(TsCommand* tsCmd){m_tsCmd = tsCmd;}
//	virtual bool Handle(Json::Value& value,TaskResult* taskResult);
//};

class TaskVideoTransCheck : public TaskState// : public TaskVideoTransState
{
public:
	TaskVideoTransCheck();
	//TaskVideoTransCheck(TsCommand* tsCmd){m_tsCmd = tsCmd;}
	virtual bool Handle(Json::Value& value,TaskResult* taskResult);
	
};

class TaskVideoTrans : public TaskState//TaskVideoTransState
{
public:
	TaskVideoTrans();
	//virtual bool Handle(Json::Value& value,TaskResult* taskResult);
};

class command 
{
public:
	command();
	~command();
	
};



class AppCommand : public command
{
protected:
	string m_commandName;

	//TaskProperty m_tsProperty;
	//TsType m_tsType;
	TaskType m_taskType;
	bool m_isProvidedResult;

	TsCommand* m_defaultTsCmd;
	
public:
	virtual TaskState* CreatTaskState();// {return new TaskState(m_defaultTsCmd);}
	virtual void DestoryTaskState(TaskState* taskState) {delete taskState;}
	string GetCommandName(){ return m_commandName;}
	AppCommand();
	~AppCommand();
	virtual bool BuildAppResult(const Json::Value& appCmd,string& appResultStr){}// = 0;
	bool  BuildAppResponse(Json::Value& appResponse,unsigned int errorCode,string ErrorDetail); 
	virtual bool  BuildAppResponse(const Json::Value& appCmd,string&){}
	bool IsProvidedResult(){return m_isProvidedResult;}
	//TaskProperty GetTaskProperty(){return m_tsProperty;}
	TaskType GetTaskType(){return m_taskType;}
};

class TsCommand : public command
{
protected:
	TsType m_tsType;
	TaskProperty m_tsProperty;
	string m_tsResultName;
	bool CommBuildTsCmd(const Json::Value& appCmd,Json::Value& tscmd);
public:
	virtual bool BuildTsCmd(const Json::Value& appCmd,string& tsCmd) = 0;
	string GetCommandName(){ return m_tsResultName;}
	TaskProperty GetTaskProperty(){return m_tsProperty;}
	TsType GetTsType(){return m_tsType;}
	string GetServerType(){return m_tsResultName;}
};

///////////////////////////////// specific command //////////////////////////////////////
class AppCmdServerStatus : public AppCommand
{
public:
	AppCmdServerStatus();
	
	~AppCmdServerStatus(){}

	virtual bool BuildAppResponse(const Json::Value& appCmd,string& appResultStr);


};

class AppCmdVideoMessage : public AppCommand
{
public:
	AppCmdVideoMessage();
	
	~AppCmdVideoMessage(){}


	virtual bool BuildAppResult(const Json::Value& tsResult,string& appResult);

};


class AppCmdVideoTrans : public AppCommand
{
public:
	AppCmdVideoTrans();

	~AppCmdVideoTrans(){}

	virtual bool BuildAppResult(const Json::Value& tsResult,string& appResult);

	virtual TaskState* CreatTaskState(){return new TaskVideoTransCheck();}
	//virtual void DestoryTaskState(TaskState* taskState){delete taskState;}
};

class AppCmdPrintScreen : public AppCommand
{
public:
	AppCmdPrintScreen();
	~AppCmdPrintScreen(){}

	virtual bool BuildAppResult(const Json::Value& tsResult,string& appResultStr);

};


class AppCmdVideoCut : public AppCommand
{
public:
	AppCmdVideoCut();

	virtual bool BuildAppResult(const Json::Value& tsResult,string& appResult);

};



class AppCmdVideoMerge : public AppCommand
{
public:
	AppCmdVideoMerge();

	virtual bool BuildAppResult(const Json::Value& tsResult,string& appResult);

};


class AppCmdDocTrans : public AppCommand
{
public:
	AppCmdDocTrans();
	static bool GetServerTypes(vector<string>& CommandNames)
	{
		CommandNames.push_back("DOC_WPS");
		CommandNames.push_back("DOC_WINDOWS");
		CommandNames.push_back("DOC");

		return true;
	}

	virtual bool BuildAppResult(const Json::Value& tsResult,string& appResult);
	//virtual void DestoryTaskState(TaskState* taskState){delete taskState;}

};


class AppCmdDynamicPptTrans : public AppCommand
{
public:
	AppCmdDynamicPptTrans();
	static void GetServerTypes(vector<string>& CommandNames)
	{
		CommandNames.push_back("DYNAMIC_PPT");
	}

	virtual bool BuildAppResult(const Json::Value& tsResult,string& appResultStr);

};


class AppCmdTaskProgress : public AppCommand
{
public:
	AppCmdTaskProgress()
	{
		m_commandName = "task_progress_request";
	
		m_taskType = NoTsTask;
		
	}

	virtual bool  BuildAppResponse(const Json::Value& appCmd,string& response);

};

class AppCmdAudioTrans : public AppCommand
{
public:
	AppCmdAudioTrans();

	virtual bool BuildAppResult(const Json::Value& tsResult,string& appResult);

};

class AppCmdAudioMessage : public AppCommand
{
public:
	AppCmdAudioMessage();

	~AppCmdAudioMessage(){}


	virtual bool BuildAppResult(const Json::Value& tsResult,string& appResult);

};


///////////////////// ts command ////////////////////////////////

class TsCmdVideoMessage : public TsCommand
{
public:
	TsCmdVideoMessage()
	{
		m_tsType = TS_VIDEO;
		m_tsResultName = "Message";
		m_tsProperty = TASK_LIGHT;
	}
	virtual bool  BuildTsCmd(const Json::Value& appCmd,string& tscmd);
};


class TsCmdVideoTrans : public TsCommand
{
public:
	TsCmdVideoTrans()
	{
		m_tsType = TS_VIDEO;
		m_tsResultName = "Transfer";
		m_tsProperty = TASK_HEAVY;
	}

	virtual bool  BuildTsCmd(const Json::Value& appCmd,string& tscmd);
};


class TsCmdVideoRequest : public TsCommand
{
public:
	TsCmdVideoRequest()
	{
		m_tsType = TS_VIDEO;
		m_tsResultName = "VideoTransRequest";
		m_tsProperty = TASK_LIGHT;
	}

	virtual bool  BuildTsCmd(const Json::Value& appCmd,string& tscmd);
};
	


class TsCmdPrintScreen : public TsCommand
{
public:
	TsCmdPrintScreen()
	{
		m_tsType = TS_VIDEO;
		m_tsResultName = "CapVideo";
		m_tsProperty = TASK_LIGHT;
	}

	virtual bool  BuildTsCmd(const Json::Value& appCmd,string& tscmd);
};


class TsCmdVideoCut : public TsCommand
{
public:
	TsCmdVideoCut()
	{
		m_tsType = TS_VIDEO;
		m_tsResultName = "VideoCut";
		m_tsProperty = TASK_LIGHT;
	}

	virtual bool  BuildTsCmd(const Json::Value& appCmd,string& tscmd);
};


class TsCmdVideoMerge : public TsCommand
{
public:
	TsCmdVideoMerge()
	{
		m_tsType = TS_VIDEO;
		m_tsResultName = "VideoMerge";
		m_tsProperty = TASK_HEAVY;
	}
	virtual bool  BuildTsCmd(const Json::Value& appCmd,string& tscmd);
};


class TsCmdDocTrans : public TsCommand
{
public:
	TsCmdDocTrans()
	{
		//m_tsResultName = 
		m_tsType = TS_DOC;
	}

	virtual bool  BuildTsCmd(const Json::Value& appCmd,string& tscmd);
};


class TsCmdDynamicPptTrans : public TsCommand
{
public:
	TsCmdDynamicPptTrans(){m_tsType = TS_DYNAMIC_PPT;}

	virtual bool  BuildTsCmd(const Json::Value& appCmd,string& tscmd);
};


class TsCmdAudioTrans : public TsCommand
{
public:
	TsCmdAudioTrans()
	{
		m_tsType = TS_VIDEO;
		m_tsResultName = "AudioTrans";
		m_tsProperty = TASK_HEAVY;
	}


	virtual bool  BuildTsCmd(const Json::Value& appCmd,string& tscmd);
};

class TsCmdAudioMessage : public TsCommand
{
public:
	TsCmdAudioMessage()
	{
		m_tsType = TS_VIDEO;
		m_tsResultName =  "AudioMessage";
	}

	virtual bool  BuildTsCmd(const Json::Value& appCmd,string& tscmd);
};

/////////////////////////////////  AppCommandDb  ///////////////////////////////////////

class AppCommandDb
{
	AppCommandDb();
public:
	static AppCommandDb& GetInstance()
	{
		static AppCommandDb instance_;
		return instance_;
	}
	
	~AppCommandDb();
	bool AddAppCommand(AppCommand* cmd);
	bool GetCommandNameInSpecificTaskType(TaskType taskType,vector<string>& CommandNames);
	bool GetResultNameInSpecificTaskType(TaskType taskType,vector<string>& CommandNames);
	AppCommand* GetAppCommand(string cmdName);
private:
	map<string,AppCommand*> m_commands;
};


/////////////////////////////////  TsCommandDb  ///////////////////////////////////////

class TsCommandDb
{
	TsCommandDb();
public:
	static TsCommandDb& GetInstance()
	{
		static TsCommandDb instance_;
		return instance_;
	}
	
	~TsCommandDb();
	bool AddTsCommand(TsCommand* cmd);
//	bool GetCommandNameInSpecificTaskType(TaskType taskType,vector<string>& CommandNames);
//	bool GetResultNameInSpecificTaskType(TaskType taskType,vector<string>& CommandNames);
	TsCommand* GetCommand(string cmdName);
	friend class AppCommandDb;
private:
	map<string,TsCommand*> m_commands;
};


#endif

