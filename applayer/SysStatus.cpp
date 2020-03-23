#include "SysStatus.h"
#include "DataPersistence.h"
#include "Log4cxxWrapper.h"
#include "TaskQueue.h"
#include "AddTask.h"
#include <sys/time.h>
#include <sys/times.h>
#include "../include/ConfigManager.h"



extern TaskWaitQueue GDocTaskWaitQueue;
extern TaskWaitQueue GVideoTaskWaitQueue;
extern TaskWaitQueue GDpptTaskWaitQueue;
extern TaskWaitQueue GNoTsTaskTaskWaitQueue;


extern TaskRunQueue GDocTaskRunQueue;
extern TaskRunQueue GVideoTaskRunQueue;
extern TaskRunQueue GDpptTaskRunQueue;
extern TaskRunQueue GNoTsTaskRunQueue;


int timeoutforRunningTask = 0;
int timeoutforWaitTask = 0;

extern int fail_times;

using namespace std;  

unsigned int DsClkRateGet()
{
    long ret = sysconf(_SC_CLK_TCK);

    return (unsigned int)ret;
}


bool IsAppTaskTimedOut(unsigned int time_value,unsigned int dwTimedOutMinutes)
{
    unsigned int dwEscapedTimeMilliSeconds = 0;  //毫秒级别
    unsigned int dwNowTime;

    dwNowTime = (unsigned int)times(NULL);
    
    /* if timer rollback */
    if (dwNowTime < time_value)
    {
        dwEscapedTimeMilliSeconds = (((unsigned int)-1) - (time_value - dwNowTime) + 1) * 1000 / DsClkRateGet();
    }
    else
    {
        dwEscapedTimeMilliSeconds = (dwNowTime - time_value)  * 1000 / DsClkRateGet();
    }

    if((dwTimedOutMinutes*60*1000) <= dwEscapedTimeMilliSeconds)
    {
        return true;
    }

    return false;
}


SysStatus& SysStatus::GetInstance()
{
	static  SysStatus instance_;
	return instance_; 
}

SysStatus::SysStatus()
{
	m_taskWaitQueues[DocTask]   = &GDocTaskWaitQueue;
	m_taskWaitQueues[DpptTask]  = &GDpptTaskWaitQueue;
	m_taskWaitQueues[VideoTask] = &GVideoTaskWaitQueue;
	m_taskWaitQueues[NoTsTask]  = &GNoTsTaskTaskWaitQueue;

	m_taskRunQueues[DocTask]    = &GDocTaskRunQueue;
	m_taskRunQueues[DpptTask]   = &GDpptTaskRunQueue;
	m_taskRunQueues[VideoTask]  = &GVideoTaskRunQueue;
	m_taskRunQueues[NoTsTask]   = &GNoTsTaskRunQueue;


    
    m_taskDb[DocTask]           = &TaskDocumentDb::GetInstance();
	m_taskDb[DpptTask]          = &TaskDpptDb::GetInstance();
	m_taskDb[VideoTask]         = &TaskVideoDb::GetInstance();
	m_taskDb[NoTsTask]          = &TaskNoUseTsDb::GetInstance();

    m_disptacher[DocTask]           = &DocDisptacher::GetInstance();
	m_disptacher[DpptTask]          = &DpptDisptacher::GetInstance();
	m_disptacher[VideoTask]         = &VideoDisptacher::GetInstance();
    

	m_dataPresistence = &DataPresistence::GetInstance(/*&SysStatus::GetInstance()*/this);
	m_taskWaitQueues[DocTask]->SetDataPresistence(m_dataPresistence);
	m_taskWaitQueues[DpptTask]->SetDataPresistence(m_dataPresistence);
	m_taskWaitQueues[VideoTask]->SetDataPresistence(m_dataPresistence);
	m_taskWaitQueues[NoTsTask]->SetDataPresistence(m_dataPresistence);

	m_taskRunQueues[DocTask]->SetDataPresistence(m_dataPresistence);
	m_taskRunQueues[DpptTask]->SetDataPresistence(m_dataPresistence);
	m_taskRunQueues[VideoTask]->SetDataPresistence(m_dataPresistence);
	m_taskRunQueues[NoTsTask]->SetDataPresistence(m_dataPresistence);

    m_DocTaskTimedOutMinutes        = ConfigManager::GetDocTaskTimedOutMinutes();
    m_DynamicPptTaskTimedOutMinutes = ConfigManager::GetDynamicPptTaskTimedOutMinutes();
    m_VideoTaskTimedOutMinutes      = ConfigManager::GetVideoTaskTimedOutMinutes();

    //set doc task appointed transServer type
    m_docTaskAppointedTsType["LINUX"] = SERVER_LINUX;
    m_docTaskAppointedTsType["WPS"] = SERVER_WPS;
    m_docTaskAppointedTsType["OFFICE"] = SERVER_OFFICE;
}
void SysStatus::SetDataPresistence(IDataPresistence *dataPresistence)
{
	if(dataPresistence != NULL)
		m_dataPresistence = dataPresistence;
}

