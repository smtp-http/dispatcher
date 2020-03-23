#include "Command.h"
#include "TsManager.h"

extern TaskWaitQueue GDocTaskWaitQueue;
extern TaskWaitQueue GVideoTaskWaitQueue;
extern TaskWaitQueue GDpptTaskWaitQueue;
extern TaskWaitQueue GNoTsTaskTaskWaitQueue;


extern TaskRunQueue GDocTaskRunQueue;
extern TaskRunQueue GVideoTaskRunQueue;
extern TaskRunQueue GDpptTaskRunQueue;
extern TaskRunQueue GNoTsTaskRunQueue;


extern  TsQueue GVideoTsQueue;
extern  TsQueue GDocOpenofficeTsQueue;
extern  TsQueue GDocWpsTsQueue;
extern  TsQueue GDocOfficeTsQueue;
extern  TsQueue GDpptTsQueue;
/////////////////////// TsRegister ////////////////////

TsRegister::TsRegister()
{
	m_addTransServer = &AddOpenofficeTransServer::GetInstance();
	m_addTransServer->add(&AddWpsTransServer::GetInstance());
	m_addTransServer->add(&AddOfficeTransServer::GetInstance());
	m_addTransServer->add(&AddVideoTransServer::GetInstance());
	m_addTransServer->add(&AddDpptTransServer::GetInstance());
}

void TsRegister::OnRegister(Json::Value& value,TsSession *session)
{
	m_addTransServer->AddSpacificTransServer(value,session);
}

////////////////////// AddTransServer /////////////////////
AddTransServer::AddTransServer()
{

}
void AddTransServer::AddTsTransaction(Json::Value & value, TsSession* session)
{
	TsRegistMessage* regMessage;
	TsMessage* message;
	int errorCode;
	string errorDetail;
	string msg;
    Task * waittask;
	
	message = TsMessageDb::GetInstance().GetMessage(TsRegistMsg,m_serverType);
	if(!message){
		// TODO:log
		return;
	}
	
	if(regMessage = dynamic_cast<TsRegistMessage *>(message)){
		
	} else {
		// TODO:log
		return;
	}
	
	
	if(m_tsQueue->CheckSameTs(value["id"].asString(),value["ip"].asString())){
		errorCode = 1;  // TODO:
		errorDetail = "Repeat ts id!";
		regMessage->BuildResponse(errorCode,errorDetail,msg);
		session->SendMessage(msg);
	} else {

		TransServer* ts = m_tsDb->GetTransServer();//VideoTransServerDb::GetInstance().GetTransServer();
		ts->SetId(value["id"].asString());
		ts->SetIp(value["ip"].asString());
        ts->SetSession(session);
		m_tsQueue->PushTs(ts);
		errorCode = 0;
		errorDetail = "";
		regMessage->BuildResponse(errorCode,errorDetail,msg);
		session->SendMessage(msg);

        waittask = m_taskWaitQueue->PopTask();
        if(waittask)
        {
		    m_disptacher->Submit(waittask);
        }
		
	}

}

///////////////////// AddDocTransServer //////////////////

AddDocTransServer::AddDocTransServer()
{
	m_tsDb = &DocTransServerDb::GetInstance();
    m_disptacher = &DocDisptacher::GetInstance();
    m_taskWaitQueue = &GDocTaskWaitQueue;
    m_docTsTypeMap["DOC"]           = SERVER_LINUX;
    m_docTsTypeMap["DOC_WPS"]       = SERVER_WPS;
    m_docTsTypeMap["DOC_WINDOWS"]   = SERVER_OFFICE;
}

