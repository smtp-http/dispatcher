//#include "CommandDb.h"
#include "Command.h"
#include "TaskResult.h"

//extern TaskVideoDb GTaskVideoDb;

//extern TaskDocumentDb GTaskDocumentDb;
//extern TaskDpptDb GTaskDpptDb;

int fail_times = 0;
int succ_times = 0;
int taskNull_times = 0;
extern int appmsgtotaltimes;
int tsLeave_times = 0;


extern int timeoutforRunningTask;
extern int timeoutforWaitTask;


extern TaskRunQueue GDocTaskRunQueue;
extern TaskRunQueue GVideoTaskRunQueue;
extern TaskRunQueue GDpptTaskRunQueue;

extern TaskWaitQueue GDocTaskWaitQueue;
extern TaskWaitQueue GVideoTaskWaitQueue;
extern TaskWaitQueue GDpptTaskWaitQueue;

//extern CommandVideoTrans GCmdVideoTrans;
extern AppCmdVideoTrans GAppCmdVideoTrans;

////////////////////////////////////// TaskResult /////////////////////////////////////////

TaskResult::TaskResult()
{
	next = NULL; 
	//m_commandDb = &CommandDb::GetInstance();
	m_appCmdDb = &AppCommandDb::GetInstance();
}

bool TaskResult::IsThisTaskType(const string resultName)
{
	vector<string>::iterator it;
	
	for (it = m_resultNames.begin(); it != m_resultNames.end(); ++ it){
		if("VideoTransRequest" == resultName && "Transfer" == *it)
			return true;
		if(resultName == *it)
			return true;
	}

	return false;
}

Task* TaskResult::PopTask()
{
	return m_taskWaitQueue->PopTask();
}

void TaskResult::DoTaskRresult(Json::Value& value,TsSession* session)
{
	Task* task;
	//string result;
	
	
	TransServer * tmp_transServer = NULL;
	string taskid;
	bool ret;
	TaskState* taskState;
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();

	if(!value["id"].isNull() && value["id"].isString()){
		taskid = value["id"].asString();
	}    
    
	// trans server reset
	tmp_transServer = m_tsqueue->GetTransServer(session);

	if(NULL != tmp_transServer){
		tmp_transServer->SetState(TsIdle);
		tmp_transServer->SetTask(NULL);
	}

	task = m_taskRunQueue->GetTaskInSpecId(taskid);
	if(!task){
		task = m_taskWaitQueue->GetTaskInSpecId(taskid);
	}
    
    
	if(!task){
		// TODO: log
		taskNull_times +=1;
		task = m_taskWaitQueue->PopTask();
		if(task){
			cout<<"ts free select task from waitqueue task id:"<<task->GetTaskId();
			m_disptacher->Submit(task);
		}
		return;
	}

	// ts reset
	(task->GetTransServer())->SetState(TsIdle);
	(task->GetTransServer())->SetTask(NULL);
    
	taskState = task->GetTaskState();
	if(!taskState){
		dsLog.Log(true,LOGGER_ERROR,"%s:%s:%d  task state is null! task id:%d\n",__FILE__,__PRETTY_FUNCTION__,__LINE__,task->GetTaskId().c_str());
		return;
	}
	taskState->Handle(value,this);
	
    

}



//////////////////////////////////// VideoTaskResult //////////////////////////////////////


VideoTaskResult& VideoTaskResult::GetInstance()
{
	static  VideoTaskResult instance_;
	return instance_; 	
}


VideoTaskResult::VideoTaskResult()
{
	//m_commandDb->GetResultNameInSpecificTaskType(VideoTask,m_resultNames);
	m_appCmdDb->GetResultNameInSpecificTaskType(VideoTask,m_resultNames);
	
	m_taskDb = &TaskVideoDb::GetInstance();//&GTaskVideoDb;
	m_disptacher = &VideoDisptacher::GetInstance();
	m_taskRunQueue = &GVideoTaskRunQueue;
	m_taskWaitQueue = &GVideoTaskWaitQueue;
	m_tsqueue = &GVideoTsQueue;
}

void VideoTaskResult::SpacificTaskResult(Json::Value& value,TsSession* session)
{
	cout << " ++++++  video result!" << endl;
	if(IsThisTaskType(value["servertype"].asString())) {
		
		cout << " ++++++  video result! 1" << endl;
		DoTaskRresult(value,session);
	} else {
		TaskResult::SpacificTaskResult(value,session);
	}
}


//////////////////////////////////// DocTaskResult //////////////////////////////////////


DocTaskResult& DocTaskResult::GetInstance()
{
	static  DocTaskResult instance_;
	return instance_; 	
}


