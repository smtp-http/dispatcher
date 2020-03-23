#include "TsUpdate.h"

////////////////////////////////////////

extern TaskRunQueue GVideoTaskRunQueue;

///////////////// TsVideoTransProgress //////////////////
TsVideoTransProgress::TsVideoTransProgress()
{
	m_taskType = "Transfer";
	m_taskRunQueue = &GVideoTaskRunQueue;
}

TsVideoTransProgress& TsVideoTransProgress::GetInstance()
{
	static TsVideoTransProgress instance_;
	return instance_;
}
void TsVideoTransProgress::OnUpdateInfo(Json::Value& value,TsSession *session)
{
	Task* task;

	
	if(value["TaskType"].isNull() || value["id"].isNull() || value["progress"].isNull()){

		// TODO:  log
		return;
	}
		
	if(isThis(value["TaskType"].asString())){
		task = m_taskRunQueue->GetTaskInSpecId(value["id"].asString());
		if(!task){
			// TODO: log
			return;
		}

		if(!value["progress"].isInt()){
			// TODO: log
			return;
		}

		task->SetProgress(value["progress"].asInt());
        task->SetTaskStartRunTime();
		
	} else {
		TsUpdate::OnUpdateInfo(value,session);
	}
}