void AddDocTransServer::AddTsTransaction(Json::Value & value, TsSession* session)
{
	TsRegistMessage* regMessage;
	TsMessage* message;
	int errorCode;
	string errorDetail;
	string msg;
    Task * waittask;
	
	message = TsMessageDb::GetInstance().GetMessage(TsRegistMsg,m_serverType);
	if(!message){
		// TODO:log
		return;
	}
	
	if(regMessage = dynamic_cast<TsRegistMessage *>(message)){
		
	} else {
		// TODO:log
		return;
	}
	
	
	if(m_tsQueue->CheckSameTs(value["id"].asString(),value["ip"].asString())){
		errorCode = 1;  // TODO:
		errorDetail = "Repeat ts id!";
		regMessage->BuildResponse(errorCode,errorDetail,msg);
		session->SendMessage(msg);
	} else {

		TransServer* ts = m_tsDb->GetTransServer();//VideoTransServerDb::GetInstance().GetTransServer();
		ts->SetId(value["id"].asString());
		ts->SetIp(value["ip"].asString());
        ts->SetSession(session);
        if(!value["servertype"].isNull() && value["servertype"].isString())
        {
            (dynamic_cast<DocTransServer *>(ts))->SetDocTsType(m_docTsTypeMap[value["servertype"].asString()]);
        }
		m_tsQueue->PushTs(ts);
		errorCode = 0;
		errorDetail = "";
		regMessage->BuildResponse(errorCode,errorDetail,msg);
		session->SendMessage(msg);

        
        waittask = m_taskWaitQueue->PopRightTaskForDocTs((dynamic_cast<DocTransServer *>(ts))->GetDocTsType());
        if(waittask)
        {
		    m_disptacher->Submit(waittask);
        }
		
	}

}


void AddDocTransServer::AddSpacificTransServer(Json::Value& value,TsSession*session)
{

}

///////////////////AddOpenofficeTransServer /////////////////
AddOpenofficeTransServer::AddOpenofficeTransServer()
{
	m_tsQueue = &GDocOpenofficeTsQueue;
	m_serverType = "DOC";
}

AddOpenofficeTransServer& AddOpenofficeTransServer::GetInstance()
{
	static  AddOpenofficeTransServer instance_;
	return instance_; 	
}

void AddOpenofficeTransServer::AddSpacificTransServer(Json::Value& value,TsSession*session)
{
	
	if(value["servertype"] == m_serverType){
		AddTsTransaction(value,session);

	} else {
		AddTransServer::AddSpacificTransServer(value,session);
	}
}

///////////////////AddWpsTransServer /////////////////
AddWpsTransServer::AddWpsTransServer()
{
	m_tsQueue = &GDocWpsTsQueue;
	m_serverType = "DOC_WPS";
}

AddWpsTransServer& AddWpsTransServer::GetInstance()
{
	static  AddWpsTransServer instance_;
	return instance_; 	
}

void AddWpsTransServer::AddSpacificTransServer(Json::Value& value,TsSession*session)
{
	
	if(value["servertype"] == m_serverType){
		
		AddTsTransaction(value,session);

	} else {
		AddTransServer::AddSpacificTransServer(value,session);
	}
}

///////////////////AddOfficeTransServer /////////////////
AddOfficeTransServer::AddOfficeTransServer()
{
	m_tsQueue = &GDocOfficeTsQueue;
	m_serverType = "DOC_WINDOWS";
}

AddOfficeTransServer& AddOfficeTransServer::GetInstance()
{
	static  AddOfficeTransServer instance_;
	return instance_; 	
}

void AddOfficeTransServer::AddSpacificTransServer(Json::Value& value,TsSession*session)
{

	if(value["servertype"] == m_serverType){
		AddTsTransaction(value,session);

	} else {
		AddTransServer::AddSpacificTransServer(value,session);
	}
}

//////////////////// AddVideoTransServer ///////////////////

AddVideoTransServer::AddVideoTransServer()
{
	m_tsQueue = &GVideoTsQueue;
	m_serverType = "VIDEO";
	m_tsDb = &VideoTransServerDb::GetInstance();
    m_disptacher = &VideoDisptacher::GetInstance();
    m_taskWaitQueue = &GVideoTaskWaitQueue;
}

AddVideoTransServer& AddVideoTransServer::GetInstance()
{
	static  AddVideoTransServer instance_;
	return instance_; 	
}