void SysStatus::SysStatusRestore()
{
	CopyMessageToQueue(DocTask);
	CopyMessageToQueue(DpptTask);
	CopyMessageToQueue(VideoTask);
	CopyMessageToQueue(NoTsTask);
}

void SysStatus::CopyMessageToQueue(TaskType type)
{
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();
	DpMessageArryType dpDocMessageRunArr;
    DpMessageArryType dpDocMessageWaitArr;
	vector<DpMessageType>::iterator run_iter;
	vector<DpMessageType>::iterator wait_iter;
	Task *task;
    
	if(m_dataPresistence->DpGetWaitMessage(dpDocMessageWaitArr,type)){
		  for (wait_iter = dpDocMessageWaitArr.begin();wait_iter!=dpDocMessageWaitArr.end();wait_iter++) {
		  	task = AddTask::BuildBasicTask(*wait_iter,type);
            if(DocTask == type)
            {
                Json::Value tmpvalue = *wait_iter;
                string doc_server_type;
                if(!tmpvalue["docServerType"].isNull()&&tmpvalue["docServerType"].isString())
                {
                    doc_server_type = tmpvalue["docServerType"].asString();
                    //cout<<"SET appiontedts :"<<doc_server_type<<endl;
                    (dynamic_cast<TaskDocument*>(task))->SetAppiontedTS(m_docTaskAppointedTsType[doc_server_type]);
                }
                else
                {
                    //cout<<"SET appiontedts noappoined"<<endl;
                    (dynamic_cast<TaskDocument*>(task))->SetAppiontedTS(SERVER_NOAPPOINTED);
                }
                
            }
            //cout<<"####CopyMessageToQueue push wait to wait: "<<task->GetTaskId()<<endl;
            task->SetTaskBrithTime();
			m_taskWaitQueues[type]->PushTask(task);
		  }
	} else {
		dsLog.Log(true,LOGGER_ERROR,"%s:%s:%d   Get wait dp  task tyep: %d fail!\n",__FILE__,__PRETTY_FUNCTION__,__LINE__,type);
	}

    if(m_dataPresistence->DpGetRunMessage(dpDocMessageRunArr,type)){
		  for (run_iter = dpDocMessageRunArr.begin();run_iter!=dpDocMessageRunArr.end();run_iter++) {
		  	task = AddTask::BuildBasicTask(*run_iter,type);
            if(DocTask == type)
            {
                Json::Value tmpvalue = *run_iter;
                string doc_server_type;
                if(!tmpvalue["docServerType"].isNull()&&tmpvalue["docServerType"].isString())
                {
                    doc_server_type = tmpvalue["docServerType"].asString();
                    //cout<<"SET appiontedts :"<<doc_server_type<<endl;
                    (dynamic_cast<TaskDocument*>(task))->SetAppiontedTS(m_docTaskAppointedTsType[doc_server_type]);
                }
                else
                {
                    //cout<<"SET appiontedts noappoined"<<endl;
                    (dynamic_cast<TaskDocument*>(task))->SetAppiontedTS(SERVER_NOAPPOINTED);
                }
                
            }
            //cout<<"####CopyMessageToQueue push run to wait: "<<task->GetTaskId()<<endl;
            task->SetTaskBrithTime();
            task ->SetPriority(255);
			m_taskWaitQueues[type]->PushTask(task);
		  }
	} else {
		dsLog.Log(true,LOGGER_ERROR,"%s:%s:%d   Get run dp  task tyep: %d fail!\n",__FILE__,__PRETTY_FUNCTION__,__LINE__,type);
	}

    //cout<<"#####CopyMessageToQueue taskwaitQueue size: "<<(m_taskWaitQueues[type]->GetTaskList()).size()<<endl;
}
/*
bool SysStatus::CopyJsonvalueToMap(Json::Value &cmdValue,DpMessageType &dpMessage)
{
	if(cmdValue.type() == Json::objectValue) {
		Json::Value::Members members( cmdValue.getMemberNames() ); 
		for ( Json::Value::Members::iterator it = members.begin();it != members.end();++it ) {  
			const std::string &key = *it;  
			if(cmdValue[key].type() == Json::stringValue) {
				dpMessage[key] = cmdValue[key].asString();
			} else {
				// TODO: error log, not string type

				return false;
			}
		}
	} else {
		// TODO: error log, error type
		return false;
	}

	return true;
}
*/

