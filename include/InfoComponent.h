#ifndef __INFO_COMPONENT_H__
#define __INFO_COMPONENT_H__

#include "IDataPersistence.h"

class QueueInfo
{
public:
	QueueInfo();
	virtual ~QueueInfo();
	virtual bool GetWaitQueueMessage(DpMessageArryType &dpMsgArry,TaskType taskType);
	virtual bool GetRunQueueMessage(DpMessageArryType &dpMsgArry,TaskType taskType);
};


class QueueInfoAdapter : public QueueInfo
{
public:
	QueueInfoAdapter(IQueueUser &);
	~QueueInfoAdapter();
	virtual bool GetWaitQueueMessage(DpMessageArryType &dpMsgArry,TaskType taskType);
	virtual bool GetRunQueueMessage(DpMessageArryType &dpMsgArry,TaskType taskType);
private:
	 IQueueUser *m_queueUser;
};


//InfoComponent�����࣬����������Ľӿ�
class InfoComponent
{
public:
	virtual ~InfoComponent();
	virtual bool Operation()=0;
	DpMessageArryType *GetMsgArry();
protected:
	DpMessageArryType *m_msgArry;
	InfoComponent();
};

//ConcreteInfoAdder�������InfoComponent���󣬿��Ը��ö���̬ ���ְ��
class ConcreteInfoComponent:public InfoComponent
{
public:
//	ConcreteInfoComponent();
	ConcreteInfoComponent(DpMessageArryType &msgArray);
	~ConcreteInfoComponent();
	virtual bool Operation();
};

//InfoAdder��װ�γ����࣬�̳���InfoComponent
class InfoAdder:public InfoComponent
{
public:
	InfoAdder(InfoComponent* com);
	void SetInfoComponent(InfoComponent* com);
	virtual ~InfoAdder();
	virtual bool Operation();
protected:
	InfoComponent* _com;
};

class RunQueueInfoAdder : public InfoAdder
{
public:
	RunQueueInfoAdder(InfoComponent* com,QueueInfo &,TaskType taskType);
	~RunQueueInfoAdder();
	virtual bool Operation();
	bool AddBehavor();
private:
	TaskType m_taskType;
	QueueInfo *m_queueInfo;
};


class WaitQueueInfoAdder : public InfoAdder
{
public:
	WaitQueueInfoAdder(InfoComponent* com,QueueInfo &,TaskType taskType);
	~WaitQueueInfoAdder();
	virtual bool Operation();
	bool AddBehavor();
private:
	TaskType m_taskType;
	QueueInfo *m_queueInfo;
};



#endif
