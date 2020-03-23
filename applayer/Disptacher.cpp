#include "Disptacher.h"

#include "TaskQueue.h"
#include "AddTask.h"
//#include "TsManager.h"
//#include "CommandDb.h"
#include "error.h"
#include "Log4cxxWrapper.h"
#include <algorithm>
#include <memory>
#include "ConfigManager.h"
#include "GetTransServer.h"


//#include "CommandDb.h"

#include "Command.h"

//using namespace std;
using namespace cdy;


extern TaskWaitQueue GDocTaskWaitQueue;
extern TaskWaitQueue GVideoTaskWaitQueue;
extern TaskWaitQueue GDpptTaskWaitQueue;
extern TaskWaitQueue GNoTsTaskTaskWaitQueue;


extern TaskRunQueue GDocTaskRunQueue;
extern TaskRunQueue GVideoTaskRunQueue;
extern TaskRunQueue GDpptTaskRunQueue;
extern TaskRunQueue GNoTsTaskRunQueue;

extern TsQueue GVideoTsQueue;
extern TsQueue GDocOpenofficeTsQueue;
extern TsQueue GDocWpsTsQueue;
extern TsQueue GDocOfficeTsQueue;
extern TsQueue GDpptTsQueue;



string ToUpperString(string str)  
{  
	string tmpstr= str;
	transform(tmpstr.begin(), tmpstr.end(), tmpstr.begin(), (int (*)(int))toupper);  
	return tmpstr;
}  
string ToLowerString(string str)  
{  
	string tmpstr= str;
	transform(tmpstr.begin(), tmpstr.end(), tmpstr.begin(), (int (*)(int))tolower);  
	return tmpstr;
}


bool CheckNoSameName(map<int,string>& mp, int count)
{
	for(int i=0;i<count-1;i++){
		for(int j=i+1;j<count;j++){
			if(mp[i] == mp[j]){
				return false;
			}
		}
	}
	return true;
}


//////////////////////////////// Disptacher //////////////////////////////////

