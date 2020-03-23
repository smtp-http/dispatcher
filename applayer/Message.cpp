#include "Message.h"
#include <memory>.
#include "json/json.h"
#include "Log4cxxWrapper.h"
#include "TsManager.h"
#include "InterfaceApi.h"

int appmsgtotaltimes = 0;

using namespace std;

TsResultMessage  tsDpptResult("DYNAMIC_PPT");
TsResultMessage  tsDocWindowsResult("DOC_WINDOWS");
TsResultMessage  tsDocResult("DOC");
TsResultMessage  tsDocWpsResult("DOC_WPS");
TsResultMessage  tsTransferResult("Transfer");
TsResultMessage  tsVideoCutResult("VideoCut");
TsResultMessage  tsVideoMergeResult("VideoMerge");
TsResultMessage  tsCapVideoResult("CapVideo");
TsResultMessage  tsMessageResult("Message");
TsRegistMessage  tsDocWpsRegistMessage("DOC_WPS");
TsRegistMessage  tsDocOfficeRegistMessage("DOC_WINDOWS");
TsRegistMessage  tsDocLinuxRegistMessage("DOC");
TsRegistMessage  tsVideoRegistMessage("VIDEO");
TsRegistMessage  tsDpptRegistMessage("DYNAMIC_PPT");


//TsLinkResponseMessage tsLinkResponseMessage("");
TsUpdateMessage tsUpdateMessage("");

//////////////////////////////// TsRegistMessage /////////////////////////////////////////

bool TsRegistMessage::BuildResponse(int errCode,const string& errorDetial,string& response)
{
	Json::Value root;

	stringstream ss;
	ss<<errCode; 
	string strErrCode = ss.str();
	
	root["errorcode"] = strErrCode;
	root["errordetail"] = errorDetial;
	root["type"] = "RegisterResponse";
	
	if(0 == errCode){
		root["result"] = "Success";
	} else {
		root["result"] = "Fail";
	}

	response = root.toStyledString();
	
	return true;
}

TsMessageDb::TsMessageDb()
{
	AddMessage(&tsDpptResult);
	AddMessage(&tsDocWindowsResult);
	AddMessage(&tsDocResult);
	AddMessage(&tsDocWpsResult);
	AddMessage(&tsTransferResult);
	AddMessage(&tsVideoCutResult);
	AddMessage(&tsVideoMergeResult);
	AddMessage(&tsCapVideoResult);
	AddMessage(&tsMessageResult);
	AddMessage(&tsDocWpsRegistMessage);
	AddMessage(&tsDocOfficeRegistMessage);
	AddMessage(&tsDocLinuxRegistMessage);
	AddMessage(&tsVideoRegistMessage);
	AddMessage(&tsDpptRegistMessage);
//	AddMessage(&tsLinkResponseMessage);
	AddMessage(&tsUpdateMessage);
}

bool TsMessageDb::AddMessage(TsMessage * msg)
{
	m_tsMessages.push_back(msg);
	return true;
}

bool TsMessageDb::GetServerTypeInSpecificType(TsMsgType msgType, vector < string > & tsMessages)
{
	vector<TsMessage *>::iterator t ;
	for(t=m_tsMessages.begin(); t!=m_tsMessages.end(); t++){
		if((*t)->GetTsMsgType() == msgType)
			tsMessages.push_back((*t)->GetServerType());
	}
	
	return true;
}

TsMessage* TsMessageDb::GetMessage(TsMsgType msgType, string serverType)
{
	TsMessage* message = NULL;
	vector<TsMessage *>::iterator t ;
	for(t=m_tsMessages.begin(); t!=m_tsMessages.end(); t++){
		if((*t)->GetTsMsgType() == msgType && (*t)->GetServerType() == serverType){
			message = *t;
			break;
		}
	}
	
	return message;
}
//////////////////////////////// TsMessageHandle ////////////////////////////////

bool TsMessageHandle::IsThisMsg(string type)
{
	if("RequestResult" == type && "TransResult" == m_type)
		return true;
		
	if(type == m_type)
		return true;
	else
		return false;
}

void TsMessageHandle::BuildAndSendResultResponse( Json::Value & value,TsSession * tsSession)
{
    Json::Value result_rsp;

    result_rsp["type"] = "ResultResponse";

    if(!value["id"].isNull()&&value["id"].isString())
    {
        result_rsp["id"] = value["id"].asString();
    }

    if(!value["servertype"].isNull()&&value["servertype"].isString())
    {
        result_rsp["servertype"] = value["servertype"].asString();
    }

    string result_rsp_str = result_rsp.toStyledString() +'\0';
    
    tsSession->SendMessage(result_rsp_str);

    return;
    
}