bool SysStatus::GetWaitQueueMessage(DpMessageArryType &dpMsgArry,TaskWaitQueue &taskWaitQ)
{
	Task* task;
	LoggerWrapper& dsLog = LoggerWrapper::GetInstance();
	TaskPriorityQueue &waitQ = taskWaitQ.GetTaskList();

	TaskPriorityQueue tmpQueue;
	
	while( !waitQ.empty()){
		task = waitQ.top( );
		waitQ.pop( );

		tmpQueue.push(task);		
		if (task){
			dpMsgArry.push_back(task->GetCmdValue());
		} else {
			dsLog.Log(true,LOGGER_ERROR,"%s:%s:%d  task null!!!\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		}
	}

	while ( !tmpQueue.empty() ){
		task = tmpQueue.top( );
		tmpQueue.pop( );
		waitQ.push(task );
	}
	return true;
}


bool SysStatus::GetDocWaitQueueMessage(DpMessageArryType &dpMsgArry)
{
	return GetWaitQueueMessage(dpMsgArry,*m_taskWaitQueues[DocTask]);
}



bool SysStatus::GetDpptWaitQueueMessage(DpMessageArryType &dpMsgArry)
{
	return GetWaitQueueMessage(dpMsgArry,*m_taskWaitQueues[DpptTask]);
}

bool SysStatus::GetVideoWaitQueueMessage(DpMessageArryType &dpMsgArry)
{
	return GetWaitQueueMessage(dpMsgArry,*m_taskWaitQueues[VideoTask]);
}


bool SysStatus::GetRunQueueMessage(DpMessageArryType &dpMsgArry,TaskRunQueue &taskRunQ)
{
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();
	list<Task *> *runQ = &taskRunQ.GetTaskList();
	
	list<Task *>::iterator it;  
	for(it = runQ->begin();it != runQ->end();it ++){  
		Task * task = *it;  
		if (task){
			dpMsgArry.push_back(task->GetCmdValue());
		} else {
			dsLog.Log(true,LOGGER_ERROR,"%s:%s:%d  task null!!!\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		}
	}  
	return true;
}

bool SysStatus::GetDocRunQueueMessage(DpMessageArryType &dpMsgArry)
{
	return GetRunQueueMessage(dpMsgArry,*m_taskRunQueues[DocTask]);
}



bool SysStatus::GetDpptRunQueueMessage(DpMessageArryType &dpMsgArry)
{
	return GetRunQueueMessage(dpMsgArry,*m_taskRunQueues[DpptTask]);
}



bool SysStatus::GetVideoRunQueueMessage(DpMessageArryType &dpMsgArry)
{
	return GetRunQueueMessage(dpMsgArry,*m_taskRunQueues[VideoTask]);
}


bool GetResultReTransMsg(DpMessageArryType &dpMsgArry)
{
	// TODO: imp
}

int SysStatus::RunQueueCheck(TaskType type,unsigned int TaskTimedOutMinutes)
{
    LoggerWrapper dsLog= LoggerWrapper::GetInstance();
    TaskRunQueue * taskRunQ         = m_taskRunQueues[type];
    TaskWaitQueue * taskWaitQ       = m_taskWaitQueues[type];
    TaskDb * taskdb                 = m_taskDb[type];
    Disptacher* taskDisptacher      = m_disptacher[type];
    Json::Value timeOutAppResult;
 
    //unsigned int dwVideoTaskTimedOuntMinutes = GetVideoTaskTimedOutMinutes();
    //unsigned int TaskTimedOutMinutes = 1;
    //检查视频转换列表

    list<Task *> runQ = taskRunQ->GetTaskList();
	
	list<Task *>::iterator it;  
	for(it = runQ.begin();it != runQ.end();it ++)
    {
        Task * task = *it;
        if(IsAppTaskTimedOut(task->GetTaskStartRunTime(),TaskTimedOutMinutes))
        {
            (task->GetTransServer())->SetState(TsIdle);
            (task->GetTransServer())->SetTask(NULL);
            timeOutAppResult["id"] = task->GetTaskId();
            stringstream ss;
        	ss<<ERROR_TASKTIMEOUT; 
        	string strErrCode = ss.str();
            timeOutAppResult["errorcode"] = strErrCode;
            task->ResultReport(timeOutAppResult);
            timeoutforRunningTask +=1;
            fail_times +=1;
            cout<<"----Running queue timeout task: "<<task->GetTaskId()<<endl;
            dsLog.Log(true,LOGGER_ERROR,"%s:%s:%d  running task timeout id: %s\n",__FILE__,__PRETTY_FUNCTION__,__LINE__,task->GetTaskId().c_str());
			taskRunQ->DelTask(task->GetTaskId());
			taskdb->GiveBackTask(task);

            task = taskWaitQ->PopTask();
            if(task)
            {
                taskDisptacher->Submit(task);
            }

        }
    }
    return 0;
}

int SysStatus::DocRunQueueCheck()
{
    RunQueueCheck(DocTask,m_DocTaskTimedOutMinutes);
    return 0;
}

int SysStatus::VideoRunQueueCheck()
{
    RunQueueCheck(VideoTask,m_VideoTaskTimedOutMinutes);
    return 0;
}

int SysStatus::DpptRunQueueCheck()
{
    RunQueueCheck(DpptTask,m_DynamicPptTaskTimedOutMinutes);
    return 0;
}

int SysStatus::WaitQueueCheck(TaskType type)
{
    LoggerWrapper dsLog= LoggerWrapper::GetInstance();
    TaskPriorityQueue &taskWaitQ = (m_taskWaitQueues[type])->GetTaskList();
    TaskDb * taskdb              = m_taskDb[type];
    Disptacher * disptacher = m_disptacher[type];
    Json::Value timeOutAppResult;

    Task*  task;
    TaskPriorityQueue tmpQueue;
    //unsigned int TaskWaitTimedOuntMinutes = GetVideoTaskTimedOutMinutes();
    unsigned int TaskWaitTimedOuntMinutes = 240;
    unsigned int taskpriority = 0;
    
    
    while( !taskWaitQ.empty() )
    {
        task = taskWaitQ.top( );
        taskWaitQ.pop();  

        if(IsAppTaskTimedOut(task->GetTaskBrithTime(),TaskWaitTimedOuntMinutes))
        {   
            //报告失败
            //VideoTaskTimedOutReport(task);            
            //将该任务归还给空闲队列
            timeoutforWaitTask +=1;
            fail_times +=1;
            dsLog.Log(true,LOGGER_ERROR,"%s:%s:%d  wait task timeout id: %s\n",__FILE__,__PRETTY_FUNCTION__,__LINE__,task->GetTaskId().c_str());
            timeOutAppResult["id"] = task->GetTaskId();
            stringstream ss;
        	ss<<ERROR_TASKTIMEOUT; 
        	string strErrCode = ss.str();
            timeOutAppResult["errorcode"] = strErrCode;
            task->ResultReport(timeOutAppResult);
            cout<<"----Wait queue timeout task: "<<task->GetTaskId()<<endl;
            taskdb->GiveBackTask(task);
            m_dataPresistence->CreateWaitMsgFile(type);
        }
        else
        {
            taskpriority = task->GetPriority();
            task->SetPriority(taskpriority+1);
            tmpQueue.push( task );
        }
    }

    while ( !tmpQueue.empty() )
    {
        task = tmpQueue.top( );
        tmpQueue.pop( );
        taskWaitQ.push(task );
    }

    //find if has idle ts, push task to idle ts
    if(!taskWaitQ.empty())
    {
        task = (m_taskWaitQueues[type])->PopTask();
    	if(task)
    	{
    		cout<<"ts free select task from waitqueue task id:"<<task->GetTaskId();
    		disptacher->Submit(task);
    	}
    }
    
	return 0;
}

int SysStatus::DocWaitQueueCheck()
{
    WaitQueueCheck(DocTask);
    return 0;
}

int SysStatus::VideoWaitQueueCheck()
{
    WaitQueueCheck(VideoTask);
    return 0;
}

int SysStatus::DpptWaitQueueCheck()
{
    WaitQueueCheck(DpptTask);
    return 0;
}



int SysStatus::OnTimer()
{
	//cout<<"#####time out"<<endl;

    SysStatus::GetInstance().DocRunQueueCheck();
    SysStatus::GetInstance().VideoRunQueueCheck();
    SysStatus::GetInstance().DpptRunQueueCheck();

    SysStatus::GetInstance().DocWaitQueueCheck();
    SysStatus::GetInstance().VideoWaitQueueCheck();
    SysStatus::GetInstance().DpptWaitQueueCheck();

    return 0;

}
