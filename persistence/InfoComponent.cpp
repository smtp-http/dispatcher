#include "InfoComponent.h"

#include <iostream>

#include "Log4cxxWrapper.h"
#include "Log4cxxWrapper.h"

using namespace std;


///////////////////////////////// QueueInfo ////////////////////////////////////

QueueInfo::QueueInfo()
{

}

QueueInfo::~QueueInfo()
{

}

bool QueueInfo::GetWaitQueueMessage(DpMessageArryType &dpMsgArry,TaskType taskType)
{
	cout <<  "++++++++++++ QueueInfo::GetWaitQueueMessage"  << endl;
	return true;
}

bool QueueInfo::GetRunQueueMessage(DpMessageArryType &dpMsgArry,TaskType taskType)
{
	cout <<  "++++++++++++ QueueInfo::GetRunQueueMessage"  << endl;
	return true;
}



//////////////////////////////// QueueInfoAdapter //////////////////////////////


QueueInfoAdapter::QueueInfoAdapter(IQueueUser &queueUser)
{
	m_queueUser = &queueUser;
}

QueueInfoAdapter::~QueueInfoAdapter()
{

}

bool QueueInfoAdapter::GetWaitQueueMessage(DpMessageArryType &dpMsgArry,TaskType taskType)
{
	switch(taskType){
		case DocTask:
			return m_queueUser->GetDocWaitQueueMessage(dpMsgArry);
		case VideoTask:
			return m_queueUser->GetVideoWaitQueueMessage(dpMsgArry);
		case DpptTask:
			return m_queueUser->GetDpptWaitQueueMessage(dpMsgArry);
		default:
			return false;
	}
}

bool QueueInfoAdapter::GetRunQueueMessage(DpMessageArryType &dpMsgArry,TaskType taskType)
{
	switch(taskType){
		case DocTask:
			return m_queueUser->GetDocRunQueueMessage(dpMsgArry);
		case VideoTask:
			return m_queueUser->GetVideoRunQueueMessage(dpMsgArry);
		case DpptTask:
			return m_queueUser->GetDpptRunQueueMessage(dpMsgArry);
		default:
			return false;
	}
}


//////////////////////////////// InfoComponent /////////////////////////////
InfoComponent::InfoComponent()
{

}

InfoComponent::~InfoComponent()
{
	cout << "~InfoComponent" << endl;
}

DpMessageArryType *InfoComponent::GetMsgArry()
{
	return m_msgArry;
}

//////////////////////////////// ConcreteInfoComponent /////////////////////////////
ConcreteInfoComponent::ConcreteInfoComponent(DpMessageArryType &msgArray)
{
	m_msgArry = &msgArray;
}

ConcreteInfoComponent::~ConcreteInfoComponent()
{
	cout << "~ConcreteInfoComponent" << endl;
}

bool ConcreteInfoComponent::Operation()
{
	//cout << "ConcreteInfoComponent::Operation" << endl;
	return true;
}


//////////////////////////////// InfoAdder /////////////////////////////
InfoAdder::InfoAdder(InfoComponent* com)
{
	this->_com = com;
}

void InfoAdder::SetInfoComponent(InfoComponent* com)
{
	this->_com = com;
}

InfoAdder::~InfoAdder()
{
	cout << "~InfoAdder" << endl;
	delete this->_com;
	this->_com = NULL;
}

bool InfoAdder::Operation()
{

}

//////////////////////////////// RunQueueInfoAdder /////////////////////////////

RunQueueInfoAdder::RunQueueInfoAdder(InfoComponent* com,QueueInfo &queueInfo,TaskType taskType):InfoAdder(com)
{
	m_queueInfo = &queueInfo;
	m_taskType = taskType;
	this->m_msgArry = com->GetMsgArry();
}

RunQueueInfoAdder::~RunQueueInfoAdder()
{
	cout << "~RunQueueInfoAdder" << endl;
}

bool RunQueueInfoAdder::Operation()
{
	bool ret;
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();
	
	ret = this->_com->Operation();
	if(!ret){
		dsLog.Log(true,LOGGER_ERROR,"%s:%s:%d   operation error.\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}
	

	ret = this->AddBehavor();
	if(!ret){
		dsLog.Log(true,LOGGER_ERROR,"%s:%s:%d   AddBehavor error.\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	return true;
}

bool RunQueueInfoAdder::AddBehavor()
{
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();
	cout << "============== m_msgArry: " << m_msgArry << "m_taskType: " << m_taskType << endl;
	if(!m_queueInfo->GetRunQueueMessage(*m_msgArry,m_taskType)){
		dsLog.Log(true,LOGGER_ERROR,"%s:%s:%d   GetDpptRunQueueMessage error.\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	return true;
}


//////////////////////////////// WaitQueueInfoAdder /////////////////////////////

WaitQueueInfoAdder::WaitQueueInfoAdder(InfoComponent* com,QueueInfo &queueInfo,TaskType taskType):InfoAdder(com)
{
	m_queueInfo = &queueInfo;
	m_taskType = taskType;
	this->m_msgArry = com->GetMsgArry();
}

WaitQueueInfoAdder::~WaitQueueInfoAdder()
{
	cout << "~RunQueueInfoAdder" << endl;
}

bool WaitQueueInfoAdder::Operation()
{
	this->_com->Operation();
	this->AddBehavor();
}

bool WaitQueueInfoAdder::AddBehavor()
{
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();

	if(!m_queueInfo->GetWaitQueueMessage(*m_msgArry,m_taskType)){
		dsLog.Log(true,LOGGER_ERROR,"%s:%s:%d   GetWaitQueueMessage error.\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////////


