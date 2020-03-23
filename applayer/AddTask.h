#ifndef __ADD_TASK_H__
#define __ADD_TASK_H__
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include "json/json.h"
#include "InterfaceApi.h"
#include "Disptacher.h"
#include "Log4cxxWrapper.h"
#include "Task.h"

using namespace std;

//class CommandDb;
class AppCommandDb;

class AddTask
{
	AddTask *next; // 1. "next" pointer in the base class
	
protected:
	vector<string> m_commandNames;
	TaskDb *m_taskDb;
	Disptacher *m_disptacher;
	//CommandDb* m_commandDb;
	AppCommandDb* m_appCmdDb;

public:
	AddTask();
	
	bool IsThisTaskType(const string cmdName);

	virtual void GetInfo() = 0;

	virtual bool TaskSubmit(Task* task) = 0;
		
	void setNext(AddTask *n)
	{
		next = n;
	}
	void add(AddTask *n)
	{
		if (next)
			next->add(n);
		else
			next = n;
	}
	virtual void AddSpacificTask(Json::Value& value,AppServerSession *appServerSion)
	{
		LoggerWrapper log= LoggerWrapper::GetInstance();
		if(next != NULL)
			next->AddSpacificTask(value,appServerSion);
		else {   // this command does not exist;
			//printf("error");
			log.Log(true,LOGGER_ERROR,"AddSpacificTask next null! [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		}
	}

	Task* BuildBasicTask(Json::Value& value);
	static Task* BuildBasicTask(Json::Value& value,TaskType);
	
	virtual bool DoAppResponse(string commandName,AppServerSession *session,Task* task);

};

class AddVideoTask : public AddTask
{
	AddVideoTask();
public:
	virtual void GetInfo(){}
	virtual bool TaskSubmit(Task*);
	static AddVideoTask& GetInstance();
	virtual void AddSpacificTask(Json::Value& value,AppServerSession *appServerSion);	
};

class AddDocTask : public AddTask
{
	AddDocTask();
public:
	virtual void GetInfo(){}
	virtual bool TaskSubmit(Task*);
	static AddDocTask& GetInstance();
	virtual void AddSpacificTask(Json::Value& value,AppServerSession *appServerSion);
};

class AddDpptTask : public AddTask
{
	AddDpptTask();
public:
	virtual void GetInfo(){}
	virtual bool TaskSubmit(Task*);
	static AddDpptTask& GetInstance();
	virtual void AddSpacificTask(Json::Value& value,AppServerSession *appServerSion);
};

class AddNoUseTsTask : public AddTask
{
	AddNoUseTsTask();
public:
	virtual void GetInfo(){}
	virtual bool TaskSubmit(Task*);
	static AddNoUseTsTask& GetInstance();
	virtual Task* BuildBasicTask(Json::Value& value);
	//virtual bool DoAppResponse(string commandName,AppServerSession *session,Task* task);
	bool DoAppResponse(const Json::Value& appCmd,AppServerSession * session,Task * task);
	virtual void AddSpacificTask(Json::Value& value,AppServerSession *appServerSion);
};

#endif

