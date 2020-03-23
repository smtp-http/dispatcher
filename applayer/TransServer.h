#ifndef __TRANS_SERVER_H__
#define __TRANS_SERVER_H__
#include <string>
#include <list>
#include "Task.h"
using namespace std;

class TsSession;

typedef enum{
	TsIdle = 0,
	TsRunning,
	TsAllState = 255
}TsState;

typedef enum{
	DocTrans = 0,
	VideoTrans,
	DpptTrans
}ServerType;

/*
typedef enum{
	OpenofficeDocTrans = 0,
	WpsDocTrans,
	OfficeDocTrans
}DocTransType;
*/

class TransServer
{
protected:
	class TransServerDb *m_tsDb;
public:
	virtual ~TransServer(){}
	TransServer(){m_tsState = TsIdle; m_task = NULL;}
	string GetId()
	{
		return m_tsId;
	}

	void SetId(string id)
	{
		m_tsId = id;
	}

	string GetIp()
	{
		return m_tsIp;
	}

	void SetIp(string ip)
	{
		m_tsIp = ip;
	}

	ServerType GetServerType()
	{
		return m_serverType;
	}

	TsState GetState()
	{
		return m_tsState;
	}

	void SetState(TsState state)
	{
		m_tsState = state;
	}

	void SetSession(TsSession* session)
	{
		m_session = session;
	}

	TsSession* GetSession()
	{
		return m_session;
	}

	void Release()
	{
	}

	void SetTask(Task* task)
	{
		m_task = task;
	}

	Task* GetTask()
	{
		return m_task;
	}

	void GivebackTs();
private:
	string m_tsId;
	string m_tsIp;

	
protected:
	TsSession* m_session;
	ServerType m_serverType;
	TsState m_tsState;
	Task* m_task;
};

class DocTransServer : public TransServer
{

public:
	DocTransServer();
	DocTransServer(string& id);
	/*
	void SetDocTransType(DocTransType docTransType);
	DocTransType GetDocTransType();
	*/
	void SetDocTsType(DocTsType ts_type);
	DocTsType GetDocTsType();

private:
	DocTsType m_docTsType;
};


class VideoTransServer : public TransServer
{
	TaskProperty m_property;
public:
	VideoTransServer();
	VideoTransServer(string& id);
	void SetTsProperty(string propertystr);
	TaskProperty GetTsProperty();
};

class DpptTransServer : public TransServer
{
public:
	DpptTransServer();
	DpptTransServer(string& id);
};


class TransServerDb
{
public:
	virtual TransServer* GetTransServer() = 0;
	void TransServerInit(TransServer *ts){ts->SetTask(NULL);}

	bool GiveBackTransServer(TransServer *ts)
	{
		list<TransServer*>::iterator it;  
		for(it=m_transServerBuf.begin();it!=m_transServerBuf.end();it++)  {  
			if(*it == ts)
				return false;
		}
		
		//ts->Release();
		TransServerInit(ts);
		m_transServerBuf.push_back(ts);

		return true;
	}

	unsigned int GetDbSize()
	{
		return m_transServerBuf.size();
	}
protected:
	list<TransServer*> m_transServerBuf;
};

class DocTransServerDb : public TransServerDb
{
	DocTransServerDb(){}
public:
	static DocTransServerDb& GetInstance()
	{
		static DocTransServerDb instance_;
		return instance_;
	}
	virtual TransServer* GetTransServer()
	{
		TransServer* ts;
		if(!m_transServerBuf.empty()){
			ts = m_transServerBuf.front();
			m_transServerBuf.pop_front();
			
		} else{
			ts =  new DocTransServer;	
		}

		TransServerInit(ts);
		return ts;
	}
};

class VideoTransServerDb : public TransServerDb
{
	VideoTransServerDb(){}
public:
	static VideoTransServerDb& GetInstance()
	{
		static VideoTransServerDb instance_;
		return instance_;
	}
	virtual TransServer* GetTransServer()
	{
		TransServer* ts;
		if(!m_transServerBuf.empty()){
			ts = m_transServerBuf.front();
			m_transServerBuf.pop_front();
		} else {
			ts = new VideoTransServer;		
		}

		TransServerInit(ts);
		return ts;
	}
};

class DpptTransServerDb : public TransServerDb
{
	DpptTransServerDb(){}
public:
	static DpptTransServerDb& GetInstance()
	{
		static DpptTransServerDb instance_;
		return instance_;
	}
	virtual TransServer* GetTransServer()
	{
		TransServer* ts;
		if(!m_transServerBuf.empty()){
			ts = m_transServerBuf.front();
			m_transServerBuf.pop_front();
		} else {
			ts = new DpptTransServer;		
		}

		TransServerInit(ts);
		return ts;
	}

};

#endif
