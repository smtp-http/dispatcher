#include <iostream>
#include <memory>
#include "InterfaceApiImp.h"
#include "Error.h"

#include "json/json.h"

#include <log4cxx/logger.h>
#include <log4cxx/logstring.h>
#include <log4cxx/propertyconfigurator.h>

#include "Log4cxxWrapper.h"



using namespace log4cxx;
extern LoggerPtr g_logger;

using namespace std;


void SessionImp::ProcessFrame(const char *buf, int length)
{
	int i,len = length,count = 0;
	string newFrame;
	int pos=0;
	len =0;

	LoggerWrapper dsLog= LoggerWrapper::GetInstance();
	
	
	if(buf == NULL){
		dsLog.Log(true,LOGGER_ERROR,"buf == NULL , [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
	}

    
	for(i=0;i<length;i++){
		if(buf[i] == '\0'){
			len = i-pos;
			pos = i;
			newFrame.clear();
			newFrame.assign(m_rdBuffer.begin(),m_rdBuffer.end());
			len += m_rdBuffer.size();

			if(len>1){
				OnFrame(newFrame);
				m_rdBuffer.clear();
			}
		} else {
			m_rdBuffer.push_back(buf[i]);
		}        
	}

	return;
}

void SessionImp::SetConnection(Connection * conn)
{
	LoggerWrapper& dsLog = LoggerWrapper::GetInstance();
	
	
	if(conn == NULL){
		dsLog.Log(true,LOGGER_ERROR,"%s:SessionImp::%s:%d  error param",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return;
	}
	
	m_connection = conn;
}



int SessionImp::SendMessage(string &str)
{
	int ret;
	LoggerWrapper& dsLog = LoggerWrapper::GetInstance();
	
	str =str+'\0';
	if(m_connection != NULL){cout << "==================== ts cmd: " << str << endl;
		ret = m_connection->Send(str.data(),str.length());

		if(ret < 0){
			dsLog.Log(true,LOGGER_ERROR,"%s:SessionImp::%s:%d",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		}
		
		return ret;
	}

	return -1;
}


void SessionImp::ShowReadBuf()
{
	vector<char>::iterator iter;
	for (iter = m_rdBuffer.begin(); iter != m_rdBuffer.end(); iter++){
		cout << (*iter);
	}

	cout << endl;
}





TsSessionImp::TsSessionImp(/*ITsSessionUser *sessionUser,*/TsSession *tssession)
{
	//m_sessionUser = sessionUser;
	m_tssession = tssession;
	m_sessionType = SESSION_TS;
}

//void TsSessionImp::SetSessionUser(ITsSessionUser *sessionUser)
//{
	//m_sessionUser = sessionUser;
//}



void TsSessionImp::Close()
{
	// close connect
	struct TsSessionInfo *tsSessionInfo;
	list<struct TsSessionInfo*>::iterator tlit;

	SessionManagerImp& smi =  SessionManagerImp::GetInstance();
	
	m_connection->Close();

	if (this->m_connection){
		delete this->m_connection;
		this->m_connection = NULL;
	}

	for (tlit = smi.m_tsSessionList.begin();tlit != smi.m_tsSessionList.end();tlit ++) { 
		tsSessionInfo = *tlit;
		if(tsSessionInfo != NULL){
			if(tsSessionInfo->session == this->m_tssession){
				
				smi.m_tsSessionList.erase(tlit);   

				delete tsSessionInfo;
				
				break;
			}
		}
	}

	delete this;
}


void TsSessionImp::OnData(const char *buf, int length, Connection *conn)
{
	ProcessFrame(buf,length);
}

	// called when user can write data to network.
void TsSessionImp::OnWrite(Connection *conn)
{

}
	// called when the connection is broken, if user call Connection::Close(), //&&&
	// will not get this callback.
void TsSessionImp::OnDisconnect(int reason, Connection *conn)
{
	SessionManagerImp& smi =  SessionManagerImp::GetInstance();

	if(this->m_tssession != NULL)
		smi.OnSessionClose(*this->m_tssession);
	
	Close();
}

bool TsSessionImp::OnFrame(string &frame)
{
	struct TsSessionInfo *tsSessionInfo;
	list<struct TsSessionInfo*>::iterator it;
	//ITsSessionUser* tsSessionUser;
	Json::Value  value;  
	LoggerWrapper& dsLog = LoggerWrapper::GetInstance();
	SessionManagerImp& smi =  SessionManagerImp::GetInstance();

	std::auto_ptr<Json::Reader> pJsonParser(new Json::Reader(Json::Features::strictMode())); 

	ITsNewFrame* itsu;
	

	if(!pJsonParser->parse(frame,value)){
		dsLog.Log(true,LOGGER_ERROR,"message Decode error! %s : %s : %d",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	if(value["type"].isNull() ||  !value["type"].isString()){
		dsLog.Log(true,LOGGER_ERROR,"message Decode error! %s : %s : %d",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}


	if(value["type"] == "LinkResponse"){
		for(it = smi.m_tsSessionList.begin();it != smi.m_tsSessionList.end();it ++) {//printf("@@@@@@@@@@@ leng:%d     0x%x     (%d)\n",smi.m_tsSessionList.size(),it,__LINE__);
			tsSessionInfo = *it;
			if(tsSessionInfo != NULL){
				if(tsSessionInfo->session == this->m_tssession){
					tsSessionInfo->timeOut = 0;
					break;
				}
			}
		}
	} else {
		itsu= smi.GetTsNewFrame();
		if(itsu == NULL){
			dsLog.Log(true,LOGGER_ERROR,"%s:%s:%d   smi.GetSessionUser() == NULL\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
			return false;
		}
		itsu->OnNewFrame(this->m_tssession,frame);
	}

	return true;
}

/////////////////////// ts client session ///////////////////////

void TsClientSessionImp::Close()
{

}

void TsClientSessionImp::OnData(const char *buf, int length, Connection *conn)
{

}

	// called when user can write data to network.
void TsClientSessionImp::OnWrite(Connection *conn)
{

}
	// called when the connection is broken, if user call Connection::Close(), //&&&
	// will not get this callback.
void TsClientSessionImp::OnDisconnect(int reason, Connection *conn)
{

}

bool TsClientSessionImp::OnFrame(string &frame)
{
}
/////////////////// /// app server session /////////////////////




void AppServerSessionImp::Close()
{
	m_connection->Close();

	if (this->m_connection){
		delete this->m_connection;
		this->m_connection = NULL;
	}

	delete this;
}

void AppServerSessionImp::OnData(const char *buf, int length, Connection *conn)
{
	//printf("++++++++++++++++++++++++++++++++++: \n%s\n",buf);
    LoggerWrapper dsLog= LoggerWrapper::GetInstance();
	
	dsLog.Log(true,LOGGER_DEBUG,"\n\n ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n MSG From App:\n%s\n",buf);
	
	ProcessFrame(buf,length);
}

void AppServerSessionImp::OnWrite(Connection *conn)
{

}

void AppServerSessionImp::OnDisconnect(int reason, Connection *conn)
{
	struct TsSessionInfo *tsSessionInfo;
	list<struct TsSessionInfo*>::iterator tlit;
	
	SessionManagerImp& smi =  SessionManagerImp::GetInstance();
	smi.OnSessionClose(*this);

	Close();
}

bool AppServerSessionImp::OnFrame(string &frame)
{

	LoggerWrapper& dsLog = LoggerWrapper::GetInstance();

	SessionManagerImp& smi =  SessionManagerImp::GetInstance();

	IAppNewFrame* iael = smi.GetAppNewFrame();
	if(iael == NULL){
		dsLog.Log(true,LOGGER_ERROR,"GetAppEventListener error! %s : %s : %d",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	iael->OnNewFrame(this->m_session,frame);

	return true;
}


//////////////////////// app client session ///////////////////


void AppClientSessionImp::Close()
{
	if(m_connection != NULL)
		m_connection->Close();

	if (this->m_connection){
		delete this->m_connection;
		this->m_connection = NULL;
	}

	delete this;
}

void AppClientSessionImp::OnData(const char *buf, int length, Connection *conn)
{
	
	ProcessFrame(buf,length);
	
}

void AppClientSessionImp::OnWrite(Connection *conn)
{

}

void AppClientSessionImp::OnDisconnect(int reason, Connection *conn)
{

	SessionManagerImp& smi =  SessionManagerImp::GetInstance();
	smi.OnSessionClose(*this);

	Close();
}

bool AppClientSessionImp::OnFrame(string &frame)
{
	Json::Value  value;  
	LoggerWrapper& dsLog = LoggerWrapper::GetInstance();
	SessionManagerImp& smi =  SessionManagerImp::GetInstance();

	std::auto_ptr<Json::Reader> pJsonParser(new Json::Reader(Json::Features::strictMode())); 
	

	if(!pJsonParser->parse(frame,value)){
		dsLog.Log(true,LOGGER_ERROR,"message Decode error! %s : %s : %d",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	if(value["command"].isNull() ||  !value["command"].isString()){
		dsLog.Log(true,LOGGER_ERROR,"message Decode error! %s : %s : %d",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}


	if(value["command"] == "response"){
		cout << "app client OnDisconnect from(OnFrame) "  << endl;
		smi.OnSessionClose(*this);

		Close();
	} else {
		dsLog.Log(true,LOGGER_ERROR,"this message may be not a AppClientSession message! %s : %s : %d",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}


	return true;
}