void AddVideoTransServer::AddTsTransaction(Json::Value & value, TsSession* session)
{
	TsRegistMessage* regMessage;
	TsMessage* message;
	int errorCode;
	string errorDetail;
	string msg;
    Task * waittask;
	
	message = TsMessageDb::GetInstance().GetMessage(TsRegistMsg,m_serverType);
	if(!message){
		// TODO:log
		return;
	}
	
	if(regMessage = dynamic_cast<TsRegistMessage *>(message)){
		
	} else {
		// TODO:log
		return;
	}
	
	
	if(m_tsQueue->CheckSameTs(value["id"].asString(),value["ip"].asString())){
		errorCode = 1;  // TODO:
		errorDetail = "Repeat ts id!";
        regMessage->BuildResponse(errorCode,errorDetail,msg);
		session->SendMessage(msg);
	} else {

		TransServer* ts = m_tsDb->GetTransServer();//VideoTransServerDb::GetInstance().GetTransServer();
		ts->SetId(value["id"].asString());
		ts->SetIp(value["ip"].asString());
        ts->SetSession(session);
        //video ts need set property
        ((VideoTransServer*)ts)->SetTsProperty(value["servertype"].asString());
        m_tsQueue->PushTs(ts);
		errorCode = 0;
		errorDetail = "";
		regMessage->BuildResponse(errorCode,errorDetail,msg);
		session->SendMessage(msg);

        waittask = m_taskWaitQueue->PopTask();
        if(waittask)
        {
		    m_disptacher->Submit(waittask);
        }
		
	}

}


void AddVideoTransServer::AddSpacificTransServer(Json::Value& value,TsSession* session)
{
    string server_type_str = value["servertype"].asString();
    string::size_type index = server_type_str.find( m_serverType );
    if ( index != string::npos )
    {
		AddTsTransaction(value,session);

	} else {
		AddTransServer::AddSpacificTransServer(value,session);
	}
}

////////////////////// AddDpptTransServer //////////////////

AddDpptTransServer::AddDpptTransServer()
{
	m_tsQueue = &GDpptTsQueue;
	m_serverType = "DYNAMIC_PPT";
	m_tsDb = &DpptTransServerDb::GetInstance();
    m_disptacher = &DpptDisptacher::GetInstance();
    m_taskWaitQueue = &GDpptTaskWaitQueue;
}

AddDpptTransServer& AddDpptTransServer::GetInstance()
{
	static  AddDpptTransServer instance_;
	return instance_; 	
}

void AddDpptTransServer::AddSpacificTransServer(Json::Value& value,TsSession*session)
{
	if(value["servertype"] == m_serverType){
		AddTsTransaction(value,session);

	} else {
		AddTransServer::AddSpacificTransServer(value,session);
	}
}

///////////////////// TsServerLeave  ////////////////////////////

TransServerLeave::TransServerLeave()
{
	m_tsLeaveHandler = &TsVideoLeaveHandler::GetInstance();
	m_tsLeaveHandler->add(&TsOpenofficeLeaveHandler::GetInstance());
	m_tsLeaveHandler->add(&TsOfficeLeaveHandler::GetInstance());
	m_tsLeaveHandler->add(&TsWpsLeaveHandler::GetInstance());
	m_tsLeaveHandler->add(&TsDpptLeaveHandler::GetInstance());
}

TransServerLeave& TransServerLeave::GetInstance()
{
	static  TransServerLeave instance_;
	return instance_; 	
}

void TransServerLeave::OnServerLeave(TsSession &session)
{
    m_tsLeaveHandler ->SpacificTsLeave(&session);
}

//////////////////////////////// TsLeaveHandler //////////////////////////////
TransServer* TsLeaveHandler::FindSession(TsSession* session)
{
	return m_tsQueue->GetTransServer(session);
}

//////////////////////////////// TsDocLeaveHandler //////////////////////////////

TsDocLeaveHandler::TsDocLeaveHandler()
{
	m_taskWaitQueue = &GDocTaskWaitQueue;
}


////////////////////////// TsOpenofficeLeaveHandler ///////////////////////////
TsOpenofficeLeaveHandler::TsOpenofficeLeaveHandler()
{
	m_tsQueue = &GDocOpenofficeTsQueue;
	m_tsDb = &DocTransServerDb::GetInstance();
	m_disptacher = &DocDisptacher::GetInstance();
    m_taskRunQueue = &GDocTaskRunQueue; 
}

