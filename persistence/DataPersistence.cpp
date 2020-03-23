#include "DataPersistence.h"
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <string>
#include <locale.h> 
#include <stdlib.h>
#include <sstream>
#include <stdarg.h> 
#include <errno.h>
#include <fstream>
#include <vector>
#include <iostream>
#include <fcntl.h> 
#include <utility>
#include "json/json.h"
#include "Log4cxxWrapper.h"
#include "Log4cxxWrapper.h"
#include "InfoComponent.h"

using namespace std;


#define set_task_dp_manager(taskType)  \
	DocDpManager DocDp(m_queueUser);  \
	DpptDpManager DpptDp(m_queueUser);  \
	VideoDpManager VideoDp(m_queueUser);  \
	switch(taskType){  \
		case DocTask: \
			m_taskDpManager = &DocDp; \
			break; \
		case DpptTask: \
			m_taskDpManager = &DpptDp; \
			break; \
		case VideoTask: \
			m_taskDpManager = &VideoDp; \
			break; \
		default: \
			m_taskDpManager = NULL; \
			return false; \
	}
	
string systemcmd(const string &cmd)
{
	char   buf[1024*10]; 
	string cmdrlt;
	cmdrlt = cmd + " 2>&1| tee /var/logfile";
	
	memset( buf, '\0', sizeof(buf) );

	system(cmdrlt.c_str());

	ifstream fin("/var/logfile"); 
	string rlt;
	while(fin.getline(buf,1024*10))
	{    
		rlt += buf;
		rlt += "\n";
	}
	
	return rlt;
}

string command(const string &cmd)
{
	FILE   *stream;  
	char   buf[1024*100]; 
	string cmdrlt;

	
	memset( buf, '\0', sizeof(buf) );
	
	stream = popen(cmd.c_str(), "r" ); 
	string rlt;
	
	int cnt = fread( buf, sizeof(char), sizeof(buf)-1,  stream); 
	buf[cnt]= '\0';
	
	pclose( stream );
	return buf;
}



//×Ö·û´®·Ö¸îº¯Êý
vector<string> split(string cmdRet,string pattern)
{
	string::size_type pos;
	vector<string> result;
	cmdRet+=pattern;//À©Õ¹×Ö·û´®ÒÔ·½±ã²Ù×÷
	int size=cmdRet.size();

	for(int i = 0; i < size; i ++){
		pos=cmdRet.find(pattern,i);
		if(pos<size){
			string s=cmdRet.substr(i,pos-i);
			result.push_back(s);
			i = pos+pattern.size()-1;
		}
	}
	
	return result;
}

bool MakeDir(string &vle) 
{
	string cmd = "mkdir -p -m 777 " + vle;
	string rlt = command(cmd);
	return true;
}

string GetDir(string &vle)
{
	size_t pos = vle.rfind("/");
	if(pos == string::npos) {
		return "";
	}

	string rltpath = vle.substr(0, pos);
	return rltpath;
}

bool IsExistedDir(const string &vle)
{
	bool rlt = true;
	DIR *dir=NULL;
	dir = opendir(vle.c_str());
	
	if(NULL == dir){
		rlt = false;
	} else {
		closedir(dir);
	}
	return rlt;
}

string IntToStr(int &vle)
{
	stringstream ss;
	string rlt;
	ss << vle;
	ss >> rlt;
	return rlt;
}

int StrToInt(string &vle)
{
	stringstream ss;
	int rlt = 0;
	ss << vle;
	ss >> rlt;
	return rlt;
}

bool IsDigitString(string &vle)
{
	bool rlt = true;
	int icount = 0;
	icount = vle.length();
	if(icount == 0)
		return false;
	for(int i = 0; i < icount; i++) {
		if(!isdigit(vle[i])){
			rlt = false;
			break;
		}
	}
	return rlt;
}



string FormatString(const char *format, ...)
{
	char szBuffer[4096];

	memset(szBuffer, '\0', sizeof(szBuffer));
	va_list args;
	va_start(args, format);
	
	vsprintf(szBuffer, format, args);
	va_end(args);
	string rlt = szBuffer;
	return rlt;
}

string GetFilename(string &vle)
{
	size_t pos = vle.rfind("/");
	if(pos == string::npos) {
		return vle;
	} else {
		return vle.substr(pos+1, vle.length()-pos-1);
	}
}

int GetFileName(string &fileName,TaskType type)
{
	
	string ret = systemcmd(GET_ALL_DOC_CMD);
	vector<string> files = split(ret,"\n");
	
	return 0;
}

string GetFilenamePrefix(string &vle)
{
	string filename = GetFilename(vle);
	size_t pos = filename.rfind(".");
	if(pos == string::npos) {
		return filename;
	} else {
		return filename.substr(0, pos);
	}
}

string BuildJsonStr(DpMessageType &msg)
{
	return msg.toStyledString();
}