DocTaskResult::DocTaskResult()
{
	//m_commandDb->GetResultNameInSpecificTaskType(DocTask,m_resultNames);
	m_appCmdDb->GetResultNameInSpecificTaskType(DocTask,m_resultNames);
	
	m_taskDb = &TaskDocumentDb::GetInstance();//  &GTaskDocumentDb;
	m_disptacher = &DocDisptacher::GetInstance();//GDocDisptacher;
	m_taskRunQueue = &GDocTaskRunQueue;
	m_taskWaitQueue = &GDocTaskWaitQueue;
}

void DocTaskResult::SpacificTaskResult(Json::Value& value,TsSession* session)
{
	//Task* task;
	if(IsThisTaskType(value["servertype"].asString())) {
		
		DoTaskRresult(value,session);
		
	} else {
		TaskResult::SpacificTaskResult(value,session);
	}
}

void DocTaskResult::DoTaskRresult(Json::Value& value,TsSession* session)
{
	Task* task;
	TaskState* taskState;
	string errorcode;
	string taskid;
	TransServer * tmp_transServer = NULL;
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();

	
	// ts reset
	if(GDocOpenofficeTsQueue.GetTransServer(session)){
		tmp_transServer = GDocOpenofficeTsQueue.GetTransServer(session);
	} else if (GDocWpsTsQueue.GetTransServer(session)){
		tmp_transServer = GDocWpsTsQueue.GetTransServer(session);
	} else if (GDocOfficeTsQueue.GetTransServer(session)){
		tmp_transServer = GDocOfficeTsQueue.GetTransServer(session);
	}

	
	if(NULL != tmp_transServer){
		tmp_transServer->SetState(TsIdle);
		tmp_transServer->SetTask(NULL);
	} else {
		dsLog.Log(true,LOGGER_ERROR,"%s:%s:%d  tmp_transServer is null!\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return;
	}

	if(!value["id"].isNull() && value["id"].isString()){
		taskid = value["id"].asString();
	} else {
		dsLog.Log(true,LOGGER_ERROR,"%s:%s:%d  error task id!\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
	}

	task = m_taskRunQueue->GetTaskInSpecId(taskid);
	cout << "++++++ DoTaskRresult    1  task:" << task << "    taskid:" << taskid<< endl;
	if(!task){
		task = m_taskWaitQueue->GetTaskInSpecId(taskid);
	}
    

	if(!task){
		// TODO: log
		taskNull_times +=1;

		task = m_taskWaitQueue->PopTask();
		if(task){
			cout<<"ts free select task from waitqueue task id:"<<task->GetTaskId();
			m_disptacher->Submit(task);
		}
		return;
	}



	// task state handling
	taskState = task->GetTaskState();
	if(!taskState){
		dsLog.Log(true,LOGGER_ERROR,"%s:%s:%d  task state is null! task id:%d\n",__FILE__,__PRETTY_FUNCTION__,__LINE__,task->GetTaskId().c_str());
		return;
	}
	
	taskState->Handle(value,this);

}


Task* DocTaskResult::PopTask()
{
	return m_taskWaitQueue->PopRightTaskForDocTs(m_docTsType);
}


void DocTaskResult::PunishTsPriority(Task *task)
{
	(dynamic_cast<TaskDocument *> (task))->PunishTsPriority(m_docTsType);
}


//////////////////////////////////// DpptTaskResult //////////////////////////////////////


DpptTaskResult& DpptTaskResult::GetInstance()
{
	static  DpptTaskResult instance_;
	return instance_; 	
}


DpptTaskResult::DpptTaskResult()
{
	//m_commandDb->GetResultNameInSpecificTaskType(DpptTask,m_resultNames);
	m_appCmdDb->GetResultNameInSpecificTaskType(DpptTask,m_resultNames);
	
	m_taskDb = &TaskDpptDb::GetInstance();//&GTaskDpptDb;
	m_disptacher = &DpptDisptacher::GetInstance();//&GDpptDisptacher;
	m_taskRunQueue = &GDpptTaskRunQueue;
	m_taskWaitQueue = &GDpptTaskWaitQueue;
	m_tsqueue = &GDpptTsQueue;
}

void DpptTaskResult::SpacificTaskResult(Json::Value& value,TsSession* session)
{
	Task* task;
	cout << "++++++   5" << endl;
	if(IsThisTaskType(value["servertype"].asString())) {
		
		cout << "++++++ 6 ---  Dppt result!" << endl;
		DoTaskRresult(value,session);
		
	} else {
		TaskResult::SpacificTaskResult(value,session);
	}
}