TsOpenofficeLeaveHandler& TsOpenofficeLeaveHandler::GetInstance()
{
	static TsOpenofficeLeaveHandler instance_;
	return instance_;
}


void TsOpenofficeLeaveHandler::SpacificTsLeave(TsSession* session)
{
	TransServer* ts;
	
	ts = FindSession(session);
	if(ts != NULL){
		ProcessTsAndTask(ts);
	} else {
		TsLeaveHandler::SpacificTsLeave(session);
	}
}


////////////////////////// TsOfficeLeaveHandler ///////////////////////////
TsOfficeLeaveHandler::TsOfficeLeaveHandler()
{
	m_tsQueue = &GDocOfficeTsQueue;
	m_tsDb = &DocTransServerDb::GetInstance();
	m_disptacher = &DocDisptacher::GetInstance();
    m_taskRunQueue = &GDocTaskRunQueue; 
}

TsOfficeLeaveHandler& TsOfficeLeaveHandler::GetInstance()
{
	static TsOfficeLeaveHandler instance_;
	return instance_;
}


void TsOfficeLeaveHandler::SpacificTsLeave(TsSession* session)
{
	TransServer* ts;
	
	if(ts = FindSession(session)){
		ProcessTsAndTask(ts);
	} else {
		TsLeaveHandler::SpacificTsLeave(session);
	}
}

////////////////////////// TsWpsLeaveHandler ///////////////////////////
TsWpsLeaveHandler::TsWpsLeaveHandler()
{
	m_tsQueue = &GDocWpsTsQueue;
	m_tsDb = &DocTransServerDb::GetInstance();
	m_disptacher = &DocDisptacher::GetInstance();
    m_taskRunQueue = &GDocTaskRunQueue; 
}

TsWpsLeaveHandler& TsWpsLeaveHandler::GetInstance()
{
	static TsWpsLeaveHandler instance_;
	return instance_;
}



void TsWpsLeaveHandler::SpacificTsLeave(TsSession* session)
{
	TransServer* ts;
	
	if(ts = FindSession(session)){
		ProcessTsAndTask(ts);
	} else {
		TsLeaveHandler::SpacificTsLeave(session);
	}
}



/////////////////////////// TsVideoLeaveHandler //////////////////////////////
TsVideoLeaveHandler::TsVideoLeaveHandler()
{
	m_tsQueue = &GVideoTsQueue;
	m_tsDb = &VideoTransServerDb::GetInstance();
	m_disptacher = &VideoDisptacher::GetInstance();
    m_taskRunQueue = &GVideoTaskRunQueue;
    m_taskWaitQueue = &GVideoTaskWaitQueue;
}

TsVideoLeaveHandler& TsVideoLeaveHandler::GetInstance()
{
	static TsVideoLeaveHandler instance_;
	return instance_;
}


void TsVideoLeaveHandler::SpacificTsLeave(TsSession* session)
{
	TransServer* ts;
	
	if(ts = FindSession(session)){
		ProcessTsAndTask(ts);
	} else {
		TsLeaveHandler::SpacificTsLeave(session);
	}
}

/////////////////////////// TsDpptLeaveHandler ////////////////////////////////
TsDpptLeaveHandler::TsDpptLeaveHandler()
{
	m_tsQueue = &GDpptTsQueue;
	m_tsDb = &DpptTransServerDb::GetInstance();
	m_disptacher = &DpptDisptacher::GetInstance();
    m_taskRunQueue = &GDpptTaskRunQueue;
    m_taskWaitQueue = &GDpptTaskWaitQueue;
}

TsDpptLeaveHandler& TsDpptLeaveHandler::GetInstance()
{
	static TsDpptLeaveHandler instance_;
	return instance_;
}


void TsDpptLeaveHandler::SpacificTsLeave(TsSession* session)
{
	TransServer* ts;
	
	if(ts = FindSession(session)){
		ProcessTsAndTask(ts);
	} else {
		TsLeaveHandler::SpacificTsLeave(session);
	}
}

