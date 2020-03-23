#ifndef __DATA_PERSISTENCE_H__
#define __DATA_PERSISTENCE_H__

#include "IDataPersistence.h"



#define GET_ALL_FILE_CMD(n) "ls dpfile | grep " #n

#define GET_ALL_DOC_CMD     GET_ALL_FILE_CMD(DOC.)
#define GET_ALL_DPPT_CMD    GET_ALL_FILE_CMD(DPPT.)
#define GET_ALL_VIDEO_CMD  GET_ALL_FILE_CMD(VIDEO.)

class DpManager{

public:
	virtual bool CreateDpFile() = 0;
	bool GetInfoFromDpFile(DpMessageArryType &dpMsg);
	
protected:
	bool WriteInfoToFile(DpMessageArryType &dpMsg);
	
	string m_filePrefix;
	string m_dataType;
};

class ResultReTransDpManager : public DpManager
{
public:
	ResultReTransDpManager(IGetResultReTransMsg *g)
	{
		m_getResultReTransMsg = g;
	}
	virtual bool CreateDpFile();

protected:
	IGetResultReTransMsg* m_getResultReTransMsg;
};

class TransTaskDpManager : public DpManager{

public:	
	
	bool GetTaskinfoFromDpFile(DpMessageArryType &dpMsg,QueueType qt);
	
	bool GetInfoAndWriteToFile(DpMessageArryType &dpMsgArry);
	bool GetInfoAndWriteToFile(DpMessageArryType &dpMsgArry,QueueType qt);

	//virtual bool GetInfoFromSystem(DpMessageArryType &dpMsgArry) = 0;
	
	
	virtual bool CreateDpFile();
	bool CreateDpFile(QueueType qt);
	
	bool CreateDpFile(const DpMessageType &dpMsg);
	
protected:
	
	IQueueUser *m_queueUser;
	TaskType m_taskType;

	string m_fileContent;
};

class DocDpManager : public TransTaskDpManager{
public:
	DocDpManager(IQueueUser *usr)
	{
		m_taskType = DocTask;
		m_filePrefix = "DOC";
		m_dataType = "";
		m_queueUser = usr;
	}
	
	//virtual bool GetInfoFromSystem(DpMessageArryType &dpMsgArry);

};

class DpptDpManager : public TransTaskDpManager{
public:
	DpptDpManager(IQueueUser *usr)
	{
		m_taskType = DpptTask;
		m_filePrefix = "DPPT";
		m_dataType = "";
		m_queueUser = usr;
	}
	

	//virtual bool GetInfoFromSystem(DpMessageArryType &dpMsgArry);

};

class VideoDpManager : public TransTaskDpManager{
public:
	VideoDpManager(IQueueUser *usr)	
	{
		m_taskType = VideoTask;
		m_filePrefix = "VIDEO";
		m_dataType = "";
		m_queueUser = usr;
	}

	//virtual bool GetInfoFromSystem(DpMessageArryType &dpMsgArry);
};


class DataPresistence:public IDataPresistence{
protected:
	IQueueUser *m_queueUser;
	TransTaskDpManager *m_taskDpManager;
	DataPresistence(IQueueUser *queueUser);
public:
	
	
	static DataPresistence& GetInstance(IQueueUser *queueUser);

	
	//virtual IDataPresistence &CreatePresistence(IQueueUser *user);
	virtual bool DpGetMessage(DpMessageArryType &dpMsg,TaskType taskType);
	virtual bool DpGetWaitMessage(DpMessageArryType &dpMsg,TaskType taskType);
	virtual bool DpGetRunMessage(DpMessageArryType &dpMsg,TaskType taskType);
	virtual bool CreateDpMsgFile(TaskType taskType);
	virtual bool CreateDpMsgFile(const DpMessageType &dpMsg,TaskType taskType);
	virtual bool CreateWaitMsgFile(TaskType taskType);
	virtual bool CreateRunMsgFile(TaskType taskType);
	
};

#endif

