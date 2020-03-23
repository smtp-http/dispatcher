#include "GetTransServer.h"

extern TsQueue GDocOpenofficeTsQueue;
extern TsQueue GDocWpsTsQueue;
extern TsQueue GDocOfficeTsQueue;


////////////////////////////////////StrategyForGetDocTs/////////////////////////////////////
StrategyForGetDocTs::StrategyForGetDocTs()
{
    m_task_type = DocTask;


    m_tsQueues["linux"] = &GDocOpenofficeTsQueue;
	m_tsQueues["wps"] = &GDocWpsTsQueue;
	m_tsQueues["office"] = &GDocOfficeTsQueue;

    m_tstypeTostr[SERVER_LINUX] = "linux";
    m_tstypeTostr[SERVER_WPS] = "wps";
    m_tstypeTostr[SERVER_OFFICE] = "office";
    
}

StrategyForGetDocTs& StrategyForGetDocTs::GetInstance()
{
	static  StrategyForGetDocTs instance_;
	return instance_; 	
}

void StrategyForGetDocTs::SpacificStrategyForGetTs(Task * task,TransServer* &ts)
{
    
    if(IsThisStrategy(task))
    {
        TsQueue* tsq;
    	TransServer* tmpts = NULL;
    	int i;
        vector<string>::iterator itor;
        map<int,DocTsType> tsPriority = (dynamic_cast<TaskDocument *>(task))->GetTsPriority();
        
        DocTsType appintedts = (dynamic_cast<TaskDocument *>(task))->GetAppiontedTS();
        //cout<<"get appiontedts :"<<appintedts<<endl;
        for(i = 0;i < tsPriority.size();i ++){
            if((appintedts != tsPriority[i])&&(appintedts != SERVER_NOAPPOINTED))
                continue;
        	tsq = m_tsQueues[m_tstypeTostr[tsPriority[i]]];
        	tmpts = tsq->GetTsIdle();

        	if(tmpts){
        		task->m_alreadyTriedTsType.push_back(i);
        		task->m_alreadyTriedTsId.push_back(tmpts->GetId());
        		break;
        	}
        }
        ts = tmpts;
        //cout<<"get ts :"<<ts<<endl;
    }
    else
    {
        StrategyForGetTs::SpacificStrategyForGetTs(task,ts);
    }
}


////////////////////////////////////GetTransServer///////////////////////////////////
GetTransServer& GetTransServer::GetInstance()
{
	static  GetTransServer instance_;
	return instance_; 	
}

GetTransServer::GetTransServer()
{
	m_strategyForGetTs = &StrategyForGetDocTs::GetInstance();
	//StrategyForGetVideoTs & strategyForGetVideoTs = StrategyForGetVideoTs::GetInstance();
	
	//m_strategyForGetTs->add(&strategyForGetVideoTs);
}
void GetTransServer::GetTS(Task * task,TransServer * &ts)
{
    m_strategyForGetTs->SpacificStrategyForGetTs(task,ts);
}