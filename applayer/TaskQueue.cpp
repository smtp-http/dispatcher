#include "TaskQueue.h"
#include <vector>
#include <iostream>
#include <stdio.h>
//#include "CommandDb.h"
#include "Command.h"
#include "DataPersistence.h"


using namespace std;

TaskWaitQueue GDocTaskWaitQueue(DocTask);
TaskWaitQueue GVideoTaskWaitQueue(VideoTask);
TaskWaitQueue GDpptTaskWaitQueue(DpptTask);
TaskWaitQueue GNoTsTaskTaskWaitQueue(NoTsTask);


TaskRunQueue GDocTaskRunQueue(DocTask);
TaskRunQueue GVideoTaskRunQueue(VideoTask);
TaskRunQueue GDpptTaskRunQueue(DpptTask);
TaskRunQueue GNoTsTaskRunQueue(NoTsTask);
//////////////////////////  TaskQueue  //////////////////////
//TaskQueue




//////////////////////////  TaskWaitQueue ///////////////////
TaskWaitQueue::TaskWaitQueue(TaskType type)
	:m_taskType(type)
{
	//m_dataPresistence = &DataPresistence::GetInstance(NULL);
}

Task* TaskWaitQueue::PopTask()
{
	Task* task;
	if(m_queue.empty())
		return NULL;

	task = m_queue.top( );
	m_queue.pop();

	m_dataPresistence->CreateWaitMsgFile(m_taskType);
		
	return task;
}

Task* TaskWaitQueue::PopRightTaskForDocTs(DocTsType tsType)
{
    Task* task;
    Task* tmptask;
    TaskPriorityQueue tmpQueue;
	if(m_queue.empty())
		return NULL;
    
    while( !m_queue.empty()){
		task = m_queue.top( );
		m_queue.pop( );

        //tmpQueue.push(task );
        if(task)
        {
            if((tsType == (dynamic_cast<TaskDocument*>(task))->GetAppiontedTS())||(SERVER_NOAPPOINTED == (dynamic_cast<TaskDocument*>(task))->GetAppiontedTS()))
            {
                m_dataPresistence->CreateWaitMsgFile(m_taskType);

                while ( !tmpQueue.empty() )
                {
            		tmptask = tmpQueue.top( );
                    tmpQueue.pop( );
            		m_queue.push(tmptask);
            	}
	            return task;
            }
            else
            {
                tmpQueue.push(task);
            }
        }
	}

    while ( !tmpQueue.empty() )
    {
		task = tmpQueue.top( );
		tmpQueue.pop( );
		m_queue.push(task );
	}
    return NULL;
}


Task* TaskWaitQueue::GetTaskInSpecId(string taskId)
{
	int i;
	vector<Task *> *vtor = (vector<Task *> *)&m_queue;

	for( int i = 0 ; i < vtor->size(); i++ ){
		if(((Task *)(vtor->operator [](i)))->GetTaskId() == taskId)
			return (Task *)(vtor->operator [](i));
	}

	return NULL;
}

Task* TaskWaitQueue::GetTaskInSpecId(string taskId,unsigned int &position)
{
	int i;
	vector<Task *> *vtor = (vector<Task *> *)&m_queue;

	for( int i = 0 ; i < vtor->size(); i++ ){
		if(((Task *)(vtor->operator [](i)))->GetTaskId() == taskId){
				position = i;
				return (Task *)(vtor->operator [](i));
			}
	}

	return NULL;
}

bool TaskWaitQueue::PushTask(Task* task)
{
	task->m_tsSession = NULL;
	m_queue.push(task);
	
	m_dataPresistence->CreateWaitMsgFile(m_taskType);
		
	return true;
}


bool TaskWaitQueue::CheckSameTaskId(string id)
{
	int i;
	vector<Task *> *vtor = (vector<Task *> *)&m_queue;

	for( int i = 0 ; i < vtor->size(); i++ ){
		if(((Task *)(vtor->operator [](i)))->GetTaskId() == id){
            
			return true;
        }
	}
	
	return false;
}

TaskPriorityQueue& TaskWaitQueue::GetTaskList()
{
    return m_queue;
}



