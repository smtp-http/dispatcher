#include "TsQueue.h"

TsQueue GVideoTsQueue;
TsQueue GDocOpenofficeTsQueue;
TsQueue GDocWpsTsQueue;
TsQueue GDocOfficeTsQueue;
TsQueue GDpptTsQueue;

bool TsQueue::PushTs(TransServer * ts)
{
	m_tsList.push_back(ts);

	return true;
}

list<TransServer *> *TsQueue::GetTsLst()
{
    return &m_tsList;
}


TransServer* TsQueue::GetTsInSpecId(string tsId)
{
	list<TransServer*>::iterator it;

	for(it = m_tsList.begin();it != m_tsList.end();it ++){
		if((*it)->GetId() == tsId){
			return (TransServer *)(*it);
		}
	}

	return NULL;
}

bool TsQueue::CheckSameTs(string tsId,string tsIp)
{
	list<TransServer*>::iterator it;
	bool ret = false;

	for(it = m_tsList.begin();it != m_tsList.end();it ++){
		if((*it)->GetId() == tsId && (*it)->GetIp() == tsIp){
			ret = true;
		}
	}

	return ret;
}

bool TsQueue::DelTs(string tsId)
{
	list<TransServer*>::iterator it;
	bool ret = false;

	for(it = m_tsList.begin();it != m_tsList.end();){
		if((*it)->GetId() == tsId){
            (*it)->GivebackTs();
			m_tsList.erase(it++);
			ret = true;
		}
        else
        {
            it ++;
        }
	}

	return ret;
}

bool TsQueue::DelTs(TransServer *ts)
{
	list<TransServer*>::iterator it;
	bool ret = false;

	for(it = m_tsList.begin();it != m_tsList.end();/*it ++*/){
		if((*it) == ts){
            ts->SetState(TsIdle);
            ts->SetTask(NULL);
		    ts->SetSession(NULL);
            ts->GivebackTs();
			m_tsList.erase(it++);
			ret = true;
		} else {
            it ++;
        }
	}

	return ret;
}

TransServer* TsQueue::GetTsIdle()
{
	list<TransServer*>::iterator it;


	for(it = m_tsList.begin();it != m_tsList.end();it ++){
		if((*it)->GetState() == TsIdle){
            (*it)->SetState(TsRunning);
			return (TransServer *)(*it);
		}
	}

	return NULL;
}

VideoTransServer* TsQueue::GetTsIdle(TaskProperty property)
{
	list<TransServer*>::iterator it;
	VideoTransServer *videoTs;

	if(property == TASK_HEAVY){
		for(it = m_tsList.begin();it != m_tsList.end();it ++){
			videoTs = (VideoTransServer *)(*it);
			if(videoTs->GetState() == TsIdle && videoTs->GetTsProperty() == TASK_HEAVY){
                (*it)->SetState(TsRunning);
				return (VideoTransServer *)(*it);
			}
		}
	} else if(property == TASK_LIGHT){
		for(it = m_tsList.begin();it != m_tsList.end();it ++){
			videoTs = (VideoTransServer *)(*it);
			if(videoTs->GetState() == TsIdle && videoTs->GetTsProperty() == TASK_LIGHT){
                (*it)->SetState(TsRunning);
				return (VideoTransServer *)(*it);
			}
		}

		for(it = m_tsList.begin();it != m_tsList.end();it ++){
			videoTs = (VideoTransServer *)(*it);
			if(videoTs->GetState() == TsIdle && videoTs->GetTsProperty() == TASK_HEAVY){
                (*it)->SetState(TsRunning);
				return (VideoTransServer *)(*it);
			}
		}
	} else {
		// TODO: error  log
	}
	
	return NULL;
}

TransServer* TsQueue::GetTransServer(TsSession* session)
{
	list<TransServer*>::iterator it;

	for(it = m_tsList.begin();it != m_tsList.end();it ++){
		if((*it)->GetSession() == session)
			return *it;
	}

	return NULL;
}


unsigned int TsQueue::GetTsStatus(Json::Value& value)
{
	list<TransServer*>::iterator it;
	unsigned int i;
	string strIpTsIdPair;

	for(it = m_tsList.begin(),i = 0;it != m_tsList.end();it ++,i ++){
		strIpTsIdPair = (*it)->GetIp() + ":" + (*it)->GetId();
		if(i == 0){
			value[0u] = strIpTsIdPair;
		} else {
			value[i] = strIpTsIdPair;
		}
	}

	return i;
}


