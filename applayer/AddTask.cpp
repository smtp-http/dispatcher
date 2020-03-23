
//#include "CommandDb.h"
#include "Command.h"
#include "AddTask.h"
#include "TsManager.h"
#include "error.h"


using namespace std;
using namespace cdy;

//extern CommandDb GCommandDb;

//extern TaskDocumentDb GTaskDocumentDb;
//extern TaskVideoDb GTaskVideoDb;
//extern TaskDpptDb GTaskDpptDb;
//extern TaskNoUseTsDb GTaskNoUseTsDb;


//==========================  AddTask ===========================

AddTask::AddTask()
{
	next = NULL; 
	//m_commandDb = &CommandDb::GetInstance();//&GCommandDb;
	m_appCmdDb = &AppCommandDb::GetInstance();
}

bool AddTask::IsThisTaskType(const string cmdName)
{
	vector<string>::iterator it;
	
	for (it = m_commandNames.begin(); it != m_commandNames.end(); ++ it){
		if(cmdName == *it)
			return true;
	}

	return false;
}

Task* AddTask::BuildBasicTask(Json::Value& value)
{

	Task *task;
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();

	//dsLog.Log(false,LOGGER_DEBUG,"[%s][%s][%d] build basic task: %s\n",__FILE__,__PRETTY_FUNCTION__,__LINE__,value["command"].asString().c_str());  

	task = m_taskDb->GetTask();
	
	if(!value["priority"].isNull() && value["priority"].isIntegral()){
		task->SetPriority(value["priority"].asInt());
	} else {
		dsLog.Log(false,LOGGER_ERROR,"[%s][%s][%d] error \"priority\" \n",__FILE__,__PRETTY_FUNCTION__,__LINE__);  
		return NULL;
	}

	if(!value["id"].isNull() && value["id"].isString()){
		task->SetTaskId(value["id"].asString());
	} else {
		dsLog.Log(false,LOGGER_ERROR,"[%s][%s][%d] error \"id\" \n",__FILE__,__PRETTY_FUNCTION__,__LINE__);  
		return NULL;
	}

	task->Init();
	task->SetCmdValue(value);
	task->SetDisptacher(m_disptacher);
	//task->SetCommand(m_commandDb->GetCommand(value["command"].asString()));
	task->SetAppCommand(m_appCmdDb->GetAppCommand(value["command"].asString()));
	
	

	if(!value["host"].isNull() && value["host"].isString()){
		task->m_appTargetIp = value["host"].asString();            
	} else {
		dsLog.Log(false,LOGGER_ERROR,"[%s][%s][%d] error \"host\" \n",__FILE__,__PRETTY_FUNCTION__,__LINE__);  
		return NULL;            
	}   

	if(!value["port"].isNull() && value["port"].isIntegral()){
		task->m_appTargetPort = value["port"].asInt();             
	} else {
		dsLog.Log(false,LOGGER_ERROR,"[%s][%s][%d] error \"port\" \n",__FILE__,__PRETTY_FUNCTION__,__LINE__);                                               
		return NULL; 
	}

	//set task brith time
    task->SetTaskBrithTime();
	
	return task;
}

Task* AddTask::BuildBasicTask(Json::Value& value,TaskType type)
{
	Task *task;
	TaskDb *taskDb;
	//CommandDb *commandDb = &CommandDb::GetInstance();
	AppCommandDb *appCmdDb = &AppCommandDb::GetInstance();
	Disptacher *disptacher;

	switch(type){
		case DocTask:
			taskDb = &TaskDocumentDb::GetInstance();
			disptacher = &DocDisptacher::GetInstance();
			break;
		case DpptTask:
			taskDb = &TaskDpptDb::GetInstance();
			disptacher = &DpptDisptacher::GetInstance();
			break;
		case VideoTask:
			taskDb = &TaskVideoDb::GetInstance();
			disptacher = &VideoDisptacher::GetInstance();
			break;
		case NoTsTask:
			
			break;
		default:

			break;
	}

	task = taskDb->GetTask();
		
	if(!value["priority"].isNull() && value["priority"].isIntegral()){
		task->SetPriority(value["priority"].asInt());
	} else {
		// TODO: LOG
		return NULL;
	}

	if(!value["id"].isNull() && value["id"].isString()){
		task->SetTaskId(value["id"].asString());
	} else {
		// TODO: LOG
		return NULL;
	}

	task->Init();
	task->SetCmdValue(value);
	task->SetDisptacher(disptacher);
	//task->SetCommand(commandDb->GetCommand(value["command"].asString()));
	task->SetAppCommand(appCmdDb->GetAppCommand(value["command"].asString()));
	

	if(!value["host"].isNull() && value["host"].isString()){
		task->m_appTargetIp = value["host"].asString();            
	} else {
		// TODO: log
		return NULL;            
	}   

	if(!value["port"].isNull() && value["port"].isIntegral()){
		task->m_appTargetPort = value["port"].asInt();             
	} else {
		//dsLog.Log(FALSE,LOGGER_ERROR,"Error message from application client. No port. :%s, [%s][%s][%d]\n",strRecv.c_str(),__FILE__,__PRETTY_FUNCTION__,__LINE__);                                               
		return NULL; 
	}

	return task;
}


