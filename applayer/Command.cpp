#include "Command.h"
#include <stdlib.h>

//#include "TsManager.h"
#include "Log4cxxWrapper.h"
#include <iostream>
#include <strstream>
#include "TaskResult.h"

using namespace std;
extern TsQueue GVideoTsQueue;
extern TsQueue GDocOpenofficeTsQueue;
extern TsQueue GDocWpsTsQueue;
extern TsQueue GDocOfficeTsQueue;
extern TsQueue GDpptTsQueue;

extern TaskWaitQueue GVideoTaskWaitQueue;
extern TaskRunQueue GVideoTaskRunQueue;

extern int fail_times;
extern int appmsgtotaltimes;
extern int succ_times;
extern int fail_times;
extern int tsLeave_times;
extern int timeoutforRunningTask;
extern int timeoutforWaitTask;
extern int taskNull_times;



AppCmdServerStatus GAppCmdServerStatus;
AppCmdVideoMessage GAppCmdVideoMessage;
AppCmdVideoTrans GAppCmdVideoTrans;
AppCmdPrintScreen GAppCmdPrintScreen;
AppCmdVideoCut GAppCmdVideoCut;
AppCmdVideoMerge GAppCmdVideoMerge;
AppCmdDocTrans GAppCmdDocTrans;
AppCmdDynamicPptTrans GAppCmdDynamicPptTrans;
AppCmdAudioTrans GAppCmdAudioTrans;
AppCmdTaskProgress GAppCmdTaskProgress;
AppCmdAudioMessage GAppCmdAudioMessage;


TsCmdVideoMessage GTsCmdVideoMessage;
TsCmdVideoTrans GTsCmdVideoTrans;
TsCmdVideoRequest GTsCmdVideoRequest;
TsCmdPrintScreen GTsCmdPrintScreen;
TsCmdVideoCut GTsCmdVideoCut;
TsCmdVideoMerge GTsCmdVideoMerge;
TsCmdDocTrans GTsCmdDocTrans;
TsCmdDynamicPptTrans GTsCmdDynamicPptTrans;
TsCmdAudioTrans GTsCmdAudioTrans;

