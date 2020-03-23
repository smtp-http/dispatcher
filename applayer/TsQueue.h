#ifndef __TS_QUEUE_H__
#define __TS_QUEUE_H__

#include <list>
#include "TransServer.h"
#include "json/json.h"

class TsQueue
{
public:
	bool PushTs(TransServer* ts);
	list<TransServer *> *GetTsLst();
	TransServer* GetTsInSpecId(string tsId);
	bool DelTs(string tsId);
	bool DelTs(TransServer *ts);
	bool CheckSameTs(string tsId,string tsIp);

	TransServer* GetTsIdle();
	VideoTransServer* GetTsIdle(TaskProperty property);
	
	TransServer* GetTransServer(TsSession* session);

	unsigned int GetTsStatus(Json::Value& value);
private:
	list<TransServer *> m_tsList;
};



#endif