bool AddTask::DoAppResponse(string commandName,AppServerSession *session,Task* task)
{
	string msg;
	//Command *cmd;
	AppCommand *appCmd;
	Json::Value valueAppRes;
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();

	
	appCmd = m_appCmdDb->GetAppCommand(commandName);
	
	if(!appCmd) 
		return false;
	
	if(m_disptacher->DuplicateTaskCheck(task->GetTaskId())){
		appCmd->BuildAppResponse(valueAppRes,0,"");
	} else {
		appCmd->BuildAppResponse(valueAppRes,RET_E_REPEATED_TS_TRANS_TASK,"App task message id is repeated!");
		msg = valueAppRes.toStyledString() + '\0';
		cout << "---- response: " << msg << endl;
		session->SendMessage(msg);
		dsLog.Log(true,LOGGER_ERROR,"%s:%s:%d   App task message id is repeated!\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	
	msg = valueAppRes.toStyledString() + '\0';
	cout << "----2 response: " << msg << endl;
	session->SendMessage(msg);
	
	return true;
}

//========================  AddVideoTask =========================
AddVideoTask& AddVideoTask::GetInstance()
{
	static  AddVideoTask instance_;
	return instance_; 	
}


AddVideoTask::AddVideoTask()
{
	//m_commandDb->GetCommandNameInSpecificTaskType(VideoTask,m_commandNames);
	m_appCmdDb->GetCommandNameInSpecificTaskType(VideoTask,m_commandNames);
	
	m_taskDb = &TaskVideoDb::GetInstance();//GTaskVideoDb;
	m_disptacher = &VideoDisptacher::GetInstance();//&GVideoDisptacher;
}

void AddVideoTask::AddSpacificTask(Json::Value& value,AppServerSession *appServerSion)
{
	Task* task;

	if(IsThisTaskType(value["command"].asString())) {
		task = BuildBasicTask(value);
		if(!task) 
			return;
		
		task->m_tasktype = VideoTask;
		if(!DoAppResponse(value["command"].asString(),appServerSion,task))
			return;

		if(!TaskSubmit(task)){
			
		}

		return;
		
	} else {
		AddTask::AddSpacificTask(value,appServerSion);
	}
}

bool AddVideoTask::TaskSubmit(Task* task)
{
	return m_disptacher->Submit(task);
}





//=========================  AddDocTask ==========================

AddDocTask& AddDocTask::GetInstance()
{
	static  AddDocTask instance_;
	return instance_; 	
}

AddDocTask::AddDocTask()
{
	//m_commandDb->GetCommandNameInSpecificTaskType(DocTask,m_commandNames);
	m_appCmdDb->GetCommandNameInSpecificTaskType(DocTask,m_commandNames);
	
	m_taskDb = &TaskDocumentDb::GetInstance();//&GTaskDocumentDb;

	m_disptacher = &DocDisptacher::GetInstance();//GDocDisptacher;
}

void AddDocTask::AddSpacificTask(Json::Value& value,AppServerSession *appServerSion)
{
	Task* task;
	string doc_server_type;
	
	if(IsThisTaskType(value["command"].asString())) {
		task = BuildBasicTask(value);
		if(!task) 
			return;
        
        task->m_tasktype = DocTask;
		TaskDocument* td = dynamic_cast<TaskDocument*>(task);
        td->SetTsPriority((DocDisptacher::GetInstance()).m_tsPriority);
		
		if(!value["docServerType"].isNull()&&value["docServerType"].isString()){
            doc_server_type = value["docServerType"].asString();
            cout<<"SET appiontedts :"<<doc_server_type<<endl;
			if(doc_server_type == "LINUX" )
                td->SetAppiontedTS(SERVER_LINUX);
            else if(doc_server_type == "WPS")
                td->SetAppiontedTS(SERVER_WPS);
            else if(doc_server_type == "OFFICE" )
                td->SetAppiontedTS(SERVER_OFFICE);
			td->SetIsSpecific(true);
		} else {
		    cout<<"SET appiontedts noappoined"<<endl;
		    td->SetAppiontedTS(SERVER_NOAPPOINTED);
			td->SetIsSpecific(false);
		}
		
		if(!DoAppResponse(value["command"].asString(),appServerSion,task))
			return;

		if(!TaskSubmit(task)){
			
		}

		return;
	} else {
		AddTask::AddSpacificTask(value,appServerSion);
	}
}

bool AddDocTask::TaskSubmit(Task* task)
{
	return m_disptacher->Submit(task);
}


//========================  AddDpptTask ==========================
AddDpptTask& AddDpptTask::GetInstance()
{
	static  AddDpptTask instance_;
	return instance_; 	
}

AddDpptTask::AddDpptTask()
{
	//m_commandDb->GetCommandNameInSpecificTaskType(DpptTask,m_commandNames);
	m_appCmdDb->GetCommandNameInSpecificTaskType(DpptTask,m_commandNames);
	
	m_taskDb = &TaskDpptDb::GetInstance();//&GTaskDpptDb;

	m_disptacher = &DpptDisptacher::GetInstance();//&GDpptDisptacher;
}

void AddDpptTask::AddSpacificTask(Json::Value& value,AppServerSession *appServerSion)
{
	Task *task;
	
	if(IsThisTaskType(value["command"].asString())){
		task = BuildBasicTask(value);
		if(!task) 
			return;
		
        task->m_tasktype = DpptTask;
		if(!DoAppResponse(value["command"].asString(),appServerSion,task))
			return;

		if(!TaskSubmit(task)){
			
		}

		return;
	} else {
		AddTask::AddSpacificTask(value,appServerSion);
	}
}

bool AddDpptTask::TaskSubmit(Task* task)
{
	return m_disptacher->Submit(task);
}



//====================== AddNoUseTsTask ===========================
AddNoUseTsTask& AddNoUseTsTask::GetInstance()
{
	static  AddNoUseTsTask instance_;
	return instance_; 	
}

AddNoUseTsTask::AddNoUseTsTask()
{
	//m_commandDb->GetCommandNameInSpecificTaskType(NoTsTask,m_commandNames);
	m_appCmdDb->GetCommandNameInSpecificTaskType(NoTsTask,m_commandNames);
	m_taskDb = &TaskNoUseTsDb::GetInstance();//&GTaskNoUseTsDb;
	
}

Task* AddNoUseTsTask::BuildBasicTask(Json::Value& value)
{
	Task *task;
	cout << "++++++AddNoUseTsTask:: BuildBasicTask " << endl;
	task = m_taskDb->GetTask();
		
	
	task->Init();
	task->SetCmdValue(value);
	task->SetDisptacher(m_disptacher);
	//task->SetCommand(m_commandDb->GetCommand(value["command"].asString()));
	task->SetAppCommand(m_appCmdDb->GetAppCommand(value["command"].asString()));
	


	return task;
    
}


bool AddNoUseTsTask::DoAppResponse(const Json::Value& appCmd,AppServerSession *session,Task* task)
{
	string msg;
	//Command *cmd;
	AppCommand *appCommand;
	Json::Value valueAppRes;
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();

	string commandName;

    if(!appCmd["command"].isNull() && appCmd["command"].isString())
        commandName = appCmd["command"].asString();

	//cmd = m_commandDb->GetCommand(commandName);
	appCommand = m_appCmdDb->GetAppCommand(commandName);
	if(!appCommand) 
		return false;

   
	appCommand->BuildAppResponse(appCmd,msg);
	session->SendMessage(msg);

	return true;
	
}

void AddNoUseTsTask::AddSpacificTask(Json::Value& value,AppServerSession *appServerSion)
{
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();
	Task* task;
	
	if(IsThisTaskType(value["command"].asString())){
		task = BuildBasicTask(value);
		if(!task) 
			return;
		task->m_tasktype = NoTsTask;
		if(!DoAppResponse(value,appServerSion,task))
			return;

		return;
	} else {
		AddTask::AddSpacificTask(value,appServerSion);
	}
}

bool AddNoUseTsTask::TaskSubmit(Task* task)
{
	 string appResult;
	 bool ret;
	 string appResultMsg;
	 SessionManager& sm = SessionManager::GetInstance();
	 
	//Command* cmd = task->GetCommand();
	AppCommand* appCmd = task->GetAppCommand();
	
	if(appCmd->IsProvidedResult()){
		ret = appCmd->BuildAppResponse(task->GetCmdValue(),appResult);
		if(!ret){
			
			return false;
		}

		appResultMsg = appResult + '\0';;

		sm.SendAppMessage(task->m_appTargetIp,task->m_appTargetPort,appResultMsg,task);
	}
}