//////////////////////////////// TsRegistMessageHandle /////////////////////////////

void TsRegistMessageHandle::SpacificHandle(Json::Value& value,TsSession* tsSession)
{
	if(IsThisMsg(value["type"].asString())){
		if(m_register != NULL){
			m_register->OnRegister(value, tsSession);
		}
	} else {
		TsMessageHandle::SpacificHandle(value,tsSession);
	}
}


//////////////////////////////// TsResultMessageHandle /////////////////////////////////
void TsResultMessageHandle::SpacificHandle(Json::Value& value,TsSession* tsSession)
{
	if(IsThisMsg(value["type"].asString())){
		if((!value["servertype"].isNull()) &&  (value["servertype"].isString())){
			BuildAndSendResultResponse(value,tsSession);
			m_taskResult->SpacificTaskResult(value,tsSession);
		} else {

		}
	} else {
		TsMessageHandle::SpacificHandle(value,tsSession);
	}
}

////////////////////////////// TsUpdateMessageHandle //////////////////////////////////
void TsUpdateMessageHandle::SpacificHandle(Json::Value& value,TsSession* tsSession)
{
	if(IsThisMsg(value["type"].asString())){
		if(m_updataInfo!= NULL){
			m_updataInfo->OnUpdateInfo(value, tsSession);
		}
	} else {
		TsMessageHandle::SpacificHandle(value,tsSession);
	}
}

////////////////////////////////  TsMessageAnalyzer  ///////////////////////////////
//class TsRegister;
TsRegister testITR;

TsMessageAnalyzer::TsMessageAnalyzer()
{

	m_messageHandle = &TsRegistMessageHandle::GetInstance(&testITR);

	TsResultMessageHandle& resultHandle = TsResultMessageHandle::GetInstance();
	TsUpdateMessageHandle& updateHandle = TsUpdateMessageHandle::GetInstance();

	m_messageHandle->add(&resultHandle);
	m_messageHandle->add(&updateHandle);
	
}

TsMessageAnalyzer::~TsMessageAnalyzer()
{

}

TsMessageAnalyzer& TsMessageAnalyzer::GetInstance()
{
	static  TsMessageAnalyzer instance_;
	return instance_; 
}

void  TsMessageAnalyzer::OnNewFrame(TsSession* tsServerSion,const string frame)
{
	Json::Value  value;  
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();

	if(tsServerSion == NULL){

		return;
	}

	std::auto_ptr<Json::Reader> pJsonParser(new Json::Reader(Json::Features::strictMode())); 

//    cout<<"#######result msg: "<<frame<<endl;
	dsLog.Log(true,LOGGER_INFO,"\n ############################################################\n MSG From TS :\n %s\n\n",frame.c_str());
	if(pJsonParser->parse(frame,value)){
		if((!value["type"].isNull()) &&  (value["type"].isString())){
			m_messageHandle->SpacificHandle( value, tsServerSion);
		} else {
			
		}
	}
}

////////////////////////////////  AppMessageAnalyzer  //////////////////////////////

AppMessageAnalyzer::AppMessageAnalyzer()
{
	m_addTask = &AddDocTask::GetInstance();
	AddVideoTask &addVideoTask = AddVideoTask::GetInstance();
	AddDpptTask &addDpptTask = AddDpptTask::GetInstance();
	AddNoUseTsTask &addNoUseTsTask = AddNoUseTsTask::GetInstance();
	

	m_addTask->add(&addVideoTask);
	m_addTask->add(&addDpptTask);
	m_addTask->add(&addNoUseTsTask);
	
}

AppMessageAnalyzer::~AppMessageAnalyzer()
{

}

AppMessageAnalyzer& AppMessageAnalyzer::GetInstance()
{
	static  AppMessageAnalyzer instance_;
	return instance_; 
}

void AppMessageAnalyzer::OnNewFrame(AppServerSession *appServerSion,string frame)
{
	Json::Value  value;  
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();

	if(appServerSion == NULL){
		dsLog.Log(true,LOGGER_ERROR,"[%s][%s][%d] appServerSion null!  \n",__FILE__,__PRETTY_FUNCTION__,__LINE__);		
		return;
	}

	std::auto_ptr<Json::Reader> pJsonParser(new Json::Reader(Json::Features::strictMode())); 

	if(pJsonParser->parse(frame,value)){
		if((!value["command"].isNull()) &&  (value["command"].isString())){
			appmsgtotaltimes +=1;
			
			m_addTask->AddSpacificTask(value,appServerSion);
		}
	}
	
}



