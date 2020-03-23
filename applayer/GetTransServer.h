#ifndef __GET_TRANS_SERVER_H__
#define __GET_TRANS_SERVER_H__
#include "TransServer.h"
#include "TsQueue.h"
#include "Log4cxxWrapper.h"




class StrategyForGetTs
{
	StrategyForGetTs *next;

protected:
	TaskType m_task_type;
	map<string,TsQueue*>m_tsQueues;
	
public:
	StrategyForGetTs()
	{
	}
	
	bool IsThisStrategy(Task * task)
	{
		if(task->m_tasktype == m_task_type)
			return true;
		else
			return false;
	}
	
		
	void setNext(StrategyForGetTs *n)
	{
		next = n;
	}
	void add(StrategyForGetTs *n)
	{
		if (next)
			next->add(n);
		else
			next = n;
	}
	virtual void SpacificStrategyForGetTs(Task * task,TransServer* &ts)
	{
		LoggerWrapper log= LoggerWrapper::GetInstance();
		if(next != NULL)
			next->SpacificStrategyForGetTs(task,ts);
		else {
			
			log.Log(true,LOGGER_ERROR,"SpacificStrategyForGetTs next null! [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		}
	}

	
};

class StrategyForGetDocTs:public StrategyForGetTs
{
	map<int,DocTsType> m_tsPriority;
	map<DocTsType,string> m_tstypeTostr;
	StrategyForGetDocTs();
public:
	static StrategyForGetDocTs& GetInstance();
	virtual void SpacificStrategyForGetTs(Task * task,TransServer* &ts);
};

/*
class StrategyForGetVideoTs:public StrategyForGetTs
{

	StrategyForGetVideoTs();
public:

	
	static StrategyForGetVideoTs& GetInstance();
	
	virtual void SpacificStrategyForGetTs(Task * task,TransServer* ts);
};

*/
class GetTransServer
{
	StrategyForGetTs * m_strategyForGetTs;
	GetTransServer();
	/*
	{
		m_strategyForGetTs = &StrategyForGetDocTs::GetInstance();
		//StrategyForGetVideoTs & strategyForGetVideoTs = StrategyForGetVideoTs::GetInstance();
		
		//m_strategyForGetTs->add(&strategyForGetVideoTs);
	}
	*/
public:
	static GetTransServer& GetInstance();
	virtual void GetTS(Task * task,TransServer * &ts);
	
};


#endif