bool GetJsonMap(string &strRead,DpMessageType &msg)
{
	Json::Reader parser(Json::Features::strictMode());
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();

	if(parser.parse(strRead,msg)){
		if(msg.type()==Json::nullValue){
			dsLog.Log(true,LOGGER_ERROR,"Json::nullValue!!! [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
			return false;
		}
	} else {
		dsLog.Log(true,LOGGER_ERROR,"json string parse fail !!! [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	return true;
}

DataPresistence& DataPresistence::GetInstance(IQueueUser *queueUser)
{
	static  DataPresistence instance_(queueUser);
	return instance_; 

}

DataPresistence::DataPresistence(IQueueUser *queueUser)
	:m_queueUser(queueUser)
{
	
}



bool DataPresistence::DpGetMessage(DpMessageArryType &dpMsg,TaskType taskType)
{
	set_task_dp_manager(taskType);
	m_taskDpManager->GetInfoFromDpFile(dpMsg);
	
	return true;
}

bool DataPresistence::DpGetWaitMessage(DpMessageArryType &dpMsg,TaskType taskType)
{
	bool ret;
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();
	
	set_task_dp_manager(taskType);

	ret = m_taskDpManager->GetTaskinfoFromDpFile(dpMsg,QueueWait);
	if(!ret){
		dsLog.Log(true,LOGGER_ERROR,"%s:%s:%d   GetTaskinfoFromDpFile error.\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}
	
	return true;
}


bool DataPresistence::DpGetRunMessage(DpMessageArryType &dpMsg,TaskType taskType)
{
	bool ret;
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();
	
	set_task_dp_manager(taskType);

	ret = m_taskDpManager->GetTaskinfoFromDpFile(dpMsg,QueueRuning);
	if(!ret){
		dsLog.Log(true,LOGGER_ERROR,"%s:%s:%d   GetTaskinfoFromDpFile error.\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	return true;
}


bool DataPresistence::CreateDpMsgFile(TaskType taskType)
{
	set_task_dp_manager(taskType);
	m_taskDpManager->CreateDpFile();
	return true;
}

bool DataPresistence::CreateDpMsgFile(const DpMessageType &dpMsg,TaskType taskType)
{
	set_task_dp_manager(taskType);
	m_taskDpManager->CreateDpFile(dpMsg);
	
	return true;
}

bool DataPresistence::CreateWaitMsgFile(TaskType taskType)
{
	set_task_dp_manager(taskType);
	m_taskDpManager->CreateDpFile(QueueWait);

	return true;
}

bool DataPresistence::CreateRunMsgFile(TaskType taskType)
{
	set_task_dp_manager(taskType);
	m_taskDpManager->CreateDpFile(QueueRuning);

	return true;
}



///////////////////////////////////// DpManager ///////////////////////////////////////

bool DpManager::GetInfoFromDpFile(DpMessageArryType &dpMsgArray)
{
	int i,j;
	char buf[2096];
	memset(buf,0,sizeof(buf));
	unsigned short msgNum,readLen;
	size_t ret;
	FILE*fp=NULL;
	string folder = "dpfile";
	string filename = "dpfile/" + m_filePrefix + m_dataType + ".data";
	string provFileName = "dpfile/" + m_filePrefix + m_dataType + ".prov";
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();
	
	if(!IsExistedDir(folder)){
		MakeDir(folder);
		return false;
	}

	fp=fopen(filename.c_str(),"r");
	if(NULL==fp){
		dsLog.Log(true,LOGGER_INFO,"%s:%s:%d   \n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		fp = fopen(provFileName.c_str(),"r");
		if(NULL==fp){
			dsLog.Log(true,LOGGER_INFO,"%s:%s:%d   \n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
			return false;
		}
		string cmd = "mv " + provFileName + "  " + filename;
			
		systemcmd(cmd);
	} else {
		string cmd = "rm -rf " + provFileName;
		systemcmd(cmd);
	}


	
	ret = fread(&msgNum,sizeof(unsigned short),1,fp);


	
	msgNum = (msgNum%256)*256 + msgNum/256;
	dsLog.Log(true,LOGGER_INFO,"%s:%s:%d	msg num: %d\n",__FILE__,__PRETTY_FUNCTION__,__LINE__,msgNum);

	for(i = 0;i < msgNum;i ++){
		ret = fread(&readLen,sizeof(unsigned short),1,fp);
		readLen = (readLen%256)*256 + readLen/256;
		ret = fread(buf,1,readLen,fp);


		string msg = buf;
		DpMessageType dpMsg;// = new DpMessageType;	

		if(GetJsonMap(msg,dpMsg)){
			dpMsgArray.push_back(dpMsg);
		} else {
			fclose(fp);
			return false;
		}

		memset(buf,0,sizeof(buf));
	}

	

	fclose(fp);
	return true;
	
}



bool DpManager::WriteInfoToFile(DpMessageArryType &dpMsg)
{
	unsigned short size;
	char buf[1024];
	int i,ret;
	FILE*fp=NULL;
	string folder = "dpfile";
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();
	memset(buf,0,sizeof(buf));
	string provFileName = "dpfile/" + m_filePrefix + m_dataType + ".prov";
	string filename = "dpfile/" + m_filePrefix + m_dataType + ".data";

	if(!IsExistedDir(folder)){
		MakeDir(folder);
		return false;
	}
	
	fp=fopen(provFileName.c_str(),"ab+");
	if(NULL==fp){
		dsLog.Log(true,LOGGER_INFO,"%s:%s:%d   \n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	size = dpMsg.size();

	dsLog.Log(true,LOGGER_INFO,"size:%d, %s:%s:%d   \n",size,__FILE__,__PRETTY_FUNCTION__,__LINE__);

	buf[0] = size/256;
	buf[1] = size%256;

	fwrite(buf, 1, 2, fp);

	string strMsg;
	for(i = 0;i < size;i ++){
		//msg = dpMsg[i];
		strMsg = BuildJsonStr(dpMsg[i]);

		//delete(msg);
		dpMsg[i].clear();

//		dsLog.Log(true,LOGGER_INFO,"strMsg:%s, %s:%s:%d   \n",strMsg.c_str(),__FILE__,__PRETTY_FUNCTION__,__LINE__);
		
		buf[0] = strMsg.length()/256;
		buf[1] = strMsg.length()%256;
		fwrite(buf, 1, 2, fp);

		fwrite(strMsg.c_str(),1,strMsg.length(),fp);
	}
	
	fclose(fp);

	string cmd0 = "rm -rf " + filename;
	systemcmd(cmd0);
	string cmd = "mv " + provFileName + " " + filename;
	systemcmd(cmd);

	return true;
}


///////////////////////////////////// ResultReTransDpManager ////////////////////////////
bool ResultReTransDpManager::CreateDpFile()
{

}

///////////////////////////////////// TransTaskDpManager ///////////////////////////////



bool TransTaskDpManager::GetTaskinfoFromDpFile(DpMessageArryType &dpMsgArray,QueueType qt)
{
	switch(qt){
		case QueueWait:
			m_dataType = "_WAIT";
			break;
		case QueueRuning:
			m_dataType = "_RUN";
			break;
		default:
			return false;
	}
	return GetInfoFromDpFile(dpMsgArray);
}



bool TransTaskDpManager::CreateDpFile()
{
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();
	DpMessageArryType  dpMsgArry;
	
	return GetInfoAndWriteToFile(dpMsgArry);
}


bool TransTaskDpManager::CreateDpFile(QueueType qt)
{
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();
	DpMessageArryType  dpMsgArry;
	
	return GetInfoAndWriteToFile(dpMsgArry,qt);
}


bool TransTaskDpManager::CreateDpFile(const DpMessageType &dpMsg)
{
	DpMessageArryType  dpMsgArry;

	dpMsgArry.push_back(dpMsg);

	return GetInfoAndWriteToFile(dpMsgArry);
}

bool TransTaskDpManager::GetInfoAndWriteToFile(DpMessageArryType &dpMsgArry)
{
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();

	//if(!GetInfoFromSystem(dpMsgArry)){
	//	dsLog.Log(true,LOGGER_ERROR,"%s:%s:%d   GetInfoFromSystem error.\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
	//	return false;
	//}

	bool ret = false;
	QueueInfo *qi = new QueueInfoAdapter(*m_queueUser);

	InfoComponent* pCom = new ConcreteInfoComponent(dpMsgArry);   
	
	InfoAdder* pDec = NULL;
	pDec = new RunQueueInfoAdder(pCom,*qi,m_taskType);          
	pDec = new WaitQueueInfoAdder(pDec,*qi,m_taskType);         
	ret = pDec->Operation();

	delete qi;
	delete pDec;

	if(ret == false){
		dsLog.Log(true,LOGGER_ERROR,"%s:%s:%d   GetQueueInfo error.\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}
	
	if(!WriteInfoToFile(dpMsgArry)){
		dsLog.Log(true,LOGGER_ERROR,"%s:%s:%d   WriteInfoToFile error.\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}


	return true;
}


bool TransTaskDpManager::GetInfoAndWriteToFile(DpMessageArryType &dpMsgArry,QueueType qt)
{
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();


	QueueInfo *qi = new QueueInfoAdapter(*m_queueUser);
	bool ret = false;
	InfoComponent* pCom = new ConcreteInfoComponent(dpMsgArry);   
	//ConcreteInfoComponent com(dpMsgArry);
	InfoAdder* pDec = NULL;
	switch(qt){
		case QueueWait:
			m_dataType = "_WAIT";
			pDec = new WaitQueueInfoAdder(pCom,*qi,m_taskType);   
			break;
		case QueueRuning:
			m_dataType = "_RUN";
			pDec = new RunQueueInfoAdder(pCom,*qi,m_taskType);
			break;
		default:
			ret = false;
	}
	
	ret = pDec->Operation();

	delete qi;
	delete pDec;
	if(ret == false){
		dsLog.Log(true,LOGGER_ERROR,"%s:%s:%d   GetQueueInfo error.\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}
	
	if(!WriteInfoToFile(dpMsgArry)){
		dsLog.Log(true,LOGGER_ERROR,"%s:%s:%d   WriteInfoToFile error.\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}


	return true;
}