////////////////////////// TaskRunQueue //////////////////////////////////
TaskRunQueue::TaskRunQueue(TaskType type)
	:m_taskType(type)
{
	
}



bool TaskRunQueue::PushTask(Task* task,TsSession* session)
{
	task->m_tsSession = session;
	m_taskList.push_back(task);

	m_dataPresistence->CreateRunMsgFile(m_taskType);
	
	return true;
}

list<Task *>& TaskRunQueue::GetTaskList()
{
    return m_taskList;
}


Task* TaskRunQueue::GetTaskInSpecId(string taskId)
{
	list<Task *>::iterator it;
	
	for(it = m_taskList.begin();it != m_taskList.end();it ++){
		if((*it)->GetTaskId() == taskId){
			return (Task *)(*it);
		}
	}

	return NULL;
}

Task* TaskRunQueue::GetTaskInSession(TsSession* session)
{
	list<Task *>::iterator it;
	
	for(it = m_taskList.begin();it != m_taskList.end();it ++){
		if((*it)->m_tsSession == session){
			return (Task *)(*it);
		}
	}

	return NULL;
}

bool TaskRunQueue::DelTask(string taskId)
{
	list<Task *>::iterator it;
	bool ret = false;
	Task* task;

	for(it = m_taskList.begin();it != m_taskList.end();it ++){
		task = *it;
		if(task->GetTaskId() == taskId){
			task->m_tsSession = NULL;
            task->SetTransServer(NULL);
			m_taskList.erase(it);
			ret = true;
			break;
		}
	}

	m_dataPresistence->CreateRunMsgFile(m_taskType);

	return ret;
}

bool TaskRunQueue::CheckSameTaskId(string taskId)
{
	list<Task *>::iterator it;
	bool ret = false;

	for(it = m_taskList.begin();it != m_taskList.end();it ++){
        cout<<"#########run queue##### id:"<<(*it)->GetTaskId()<<endl;
		if((*it)->GetTaskId() == taskId){
			ret = true;
            //return true;
		}
	}

	return ret;
}

/////////////////////////  TaskQueueManager //////////////////////

TaskQueueManager::TaskQueueManager()
{
	vector<string> videoResultName;  //ServerType
	vector<string> DocResultName;
	vector<string> DpptResultName;
	vector<string>::iterator it;
	
	//m_commandDb = &CommandDb::GetInstance();
	m_appCmdDb = &AppCommandDb::GetInstance();

	if(!m_appCmdDb->GetResultNameInSpecificTaskType(DocTask,DocResultName)){
		// TODO: error log
	}

	if(!m_appCmdDb->GetResultNameInSpecificTaskType(VideoTask,videoResultName)){
		// TODO: error log
	}

	if(!m_appCmdDb->GetResultNameInSpecificTaskType(DpptTask,DpptResultName)){
		// TODO: error log
	}

	
		
	for (it = DocResultName.begin(); it != DocResultName.end(); ++ it){
		m_taskRunQList[*it] = &GDocTaskRunQueue;
		m_taskWaitQList[*it] = &GDocTaskWaitQueue;
	}

	for (it = videoResultName.begin(); it != videoResultName.end(); ++ it){
		m_taskRunQList[*it] = &GVideoTaskRunQueue;
		m_taskWaitQList[*it] = &GVideoTaskWaitQueue;
	}

	for (it = DpptResultName.begin(); it != DpptResultName.end(); ++ it){
		m_taskRunQList[*it] = &GDpptTaskRunQueue;
		m_taskWaitQList[*it] = &GDpptTaskWaitQueue;
	}

}

TaskQueueManager& TaskQueueManager::GetInstance()
{
	static  TaskQueueManager instance_;
	return instance_; 	
}





TaskRunQueue* TaskQueueManager::GetRunQueue(string serverType)
{
	if(m_taskRunQList.find(serverType) != m_taskRunQList.end())
		return m_taskRunQList[serverType];
	else {
		// TODO: error log
		return NULL;
	}
}

TaskWaitQueue* TaskQueueManager::GetWaitQueue(string serverType)
{
	if(m_taskWaitQList.find(serverType) != m_taskWaitQList.end())
		return m_taskWaitQList[serverType];
	else {
		// TODO: error log
		return NULL;
	}
}