TsCmdAudioMessage GTsCmdAudioMessage;
//////////////////// TaskState ////////////////////////
bool TaskState::Handle(Json::Value& value,TaskResult* taskResult)
{	
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();
	string errorcode;
	
	Task* task = this->GetTask();
	if(!task){
		dsLog.Log(true,LOGGER_ERROR,"%s:%s:%d  get task error!\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	// errorcode handling
	
	errorcode = value["errorcode"].asString();
	if(atoi(errorcode.c_str()) != 0){
		taskResult->PunishTsPriority(task);
		if(task->IsExceedMaxTryTimes()){
			// TODO: log
			fail_times +=1;
            /*
            cout<<"++++++++++++++++++++++++++++++++total times: "<<appmsgtotaltimes<<endl;
            cout<<"++++++++++++++++++++++++++++++++succ_times times: "<<succ_times<<endl;
            cout<<"++++++++++++++++++++++++++++++++fail times: "<<fail_times<<endl;
            cout<<"++++++++++++++++++++++++++++++++timeoutforRunningTask times:"<<timeoutforRunningTask<<endl;
            cout<<"++++++++++++++++++++++++++++++++timeoutforWaitTask times:"<<timeoutforWaitTask<<endl;
            */
			dsLog.Log(true,LOGGER_DEBUG,"\n\n ++++++++++++++++++++++++++++++++total times:  %d\n ++++++++++++++++++++++++++++++++succ_times times:  %d\n ++++++++++++++++++++++++++++++++fail times:  %d\n ++++++++++++++++++++++++++++++++tsLeave_times:  %d\n ++++++++++++++++++++++++++++++++timeoutforRunningTask times:  %d\n ++++++++++++++++++++++++++++++++timeoutforWaitTask times:  %d\n",
			appmsgtotaltimes,succ_times,fail_times,tsLeave_times,timeoutforRunningTask,timeoutforWaitTask);
			task->ResultReport(value);
			taskResult->m_taskRunQueue->DelTask(task->GetTaskId());
			taskResult->m_taskDb->GiveBackTask(task);

			//task = taskResult->m_taskWaitQueue->PopTask();
			task = taskResult->PopTask();
			if(task){
				taskResult->m_disptacher->Submit(task);
			}
		} else {
			task ->SetPriority(255);
			taskResult->m_disptacher->Submit(task);
		}

		return true;
	}
	
	succ_times +=1;
	dsLog.Log(true,LOGGER_DEBUG,"\n\n ++++++++++++++++++++++++++++++++total times:  %d\n ++++++++++++++++++++++++++++++++succ_times times:  %d\n ++++++++++++++++++++++++++++++++fail times:  %d\n ++++++++++++++++++++++++++++++++tsLeave_times:  %d\n ++++++++++++++++++++++++++++++++timeoutforRunningTask times:  %d\n ++++++++++++++++++++++++++++++++timeoutforWaitTask times:  %d\n",
	appmsgtotaltimes,succ_times,fail_times,tsLeave_times,timeoutforRunningTask,timeoutforWaitTask);
    
    cout<<"++++++++++++++++++++++++++++++++total times: "<<appmsgtotaltimes<<endl;
    cout<<"++++++++++++++++++++++++++++++++succ_times times: "<<succ_times<<endl;
    cout<<"++++++++++++++++++++++++++++++++fail times: "<<fail_times<<endl;
    cout<<"++++++++++++++++++++++++++++++++taskNull_times: "<<taskNull_times<<endl;
    cout<<"++++++++++++++++++++++++++++++++tsLeave_times: "<<tsLeave_times<<endl;
    cout<<"++++++++++++++++++++++++++++++++timeoutforRunningTask times:"<<timeoutforRunningTask<<endl;
    cout<<"++++++++++++++++++++++++++++++++timeoutforWaitTask times:"<<timeoutforWaitTask<<endl;
    
	// task result
	task->ResultReport(value);
	taskResult->m_taskRunQueue->DelTask(task->GetTaskId());
	taskResult->m_taskDb->GiveBackTask(task);


	// Task completed, redistributed
	task = taskResult->m_taskWaitQueue->PopTask();
	if(!task){
		dsLog.Log(true,LOGGER_INFO,"%s:%s:%d task wait queue is empty\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return true;
	}
	cout<<"ts free select task from waitqueue task id:"<<task->GetTaskId();
	taskResult->m_disptacher->Submit(task);
	
	return true;

}




////////////////// TaskVideoTransCheck ///////////////
TaskVideoTransCheck::TaskVideoTransCheck()
{
	m_tsCmd = &GTsCmdVideoRequest;
}


bool TaskVideoTransCheck::Handle( Json::Value &value,TaskResult* taskResult)
{	
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();

	
	Task* task = this->GetTask();
	if(!task){
		dsLog.Log(true,LOGGER_ERROR,"%s:%s:%d  get task error!\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	Json::Value appCmdValue = task->GetCmdValue();
	/*{"type":"TransResult","result":"Success/Fail","servertype":"Transfer","id":"FFFFFFFFFFFFFF", "file":"filename","targetfolder":"\\10.1.200.101\somepath\somefile.trans.mp4","errorcode":"4004";"errordetail":"not found"}
	*/

	// errorcode handling
	Json::Value videoTransValue;
	
	if(value["result"].isNull() || !value["result"].isString()){
		dsLog.Log(true,LOGGER_ERROR,"%s:%s:%d \"result\" err!\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}
	
	if("false" == value["result"].asString()){
		cout << "Video does not need to be converted ....." << endl;
		videoTransValue["type"] = "TransResult";
		videoTransValue["result"] = "Success";
		videoTransValue["servertype"] = "Transfer";
		if(!value["id"].isNull() && value["id"].isString())
			videoTransValue["id"] = value["id"].asString();
		else {
			dsLog.Log(true,LOGGER_ERROR,"%s:%s:%d \"id\" err!\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
			return false;
		}
		
		if(!appCmdValue["file"].isNull() && appCmdValue["file"].isString())
			videoTransValue["file"] = appCmdValue["file"].asString();
		else {
			dsLog.Log(true,LOGGER_ERROR,"%s:%s:%d \"file\" err!\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
			return false;
		}
		cout << " ====== app cmd: " << appCmdValue.toStyledString() << endl;
		if(!appCmdValue["targetFile"].isNull() && appCmdValue["targetFile"].isString())
			videoTransValue["targetFile"] = appCmdValue["targetFile"].asString();
		else {
			dsLog.Log(true,LOGGER_ERROR,"%s:%s:%d \"targetFile\" err!\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
			return false;
		}

		videoTransValue["errorcode"] = "0";
		task->ResultReport(videoTransValue);

		taskResult->m_taskRunQueue->DelTask(task->GetTaskId());
		taskResult->m_taskDb->GiveBackTask(task);

		//task = taskResult->m_taskWaitQueue->PopTask();
		task = taskResult->PopTask();
		if(task){
			taskResult->m_disptacher->Submit(task);
		}
	} else if("true" == value["result"].asString()){
		//task->GetTaskState()->ChangeState(task,state);
		TaskState* nextTaskState = new TaskVideoTrans();
		task->ChangeState(nextTaskState);
		taskResult->m_disptacher->Submit(task);
		cout << "video trans ......" << endl;
	} else {
		dsLog.Log(true,LOGGER_ERROR,"%s:%s:%d VideoTransRequest result err!\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	return true;
}


//////////////////// TaskVideoTrans //////////////////
TaskVideoTrans::TaskVideoTrans()
{
	m_tsCmd = &GTsCmdVideoTrans;
}


///////////////////// command ///////////////////////
command::command()
{

}

command::~command()
{

}

///////////////////// AppCommand ////////////////////
AppCommand::AppCommand()
{
	m_isProvidedResult = true;
}

AppCommand::~AppCommand()
{

}

TaskState* AppCommand::CreatTaskState() 
{
	return new TaskState(m_defaultTsCmd);
}

bool  AppCommand::BuildAppResponse(Json::Value& appResponse,unsigned int errorCode,string ErrorDetail)
{
	stringstream ss;
	ss<<errorCode; 
	string strErrCode = ss.str();
	appResponse["errorcode"] = strErrCode;
	appResponse["errordetail"] = ErrorDetail;
	appResponse["command"] = "response";
	return true;
}

////////////////////////// specific app command /////////////////////////////
////////////////////////// AppCmdServerStatus //////////////////////////////
AppCmdServerStatus::AppCmdServerStatus()
{
	m_commandName = "server_status_task";
	m_taskType = NoTsTask;
	m_isProvidedResult = true;
	m_defaultTsCmd = NULL;
}

bool AppCmdServerStatus::BuildAppResponse(const  Json::Value & appCmd, string & appResultStr)
{
	Json::Value DocServerNum;
	Json::Value DocOfficeServerNum;
	Json::Value PptServerNum;
	Json::Value VideoServerNum;
	Json::Value DocWpsServerNum;

	unsigned int ret;

	Json::Value appResult;
	
	appResult["command"] = "server_status_result";

	
	ret = GDocOpenofficeTsQueue.GetTsStatus(DocServerNum);
	if(ret == 0){
		appResult["doc_trans"] = "";
	} else {
		appResult["doc_trans"] = DocServerNum;
	}

	
	ret = GDocOfficeTsQueue.GetTsStatus(DocOfficeServerNum);
	if(ret == 0){
		appResult["doc_windows_trans"] = "";
	} else {
		appResult["doc_windows_trans"] = DocOfficeServerNum;
	}

	ret = GDocWpsTsQueue.GetTsStatus(DocWpsServerNum);
	if(ret == 0){
		appResult["doc_wps_trans"] = "";
	} else {
		appResult["doc_wps_trans"] = DocWpsServerNum;
	}

	ret = GVideoTsQueue.GetTsStatus(VideoServerNum);
	if(ret == 0){
		appResult["video_trans"] = "";
	} else {
		appResult["video_trans"] = VideoServerNum;
	}

	ret = GDpptTsQueue.GetTsStatus(PptServerNum);
	if(ret == 0){
		appResult["dynamic_ppt_trans"] = "";
	} else {
		appResult["dynamic_ppt_trans"] = PptServerNum;
	}

	appResultStr = appResult.toStyledString()+"\0";
	
	return true;
}






////////////////////////// AppCmdVideoMessage //////////////////////////////
AppCmdVideoMessage::AppCmdVideoMessage()
{
	m_commandName = "video_message_task";

	//m_tsType = TS_VIDEO;
	m_taskType = VideoTask;

	m_defaultTsCmd = &GTsCmdVideoMessage;
}
bool AppCmdVideoMessage::BuildAppResult(const  Json::Value & tsResult, string & appResultStr)
{
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();
	unsigned int code,duration,frameRate,bitRate;

	Json::Value appResult;
	
	appResult["command"] = "video_message_result";

	_common_build_app_result(appResult,tsResult,code);

	if(!tsResult["errorcode"].isNull() && tsResult["errorcode"].isString() &&("0" != tsResult["errorcode"].asString())){
		appResultStr = appResult.toStyledString();

		return true;   
	}
	if(!tsResult["duration"].isNull() && tsResult["duration"].isString()){
		duration = atoll(tsResult["duration"].asString().c_str());
		appResult["duration"] = duration;
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from from ts result. No \"duration\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	if(!tsResult["resolution"].isNull() && tsResult["resolution"].isString()){
		appResult["resolution"] = tsResult["resolution"].asString();
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from from ts result. No \"resolution\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	if(!tsResult["framerate"].isNull() && tsResult["framerate"].isString()){
		frameRate = atoi(tsResult["framerate"].asString().c_str());
		appResult["framerate"] = frameRate;
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from from ts result. No \"framerate\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	if(!tsResult["audiocode"].isNull() && tsResult["audiocode"].isString()){
		appResult["audiocode"] = tsResult["audiocode"].asString();
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from ts result. No \"audiocode\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	if(!tsResult["videocode"].isNull() && tsResult["videocode"].isString()){
		appResult["videocode"] = tsResult["videocode"].asString();
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from from ts result. No \"videocode\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	if(!tsResult["bitrate"].isNull() && tsResult["bitrate"].isString()){
		bitRate = atoi(tsResult["bitrate"].asString().c_str());
		appResult["bitrate"] = frameRate;
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from ts result. No \"bitrate\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	if(!tsResult["audiotrack"].isNull() && tsResult["audiotrack"].isString()){
		appResult["audiotrack"] = tsResult["audiotrack"].asString();
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from ts result. No \"audiotrack\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	if(!tsResult["videotype"].isNull() && tsResult["videotype"].isString()){
		appResult["videotype"] = tsResult["videotype"].asString();
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from ts result. No \"videotype\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	appResultStr = appResult.toStyledString();

	return true;
}






////////////////////////// AppCmdVideoTrans //////////////////////////////
AppCmdVideoTrans::AppCmdVideoTrans()
{
	m_commandName = "video_trans_task";
	//m_tsType = TS_VIDEO;
	m_taskType = VideoTask;
	m_defaultTsCmd = &GTsCmdVideoRequest;

}
bool AppCmdVideoTrans::BuildAppResult(const Json::Value& tsResult,string& appResultStr)
{
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();
	unsigned int code;

	cout << " ++++++ BuildAppResult    1   \n tsResult:" << tsResult.toStyledString() << endl;
	
	Json::Value appResult;
	
	appResult["command"] = "video_trans_result";

	_common_build_app_result(appResult,tsResult,code);

	if(!tsResult["errorcode"].isNull() && tsResult["errorcode"].isString() &&("0" != tsResult["errorcode"].asString())){
		appResultStr = appResult.toStyledString();
		cout << " ++++++ BuildAppResult    1-1"  << endl;
		return true;   
	}
	cout << " ++++++ BuildAppResult    2      tsResult[\"file\"]:" << tsResult["file"] << endl;
	if(!tsResult["file"].isNull() && tsResult["file"].isString()){
		cout << " ++++++ BuildAppResult    4"  << endl;
		appResult["file"] = tsResult["file"].asString();
	} else {
		cout << " ++++++ BuildAppResult    5"  << endl;
		dsLog.Log(false,LOGGER_ERROR,"error message from ts result. No \"file\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	appResultStr = appResult.toStyledString();
	cout << " ++++++ BuildAppResult    6"  << endl;
	return true;
}





////////////////////////// AppCmdPrintScreen //////////////////////////////
AppCmdPrintScreen::AppCmdPrintScreen()
{
	m_commandName = "print_screen_task";
	//m_tsType = TS_VIDEO;
	m_taskType = VideoTask;
	m_defaultTsCmd = &GTsCmdPrintScreen;

}
bool AppCmdPrintScreen::BuildAppResult(const  Json::Value & tsResult, string & appResultStr)
{
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();
	unsigned int code,count;

	Json::Value appResult;
	
	appResult["command"] = "print_screen_result";

	_common_build_app_result(appResult,tsResult,code);

	if(!tsResult["errorcode"].isNull() && tsResult["errorcode"].isString() &&("0" != tsResult["errorcode"].asString())){
		appResultStr = appResult.toStyledString();

		return true;   
	}

	if(!tsResult["count"].isNull() && tsResult["count"].isString()){
		count = atoi(tsResult["count"].asString().c_str());
		appResult["count"] = code;
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from ts result. No \"count\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	appResultStr = appResult.toStyledString();
	
	return true;
}






////////////////////////// AppCmdVideoCut //////////////////////////////
AppCmdVideoCut::AppCmdVideoCut()
{
	m_commandName = "video_cut_task";

	//m_tsType = TS_VIDEO;
	m_taskType = VideoTask;

	m_defaultTsCmd = &GTsCmdVideoCut;
}
bool AppCmdVideoCut::BuildAppResult(const Json::Value& tsResult,string& appResultStr)
{
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();
	unsigned int code;

	Json::Value appResult;
	
	appResult["command"] = "video_cut_result";

	_common_build_app_result(appResult,tsResult,code);

	if(!tsResult["errorcode"].isNull() && tsResult["errorcode"].isString() &&("0" != tsResult["errorcode"].asString())){
		appResultStr = appResult.toStyledString();

		return true;   
	}

    
	if(!tsResult["targetFile"].isNull() && tsResult["targetFile"].isString()){
		appResult["targetFile"] = tsResult["targetFile"].asString();
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from ts result. No \"targetFile\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}
	

    appResultStr = appResult.toStyledString();
	
	return true;
}





////////////////////////// AppCmdVideoMerge //////////////////////////////
AppCmdVideoMerge::AppCmdVideoMerge()
{
	m_commandName = "video_merge_task";

	//m_tsType = TS_VIDEO;
	m_taskType = VideoTask;

	m_defaultTsCmd = &GTsCmdVideoMerge;
}
bool AppCmdVideoMerge::BuildAppResult(const Json::Value& tsResult,string& appResultStr)
{

	Json::Value appResult;


	appResultStr = appResult.toStyledString();
	
	return true;
}




////////////////////////// AppCmdDocTrans //////////////////////////////
AppCmdDocTrans::AppCmdDocTrans()
{
	m_commandName = "doc_trans_task";

	m_taskType = DocTask;

	m_defaultTsCmd = &GTsCmdDocTrans;
}

bool AppCmdDocTrans::BuildAppResult(const Json::Value& tsResult,string& appResultStr)
{
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();
	unsigned int code;

	Json::Value appResult;
    
	appResult["command"] = "doc_trans_result";

	_common_build_app_result(appResult,tsResult,code);

	if(!tsResult["errorcode"].isNull() && tsResult["errorcode"].isString() &&("0" != tsResult["errorcode"].asString())){
		appResultStr = appResult.toStyledString();

		return true;   
	}

	if(!tsResult["pages"].isNull() && tsResult["pages"].isString()){
		appResult["countpage"] = tsResult["pages"].asString();
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from ts result. No \"pages\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}
	appResultStr = appResult.toStyledString();
	
	return true;
}




////////////////////////// AppCmdDynamicPptTrans //////////////////////////////
AppCmdDynamicPptTrans::AppCmdDynamicPptTrans()
{
	m_commandName = "dynamic_ppt_trans_task";

	//m_tsType = TS_DYNAMIC_PPT;
	m_taskType = DpptTask;

	m_defaultTsCmd = &GTsCmdDynamicPptTrans;
}


bool AppCmdDynamicPptTrans::BuildAppResult(const Json::Value& tsResult,string& appResultStr)
{

	LoggerWrapper dsLog= LoggerWrapper::GetInstance();
	unsigned int code;

	Json::Value appResult;
    
	appResult["command"] = "dynamic_ppt_trans_result";

	_common_build_app_result(appResult,tsResult,code);

	if(!tsResult["errorcode"].isNull() && tsResult["errorcode"].isString() &&("0" != tsResult["errorcode"].asString())){
		appResultStr = appResult.toStyledString();

		return true;   
	}

	if(!tsResult["pages"].isNull() && tsResult["pages"].isString()){
		appResult["countpage"] = tsResult["pages"].asString();
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from ts result. No \"pages\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}
	appResultStr = appResult.toStyledString();
	
	return true;
}





////////////////////////// AppCmdTaskProgress //////////////////////////////

bool  AppCmdTaskProgress::BuildAppResponse(const Json::Value& appCmd,string& response)
{
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();
	strstream ssPosi;  
	string sPosi;  
	int progress;
	Task* task;
	unsigned int position;

	Json::Value appResult;
	
	appResult["command"] = "video_progress_result";
	if(appCmd["task_type"].isString())
		appResult["task_type"] = appCmd["task_type"].asString();
	else
		dsLog.Log(true,LOGGER_ERROR," task_type is not string! \n");

	if(appCmd["id"].isString())
		appResult["id"] = appCmd["id"].asString();
	else
		dsLog.Log(true,LOGGER_ERROR," id is not string! \n");

	task = GVideoTaskRunQueue.GetTaskInSpecId(appCmd["id"].asString());
	if(task){
		progress = task->GetProgress();
		ssPosi << progress;
		ssPosi >> sPosi;
		appResult["task_execute_progress"] = sPosi;
		appResult["wait_queue_position"] = "0";
	} else {
		appResult["task_execute_progress"] = "0";
		task = GVideoTaskWaitQueue.GetTaskInSpecId(appCmd["id"].asString(),position);
		if(task){
			ssPosi << (position+1);  
			ssPosi >> sPosi;
			appResult["wait_queue_position"] = sPosi;
		} else {
			appResult["wait_queue_position"] = "0";
		}
	}
	

	response = appResult.toStyledString()+"\0";

	return true;
}




////////////////////////// AppCmdAudioTrans //////////////////////////////
AppCmdAudioTrans::AppCmdAudioTrans()
{
	m_commandName = "audio_trans_task";
	//m_tsType = TS_VIDEO;
	m_taskType = VideoTask;

	m_defaultTsCmd = &GTsCmdAudioTrans;
}
bool AppCmdAudioTrans::BuildAppResult(const Json::Value& tsResult,string& appResultStr)
{
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();
	unsigned int code;

	Json::Value appResult;
	
	appResult["command"] = "audio_trans_result";

	_common_build_app_result(appResult,tsResult,code);

	if(!tsResult["errorcode"].isNull() && tsResult["errorcode"].isString() &&("0" != tsResult["errorcode"].asString())){
		appResultStr = appResult.toStyledString();

		return true;   
	}

	if(!tsResult["file"].isNull() && tsResult["file"].isString()){
		appResult["file"] = tsResult["file"].asString();
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from ts result. No \"file\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	appResultStr = appResult.toStyledString();
	
	return true;
}





////////////////////////// AppCmdAudioMessage //////////////////////////////
AppCmdAudioMessage::AppCmdAudioMessage()
{
	m_commandName = "audio_message_task";

	//m_tsType = TS_VIDEO;
	
	m_taskType = VideoTask;

	m_defaultTsCmd = &GTsCmdAudioMessage;

}



bool AppCmdAudioMessage::BuildAppResult(const Json::Value& tsResult,string& appResultStr)
{
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();
	unsigned int code,duration;//,frameRate,bitRate;

	Json::Value appResult;
	
	appResult["command"] = "audio_message_result";


	_common_build_app_result(appResult,tsResult,code);

	if(!tsResult["errorcode"].isNull() && tsResult["errorcode"].isString() &&("0" != tsResult["errorcode"].asString())){
		appResultStr = appResult.toStyledString();

		return true;   
	}

	if(!tsResult["duration"].isNull() && tsResult["duration"].isString()){
		duration = atoll(tsResult["duration"].asString().c_str());
		appResult["duration"] = duration;
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from from ts result. No \"duration\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}
	
	appResultStr = appResult.toStyledString();

	return true;
}

///////////////////////////////// TsCommand /////////////////////////////
bool TsCommand::CommBuildTsCmd(const  Json::Value& appCmd, Json::Value& tsCmd)
{
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();
	
	tsCmd["type"] = m_tsResultName;
	tsCmd["id"] = appCmd["id"].asString();
	
	if(!appCmd["file"].isNull() && appCmd["file"].isString()){
		tsCmd["file"] = appCmd["file"].asString();
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from application command. No \"file\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}
	
	return true;
}


////////////////////////// specific ts command /////////////////////////////

bool  TsCmdVideoMessage::BuildTsCmd(const Json::Value& appCmd,string& tscmd)
{
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();
	Json::Value tsCmd;

/*
	tsCmd["type"] = "Message";
	tsCmd["id"] = appCmd["id"].asString();
	
	if(!appCmd["file"].isNull() && appCmd["file"].isString()){
		tsCmd["file"] = appCmd["file"].asString();
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from application command. No \"file\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}
*/
	if(CommBuildTsCmd(appCmd,tsCmd))
		tscmd = tsCmd.toStyledString();
	else 
		return false;
	
	return true;
}


bool TsCmdVideoTrans::BuildTsCmd(const Json::Value& appCmd,string& tscmd)
{
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();
	Json::Value tsCmd;
	stringstream ss,ss1;
	//string tscmd;

/*
	tsCmd["type"] = "Transfer";
	tsCmd["id"] = appCmd["id"].asString();
	
	if(!appCmd["file"].isNull() && appCmd["file"].isString()){
		tsCmd["file"] = appCmd["file"].asString();
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from application command. No \"file\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return "";
	}
*/
	if(CommBuildTsCmd(appCmd,tsCmd))
		tscmd = tsCmd.toStyledString();
	else 
		return false;

	if(!appCmd["targetFile"].isNull() && appCmd["targetFile"].isString()){
		tsCmd["targetfolder"] = appCmd["targetFile"].asString();
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from application command. No \"targetFile\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	if(!appCmd["format"].isNull() && appCmd["format"].isInt()){
		ss << appCmd["format"].asInt();
		tsCmd["format"] = ss.str();
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from application command. No \"format\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	if(!appCmd["bitrate"].isNull() && appCmd["bitrate"].isInt()){
		ss1 << appCmd["bitrate"].asInt();
		tsCmd["bitrate"] = ss1.str();
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from application command. No \"bitrate\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	if(!appCmd["resolution"].isNull() && appCmd["resolution"].isString()){
		tsCmd["resolution"] = appCmd["resolution"].asString();
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from application command. No \"resolution\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	tscmd = tsCmd.toStyledString();
	
	return true;
}

bool TsCmdVideoRequest::BuildTsCmd(const Json::Value& appCmd,string& tscmd)
{
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();
	Json::Value tsCmd;
	stringstream ss,ss1;


	tsCmd["type"] = "VideoTransRequest";
	tsCmd["id"] = appCmd["id"].asString();
	
	if(!appCmd["file"].isNull() && appCmd["file"].isString()){
		tsCmd["file"] = appCmd["file"].asString();
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from application command. No \"file\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

    if(!appCmd["targetFile"].isNull() && appCmd["targetFile"].isString()){
		tsCmd["targetfolder"] = appCmd["targetFile"].asString();
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from application command. No \"targetFile\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	if(!appCmd["format"].isNull() && appCmd["format"].isInt()){
		ss << appCmd["format"].asInt();
		tsCmd["format"] = ss.str();
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from application command. No \"format\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	if(!appCmd["bitrate"].isNull() && appCmd["bitrate"].isInt()){
		ss1 << appCmd["bitrate"].asInt();
		tsCmd["bitrate"] = ss1.str();
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from application command. No \"bitrate\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	if(!appCmd["resolution"].isNull() && appCmd["resolution"].isString()){
		tsCmd["resolution"] = appCmd["resolution"].asString();
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from application command. No \"resolution\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	tscmd = tsCmd.toStyledString();
	
	return true;
}



bool TsCmdPrintScreen::BuildTsCmd(const Json::Value& appCmd,string& tscmd)
{
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();
	Json::Value tsCmd;
	stringstream start,interval,count;


	if(CommBuildTsCmd(appCmd,tsCmd))
		tscmd = tsCmd.toStyledString();
	else 
		return false;

	
	if(!appCmd["targetFolder"].isNull() && appCmd["targetFolder"].isString()){
		tsCmd["targetfolder"] = appCmd["targetFolder"].asString();
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from application command. No \"targetFolder\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	
	if(!appCmd["prefix"].isNull() && appCmd["prefix"].isString()){
		tsCmd["prefix"] = appCmd["prefix"].asString();
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from application command. No \"prefix\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	if(!appCmd["start"].isNull() && appCmd["start"].isIntegral()){
		start << appCmd["start"].asInt();
		tsCmd["start"] = start.str();
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from application command. No \"start\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	if(!appCmd["interval"].isNull() && appCmd["interval"].isIntegral()){
		interval << appCmd["interval"].asInt();
		tsCmd["interval"] = interval.str();
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from application command. No \"interval\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	if(!appCmd["count"].isNull() && appCmd["count"].isIntegral()){
		count << appCmd["count"].asInt();
		tsCmd["count"] = start.str();
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from application command. No \"count\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	tscmd = tsCmd.toStyledString();
	
	return true;
}




bool TsCmdVideoCut::BuildTsCmd(const Json::Value& appCmd,string& tscmd)
{
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();
	Json::Value tsCmd;
	stringstream starttime,endtime;


	tsCmd["type"] = "VideoCut";
	tsCmd["id"] = appCmd["id"].asString();
	
	if(!appCmd["file"].isNull() && appCmd["file"].isString()){
		tsCmd["file"] = appCmd["file"].asString();
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from application command. No \"file\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}
	
	if(!appCmd["targetFile"].isNull() && appCmd["targetFile"].isString()){
		tsCmd["targetFile"] = appCmd["targetFile"].asString();
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from application command. No \"targetFile\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}


	if(!appCmd["starttime"].isNull() && appCmd["starttime"].isIntegral()){
		starttime << appCmd["starttime"].asInt();
		tsCmd["starttime"] = starttime.str();
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from application command. No \"starttime\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	if(!appCmd["endtime"].isNull() && appCmd["endtime"].isIntegral()){
		endtime << appCmd["endtime"].asInt();
		tsCmd["endtime"] = endtime.str();
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from application command. No \"endtime\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	tscmd = tsCmd.toStyledString();
	
	return true;
}



bool TsCmdVideoMerge::BuildTsCmd(const Json::Value& appCmd,string& tscmd)
{
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();
	Json::Value tsCmd;
	stringstream start,interval,count;


	tsCmd["type"] = "VideoMerge";
	tsCmd["id"] = appCmd["id"].asString();
	
	if(!appCmd["file"].isNull() && appCmd["file"].isString()){
		tsCmd["file"] = appCmd["file"].asString();
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from application command. No \"file\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}
	
	if(!appCmd["targetFile"].isNull() && appCmd["targetFile"].isString()){
		tsCmd["targetFile"] = appCmd["targetFile"].asString();
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from application command. No \"targetFolder\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	
	if(!appCmd["index"].isNull() && appCmd["index"].isString()){
		tsCmd["index"] = appCmd["index"].asString();
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from application command. No \"prefix\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}


	tscmd = tsCmd.toStyledString();
	
	return true;
}



bool TsCmdDocTrans::BuildTsCmd(const Json::Value& appCmd,string& tscmd)
{
	Json::Value tsCmd;
	
    
    LoggerWrapper dsLog= LoggerWrapper::GetInstance();

    if(!appCmd["type"].isNull() && appCmd["type"].isString())
    {
        tsCmd["transtype"] = appCmd["type"].asString();
    }
    else
    {
        dsLog.Log(false,LOGGER_ERROR,"error message from application command. No \"file\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
    }

    if(!appCmd["id"].isNull() && appCmd["id"].isString())
    {
        tsCmd["id"] = appCmd["id"].asString();
    }
    else
    {
        dsLog.Log(false,LOGGER_ERROR,"error message from application command. No \"file\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
    }

    if(!appCmd["file"].isNull() && appCmd["file"].isString())
    {
        tsCmd["file"] = appCmd["file"].asString();
    }
    else
    {
        dsLog.Log(false,LOGGER_ERROR,"error message from application command. No \"file\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
    }

    if(!appCmd["targetFolder"].isNull() && appCmd["targetFolder"].isString())
    {
        tsCmd["targetfolder"] = appCmd["targetFolder"].asString();
    }
    else
    {
        dsLog.Log(false,LOGGER_ERROR,"error message from application command. No \"file\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
    }

    if(!appCmd["prefix"].isNull() && appCmd["prefix"].isString())
    {
        tsCmd["prefix"] = appCmd["prefix"].asString();
    }
    else
    {
        dsLog.Log(false,LOGGER_ERROR,"error message from application command. No \"file\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
    }

    tsCmd["type"] = "Transfer";

    tscmd = tsCmd.toStyledString();
	//cout << "++++++ build TsCmdDocTrans: "<< tscmd << endl;
        
    return true;
}


bool TsCmdDynamicPptTrans::BuildTsCmd(const Json::Value& appCmd,string& tscmd)
{
	Json::Value tsCmd;
    
    LoggerWrapper dsLog= LoggerWrapper::GetInstance();

    if(!appCmd["type"].isNull() && appCmd["type"].isString())
    {
        tsCmd["transtype"] = appCmd["type"].asString();
    }
    else
    {
        dsLog.Log(false,LOGGER_ERROR,"error message from application command. No \"file\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
    }

    if(!appCmd["id"].isNull() && appCmd["id"].isString())
    {
        tsCmd["id"] = appCmd["id"].asString();
    }
    else
    {
        dsLog.Log(false,LOGGER_ERROR,"error message from application command. No \"file\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
    }

    if(!appCmd["file"].isNull() && appCmd["file"].isString())
    {
        tsCmd["file"] = appCmd["file"].asString();
    }
    else
    {
        dsLog.Log(false,LOGGER_ERROR,"error message from application command. No \"file\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
    }

    if(!appCmd["targetFolder"].isNull() && appCmd["targetFolder"].isString())
    {
        tsCmd["targetfolder"] = appCmd["targetFolder"].asString();
    }
    else
    {
        dsLog.Log(false,LOGGER_ERROR,"error message from application command. No \"file\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
    }

    if(!appCmd["prefix"].isNull() && appCmd["prefix"].isString())
    {
        tsCmd["prefix"] = appCmd["prefix"].asString();
    }
    else
    {
        dsLog.Log(false,LOGGER_ERROR,"error message from application command. No \"file\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
    }

    tsCmd["type"] = "Transfer";

    tscmd = tsCmd.toStyledString();
        
    return true;
}


bool  TsCmdAudioTrans::BuildTsCmd(const Json::Value& appCmd,string& tscmd)
{
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();
	Json::Value tsCmd;
	stringstream ss,ss1;


	tsCmd["type"] = "AudioTrans";
	tsCmd["id"] = appCmd["id"].asString();
	
	if(!appCmd["file"].isNull() && appCmd["file"].isString()){
		tsCmd["file"] = appCmd["file"].asString();
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from application command. No \"file\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

    if(!appCmd["targetFile"].isNull() && appCmd["targetFile"].isString()){
		tsCmd["targetFile"] = appCmd["targetFile"].asString();
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from application command. No \"targetFile\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	tscmd = tsCmd.toStyledString();
	return true;
}


bool  TsCmdAudioMessage::BuildTsCmd(const Json::Value& appCmd,string& tscmd)
{
	LoggerWrapper dsLog= LoggerWrapper::GetInstance();
	Json::Value tsCmd;
	
	tsCmd["type"] = "AudioMessage";
	tsCmd["id"] = appCmd["id"].asString();
	
	if(!appCmd["file"].isNull() && appCmd["file"].isString()){
		tsCmd["file"] = appCmd["file"].asString();
	} else {
		dsLog.Log(false,LOGGER_ERROR,"error message from application command. No \"file\".  [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return false;
	}

	tscmd = tsCmd.toStyledString();
	
	return true;
}


/////////////////////////////////////// AppCommandDb ////////////////////////////////////////
AppCommandDb::AppCommandDb()
{

	AddAppCommand(&GAppCmdServerStatus);
	AddAppCommand(&GAppCmdVideoMessage);
	AddAppCommand(&GAppCmdVideoTrans);
	AddAppCommand(&GAppCmdPrintScreen);
	AddAppCommand(&GAppCmdVideoMerge);
	AddAppCommand(&GAppCmdDocTrans);
	AddAppCommand(&GAppCmdDynamicPptTrans);
	AddAppCommand(&GAppCmdAudioTrans);
	AddAppCommand(&GAppCmdVideoCut);
	AddAppCommand(&GAppCmdTaskProgress);
	AddAppCommand(&GAppCmdAudioMessage);
    
}

AppCommandDb::~AppCommandDb()
{

}


bool AppCommandDb::AddAppCommand(AppCommand* cmd)
{
	m_commands[cmd->GetCommandName()] = cmd;
	return true;
}


bool AppCommandDb::GetCommandNameInSpecificTaskType(TaskType taskType,vector<string>& CommandNames)
{
	map<string,AppCommand *>::iterator it;
	
	for(it=m_commands.begin();it!=m_commands.end();++it){
		if(it->second->GetTaskType() == taskType)
			CommandNames.push_back(it->first);
	}

	return true;
}

bool AppCommandDb::GetResultNameInSpecificTaskType(TaskType taskType,vector<string>& CommandNames)
{
	map<string,TsCommand*>::iterator it;
	TsCommandDb* tsCmdDb = &TsCommandDb::GetInstance();
	
	if(taskType == NoTsTask){
		
		return false;
	}

	if(taskType == DocTask){
		AppCmdDocTrans::GetServerTypes(CommandNames);
		return true;
	}

	if(taskType == DpptTask){
		AppCmdDynamicPptTrans::GetServerTypes(CommandNames);
		return true;
	}

	if(taskType == VideoTask){
		for(it=tsCmdDb->m_commands.begin();it!=tsCmdDb->m_commands.end();++it){
			if(it->second->GetTsType() == TS_VIDEO){
				CommandNames.push_back(it->second->GetServerType());
			}
				//TODO: get result names
		}

		return true;
	}
	

	//CommandNames.push_back("Transfer");

	return false;
}



AppCommand* AppCommandDb::GetAppCommand(string cmdName)
{
	if(m_commands.find(cmdName) != m_commands.end()){
		return m_commands[cmdName];
	} else {
		return NULL;
	}
}


/////////////////////////////////////// CommandDb ////////////////////////////////////////
TsCommandDb::TsCommandDb()
{
	AddTsCommand(&GTsCmdVideoMessage);
	AddTsCommand(&GTsCmdVideoTrans);
	AddTsCommand(&GTsCmdVideoRequest);
	AddTsCommand(&GTsCmdPrintScreen);
	AddTsCommand(&GTsCmdVideoMerge);
	AddTsCommand(&GTsCmdDocTrans);
	AddTsCommand(&GTsCmdDynamicPptTrans);
	AddTsCommand(&GTsCmdAudioTrans);
	AddTsCommand(&GTsCmdVideoCut);
	AddTsCommand(&GTsCmdAudioMessage);
}

TsCommandDb::~TsCommandDb()
{
	
}


bool TsCommandDb::AddTsCommand(TsCommand* cmd)
{
	m_commands[cmd->GetCommandName()] = cmd;
	return true;
}



TsCommand* TsCommandDb::GetCommand(string cmdName)
{
	if(m_commands.find(cmdName) != m_commands.end()){
		return m_commands[cmdName];
	} else {
		return NULL;
	}
}









