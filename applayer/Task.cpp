
#include "Command.h"
#include "Task.h"
//#include "CommandDb.h"
#include "InterfaceApi.h"
/////////////////////////// Task /////////////////////////////
void Task::ResultReport(Json::Value& value)
{
	string result;
	AppCommand* appCmd;
	bool ret;
	
	appCmd = this->GetAppCommand();
	cout << " ++++++ result report()  appcmd: " << appCmd->GetCommandName() << endl;
	ret = appCmd->BuildAppResult(value,result);
	if(ret){cout << " ++++++ result report:    1" << endl;
		SessionManager::GetInstance().SendAppMessage(this->m_appTargetIp,this->m_appTargetPort,result,this);
	} else {

	}
}

void Task::SetAppCommand(AppCommand* appCmd)
{
	cout << "++++++ Task::SetAppCommand  appcmd: " << appCmd << endl;
	m_appCmd = appCmd;
	TaskState* state = appCmd->CreatTaskState();
	InitTaskState(appCmd->CreatTaskState(),this);
}

void Task::InitTaskState(TaskState* taskState,Task* task)
{
	m_taskState = taskState;
	taskState->SetTask(task);
}

void Task::ChangeState(TaskState *state)
{
	if(m_taskState){
		delete m_taskState;
	}
	
	InitTaskState(state,this);
}
///////////////////////// TaskDocument ///////////////////////
TaskDocument::TaskDocument()
{
	m_maxTryTimes = 3;

    ts_typepair["linux"] = SERVER_LINUX;
    ts_typepair["wps"] = SERVER_WPS;
    ts_typepair["office"] = SERVER_OFFICE;

    SetTsPriority((DocDisptacher::GetInstance()).m_tsPriority);
}

void TaskDocument::PunishTsPriority(DocTsType ts_type)
{
    DocTsType tmptstype;
    map<int,DocTsType>::iterator itor;
    map<int,DocTsType>::iterator tmp_itor;
    bool findtstype = false;

    if(SERVER_NOAPPOINTED != m_appointedServerType)
    {
        return;
    }
    for(itor = m_tsPriority.begin(); itor != m_tsPriority.end(); itor++)
    {
        if(ts_type == itor->second)
            findtstype = true;
        if(findtstype )
        {
            tmp_itor = itor;
            if(itor++ != m_tsPriority.end())
                tmp_itor->second = itor->second;
            else
                tmp_itor->second = ts_type;
            itor = tmp_itor;
        }
    }
}
void TaskDocument::SetTsPriority(map<int,string> & ts_priority)
{
    m_tsPriority.clear();
    map<int,string>::iterator itor;
    for(itor = ts_priority.begin(); itor != ts_priority.end(); itor++)
    {
        m_tsPriority.insert(make_pair(itor->first,ts_typepair[itor->second]));
    }
}

map<int,DocTsType> & TaskDocument::GetTsPriority()
{
    return m_tsPriority;
}



///////////////////////// TaskVideo ///////////////////////
TaskVideo::TaskVideo()
{
	m_maxTryTimes = 3;
}



///////////////////////// TaskDppt ///////////////////////
TaskDppt::TaskDppt()
{
	m_maxTryTimes = 3;
}




/////////////////////// TaskDocumentDb //////////////////
TaskDocumentDb::TaskDocumentDb()
{

}


TaskDocumentDb& TaskDocumentDb::GetInstance()
{
		static  TaskDocumentDb instance_;
		return instance_; 	
}


////////////////////// TaskVideoDb //////////////////////

TaskVideoDb::TaskVideoDb()
{

}

TaskVideoDb& TaskVideoDb::GetInstance()
{
		static  TaskVideoDb instance_;
		return instance_; 	
}

//////////////////// TaskNoUseTsDb ////////////////////////
TaskNoUseTsDb::TaskNoUseTsDb()
{

}

TaskNoUseTsDb& TaskNoUseTsDb::GetInstance()
{
		static  TaskNoUseTsDb instance_;
		return instance_; 	
}


//////////////////// TaskDpptDb ///////////////////////////

TaskDpptDb::TaskDpptDb()
{

}

TaskDpptDb& TaskDpptDb::GetInstance()
{
		static  TaskDpptDb instance_;
		return instance_; 	
}