bool Disptacher::SubmitTaskToSpecTs(Task* task,TransServer * ts)
{
	string tsCmd;
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();
	TaskState* taskState;

	if(task == NULL || ts == NULL){
		dsLog.Log(true,LOGGER_ERROR,"param error !!!!!, [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}
	task->m_alreadyTriedTsId.push_back(ts->GetId());

	taskState = task->GetTaskState();
	if(NULL == taskState){
		dsLog.Log(true,LOGGER_ERROR,"taskState is NULL !!!!!, [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}
	
	TsCommand* cmd = taskState->GetTsCommand();
	if(NULL == cmd){
		dsLog.Log(true,LOGGER_ERROR,"ts command is NULL !!!!!, [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}
	
	if(!cmd->BuildTsCmd(task->GetCmdValue(),tsCmd)){
		dsLog.Log(true,LOGGER_ERROR,"build video ts cmd error !!!!!, [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	} 
	cout << " ++++++ SubmitTaskToSpecTs " << "   tsCmd: " << tsCmd << endl;
	string tsMsg = tsCmd + '\0';
	TsSession* session = ts->GetSession();
	session->SendMessage(tsMsg);
	task->m_triedTimes += 1;

	if(DuplicateTaskCheck(task->GetTaskId())){
		cout<<"push task in run queue, id:"<<task->GetTaskId()<<endl;
		task->SetTaskStartRunTime();
		m_taskRunQueue->PushTask(task,session);
	}

	task->SetTransServer(ts);
	ts->SetTask(task);
	
	return true;
}


//////////////////////////////// DocDisptacher ///////////////////////////////

DocDisptacher::DocDisptacher()
{
	m_taskWaitQueue = &GDocTaskWaitQueue;
	m_taskRunQueue = &GDocTaskRunQueue;

	GetDocTsPriority();
	
	m_tsQueues["linux"] = &GDocOpenofficeTsQueue;
	m_tsQueues["wps"] = &GDocWpsTsQueue;
	m_tsQueues["office"] = &GDocOfficeTsQueue;

	GetDocTsPriority();
}

DocDisptacher& DocDisptacher::GetInstance()
{
	static DocDisptacher instance_;
	return instance_;
}

bool DocDisptacher::Submit(Task *task)
{
	TsQueue* tsq;
	TransServer* ts = NULL;
	int i;
	string tsCmd;
	Json::Value appResponse;
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();
	//Command* cmd;
	bool ret;
    //vector<string>::iterator itor;

    if(NULL == task)
    {
        dsLog.Log(true,LOGGER_ERROR,"%s:%s:%d  sub task is null\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
        return false;
    }


    (GetTransServer::GetInstance()).GetTS(task,ts);
	if(ts){
//        cout<<"####select idle ts id: "<<ts->GetId()<<", ts type: "<<ts->GetDocTsType()<<endl;
        dsLog.Log(true,LOGGER_DEBUG,"\n ###################################################\n select idle ts id : %s, ip : %s\n",
            (ts->GetId()).c_str(),(ts->GetIp()).c_str());
		if(!SubmitTaskToSpecTs(task,ts)){
			// TODO: log
			return false;
		}
	} else {
	    cout<<"push task in wait queue, id:"<<task->GetTaskId()<<endl;
		m_taskWaitQueue->PushTask(task);
	}

	return true;
}

#define MAX_DOCUMENT_SEVER_TYPE_NUM 3
bool DocDisptacher::GetDocTsPriority()
{
	Json::Value  value;  
	std::auto_ptr<Json::Reader> pJsonParser(new Json::Reader(Json::Features::strictMode())); 

	LoggerWrapper dsLog= LoggerWrapper::GetInstance();

	string strTsPriority = "\"TsPriority\":" + ConfigManager::GetTsPriority();
	string strWholeJson = "{"+strTsPriority+"}";
	//string strPriorityArray[MAX_DOCUMENT_SEVER_TYPE_NUM];

	map<string,TsQueue*> tsQueue;
	
	if(pJsonParser->parse(strWholeJson,value)){
		const Json::Value TsPriority = value["TsPriority"];
 
		if((!value["TsPriority"].isNull()) && (value["TsPriority"].isArray()) && (MAX_DOCUMENT_SEVER_TYPE_NUM == TsPriority.size()) ){
			for (unsigned int i = 0; i < TsPriority.size(); i++){
				if(TsPriority[i].isString()){
					if(( "LINUX" == ToUpperString(TsPriority[i].asString()))||( "OFFICE" == ToUpperString(TsPriority[i].asString()))||( "WPS" == ToUpperString(TsPriority[i].asString()))){
						m_tsPriority[i] = TsPriority[i].asString();
						
					} else {
						dsLog.Log(true,LOGGER_ERROR,"error config file. Invalid TsPriority format  . :%s , [%s][%s][%d]\n",strTsPriority.c_str(),__FILE__,__PRETTY_FUNCTION__,__LINE__);               
						return false;
					}
				} else {
					dsLog.Log(true,LOGGER_ERROR,"error config file. Invalid TsPriority format  . :%s , [%s][%s][%d]\n",strTsPriority.c_str(),__FILE__,__PRETTY_FUNCTION__,__LINE__);               
					return false;            
				}
			}
			if(!CheckNoSameName(m_tsPriority,MAX_DOCUMENT_SEVER_TYPE_NUM)){
				dsLog.Log(true,LOGGER_ERROR,"error config file. Invalid TsPriority format  . :%s , [%s][%s][%d]\n",strTsPriority.c_str(),__FILE__,__PRETTY_FUNCTION__,__LINE__);               
				return false;                
			}
		} else{
  
			dsLog.Log(true,LOGGER_ERROR,"error config file. Invalid TsPriority format . :%s , [%s][%s][%d]\n",strTsPriority.c_str(),__FILE__,__PRETTY_FUNCTION__,__LINE__);               
			return false;            
		}

	} else{
		dsLog.Log(true,LOGGER_ERROR,"error config file. Invalid TsPriority format  . :%s , [%s][%s][%d]\n",strTsPriority.c_str(),__FILE__,__PRETTY_FUNCTION__,__LINE__);               
		return false;            
	}    

	return true;

}

///////////////////////////////// VideoDisptacher /////////////////////////////

VideoDisptacher::VideoDisptacher()
{
	m_taskWaitQueue = &GVideoTaskWaitQueue;
	m_taskRunQueue = &GVideoTaskRunQueue;
	m_tsQueues["Video1"] = &GVideoTsQueue;
}

VideoDisptacher& VideoDisptacher::GetInstance()
{
	static VideoDisptacher instance_;
	return instance_;
}

bool VideoDisptacher::Submit(Task *task)
{
	TsQueue* tsq;
	VideoTransServer* ts = NULL;
	string tsCmd;
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();

	if(NULL == task){
		return false;
	}

	tsq = m_tsQueues["Video1"];
	//Command *cmd = task->GetCommand();
	AppCommand *appCmd = task->GetAppCommand();
	if(!appCmd){
		return false;
	}

	
	
	ts = tsq->GetTsIdle(/*appCmd->GetTaskProperty()*/task->GetTaskState()->GetTsCommand()->GetTaskProperty()); //tsq->GetTsIdle();
	if(ts){
 //       cout<<"####select idle ts id: "<<ts->GetId()<<endl;
        dsLog.Log(true,LOGGER_DEBUG,"\n ###################################################\n select idle ts id : %s, ip : %s\n",
            (ts->GetId()).c_str(),(ts->GetIp()).c_str());
		if(!SubmitTaskToSpecTs(task,ts)){
			// TODO: log
			return false;
		}
	} else {
		m_taskWaitQueue->PushTask(task);
	}

}

//////////////////////////////// DpptDisptacher ///////////////////////////////

DpptDisptacher::DpptDisptacher()
{
	m_taskWaitQueue = &GDpptTaskWaitQueue;
	m_taskRunQueue = &GDpptTaskRunQueue;
	m_tsQueues["Dppt1"] = &GDpptTsQueue;
}


DpptDisptacher& DpptDisptacher::GetInstance()
{
	static DpptDisptacher instance_;
	return instance_;
}


bool DpptDisptacher::Submit(Task *task)
{
	TsQueue* tsq;
	TransServer* ts = NULL;
	string tsCmd;
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();

	if(NULL == task){
		return false;
	}
	

	tsq = m_tsQueues["Dppt1"];
	ts = tsq->GetTsIdle();
	if(ts){
//        cout<<"####select idle ts id: "<<ts->GetId()<<endl;
		dsLog.Log(true,LOGGER_DEBUG,"\n ###################################################\n select idle ts id : %s, ip : %s\n",
            (ts->GetId()).c_str(),(ts->GetIp()).c_str());
		if(!SubmitTaskToSpecTs(task,ts)){
			// TODO: log
			return false;
		}
	} else {
		m_taskWaitQueue->PushTask(task);
	}
}

