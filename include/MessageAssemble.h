#ifndef _MESSAGE_ASSEMBLE_H
#define _MESSAGE_ASSEMBLE_H

#include <string>
//#include <sockLib.h>
#include "kdvtype.h"
#include <list>
//#include "InterfaceApi.h"


class RegisterInfo;
//class TsSession;
class TaskResult;
class AppMessageBase;
//class Connection;

using namespace std;
typedef list<std::string>  TransitionServerList;

typedef enum
{

    TS_DOCUMENT_LINUX = 0,

    TS_DOCUMENT_OFFICE = 1,

    TS_DOCUMENT_WPS = 2,

    TS_VIDEO = 3,

    TS_DYNAMIC_PPT =4

} TsType;


typedef enum
{
	TS_HEAVY = 0,
	TS_LIGHT = 1
}TsProperty;

typedef enum
{
	TASK_HEAVY = 0,
	TASK_LIGHT = 1
}TaskProperty;
#if 0
//时间信息
typedef struct
{
	//年(2006)
	unsigned int m_wYear;
	//月(1--12)
	unsigned int m_wMonth;
	//日(1--31)
	unsigned int m_wDay;
	//时(0--23)
	unsigned int m_wHour;
	//分(0--59)
	unsigned int m_wMinute;
	//秒(0--59)
	unsigned int m_wSecond;
}TaskTimeInfo;
#endif

#if 0
typedef enum
{

    MESSAGE_DOCUMENT_TRANS = 0,

    MESSAGE_DANAMIC_DOCUMENT_TRANS = 1,    
    	
    MESSAGE_VIDEO_TRANS = 2,

    MESSAGE_VIDEO_CAPATURE = 3

} MessageType;
#endif

typedef enum
{

    TS_MESSAGE_SVR_REGISTER = 0,

    TS_MESSAGE_REGISTER_RESPONSE = 1,

    TS_MESSAGE_DOC_PPT_TRANSMIT = 2,

    TS_MESSAGE_VIDEO_TRANSMIT = 3,

    TS_MESSAGE_VIDEO_CAPTURE = 4,

    TS_MESSAGE_VIDEO_INFO = 5,

    TS_MESSAGE_ACCEPT_TASK = 6,    

    TS_MESSAGE_TRANS_RESULT = 7,

    TS_MESSAGE_RESULT_RESPONSE = 8,

    TS_MESSAGE_SVRLINK_CHECK = 9,

    TS_MESSAGE_LINK_RESPONSE = 10,

	TS_MESSAGE_VIDEO_CUT = 11,

	TS_MESSAGE_VIDEO_MERGE = 12,
	
	TS_MESSAGE_PROGRESS_INFO = 13

} TsMessageType;

typedef enum
{

    TS_MESSAGE_DOC_LINUX_TRANS_RESULT = 0,

	TS_MESSAGE_DOC_OFFICE_TRANS_RESULT = 1,	

	TS_MESSAGE_DOC_WPS_TRANS_RESULT = 2,	

    TS_MESSAGE_DYNAMIC_PPT_RESULT = 3,

    TS_MESSAGE_VIDEO_TRANS_RESULT = 4,

    TS_MESSAGE_VIDEO_CAPTURE_RESULT = 5,    

    TS_MESSAGE_VIDEO_INFO_RESULT = 6,  
	
	TS_MESSAGE_VIDEO_CUT_RESULT = 7,

	TS_MESSAGE_VIDEO_MERGE_RESULT = 8

} TsTaskResultType;


typedef enum
{

    APP_MESSAGE_VIDEO_TRANS_TASK = 0,

    APP_MESSAGE_VIDEO_CAPTURE_TASK = 1,

    APP_MESSAGE_VIDEO_INFO_TASK = 2,

    APP_MESSAGE_VIDEO_TRANS_RESULT = 3,

    APP_MESSAGE_VIDEO_CAPTURE_RESULT = 4,

	APP_MESSAGE_VIDEO_INFO_RESULT = 5,

	APP_MESSAGE_DOC_TRANS_TASK = 6,

    APP_MESSAGE_DOC_TRANS_RESULT = 7,

    APP_MESSAGE_DYNAMIC_PPT_TRANS_TASK = 8,

    APP_MESSAGE_DYNAMIC_PPT_TRANS_RESULT = 9,

    APP_MESSAGE_SERVER_STATUS_TASK = 10,

    APP_MESSAGE_SERVER_STATUS_RESULT = 11,

	APP_MESSAGE_RESPONSE = 12,

	APP_MESSAGE_VIDEO_CUT_TASK = 13,

	APP_MESSAGE_VIDEO_CUT_RESULT = 14,

	APP_MESSAGE_VIDEO_MERGE_TASK = 15,

	APP_MESSAGE_VIDEO_MERGE_RESULT = 16,

	APP_MESSAGE_TASK_PROGRESS = 17,

	APP_MESSAGE_TASK_PROGRESS_RESULT = 18

//    APP_MESSAGE_PRINT_SCREEN_PROGRESS = 13

} AppMessageType;


typedef enum
{

    MESSAGE_DOCUMENT_SWF = 0,
    	
    MESSAGE_DOCUMENT_JPG = 1,

    MESSAGE_DOCUMENT_ALL = 2

} TransType;


typedef enum
{
	DOC_AUTO_SERVER = 0,       //按照优先级不指定具体转换服务器类型

    DOC_LINUX_SERVER = 1,
    	
    DOC_OFFICE_SERVER = 2,

    DOC_WPS_SERVER = 3

} DocTransServerType;


typedef struct tagCapVideoParam
{
    unsigned int dwStartTIme;
    unsigned int dwSustainedTime;
    unsigned int dwPhotoCount;
    unsigned int dwReserved1;
}TCapVideoParm;

typedef struct tagCutVideoParam
{
	unsigned int startTimeMs;
	unsigned int endTimeMs;
}TCutVideoParm;

typedef struct tagTransVideoParam
{
    unsigned int dwFormat;   
    unsigned int dwBitrate;
    unsigned int dwResolutionWidth;
    unsigned int dwResolutionHeight;
}TTransVideoParm;

typedef struct tagTransDocParam
{
    TransType enumTansType;
	//0为不指定，1为linux 的openoffice服务器，2为office服务器，3为wps服务器
	DocTransServerType enumAppointedServerType;   
	// 十六进制的个位是linux转换失败次数计数，十位是office转换失败次数计数，百位是wps转换失败次数计数
	//该部分组合顺序需和DcManage::m_strTsErrorIndex的初始值保持一致
    unsigned int dwFailedNumAssemble;           
    unsigned int dwReserved3;
}TTransDocParm;

typedef struct tagTransDynamicPPTParm
{
    unsigned int dwReserved0;
    unsigned int dwReserved1;
    unsigned int dwReserved2;
    unsigned int dwReserved3;
}TTransDynamicPPTParm;

typedef struct tagMergeVideoParm
{
    unsigned int dwIndex;
    unsigned int dwReserved1;
    unsigned int dwReserved2;
    unsigned int dwReserved3;
}TMergeVideoParm;


union UnionMessageParam 
{
    TTransDocParm unTransDoc;
    TTransDynamicPPTParm unTransDynamicPPT;
    TTransVideoParm unTransVideo;
    TCapVideoParm unCapVideo;
	TCutVideoParm unCutVideo;
	TMergeVideoParm unMergeVideo;
}; 


typedef struct tagVideoInfoResultParm
{
	string strResolution;
	string strAudioCode;
	string strVideoCode;
	string strAudioTrack;
	string strVideoType;
	unsigned int dwDuration;
	unsigned int dwFrameRate;
	unsigned int dwBitRate;
}TVideoInfoResultParm;


//Added by wwz 160422
//typedef struct tagTsResultParam
//{
//	string tsResult;
//	unsigned int tsPageNum;
//	unsigned int tsErrorCode;
//}TsResult;


class TsMessageBase //是作为和ts服务器之间消息的基类
{
protected:
    TsMessageType m_TsMessageType;
public:
    TsMessageBase(){ };
    void SetTsMessageType(TsMessageType type);
    TsMessageType GetTsMessageType();
	virtual void SetMessageId(string strMessageId){};
	virtual void SetSrcFile(string strSrcFile){};
	virtual void SetTargetFolder(string strTargetFolder){};
	virtual void SetFormat(string strFormat){};
    virtual void SetBitRate(string strBitRate){};
    virtual void SetResolution(string strResolution){};	
	virtual void SetPrefix(string strPrefix){};
    virtual void SetStart(string strStart){};
	virtual void SetCount(string strCount){};
    virtual void SetInterval(string strInterval){};
    virtual void SetStartTime(unsigned int startTimeMs){};
	virtual void SetEndTime(unsigned int startTimeMs){};
	virtual void SetFileIndex(string strFileIndex){};
    virtual bool Encode(string &json)= 0;
    virtual TsMessageBase* Decode(const string &json) = 0;

};

class AppMessageBase //是作为和app服务器之间消息的基类
{
protected:
    AppMessageType m_AppMessageType;
public:
    AppMessageBase();
    void SetAppMessageType(AppMessageType type);
    AppMessageType GetAppMessageType();    
    virtual bool Encode(string &json)= 0; 
    virtual AppMessageBase* Decode(const string &json) = 0;    
    virtual bool GetAppTargetInfo(string &strAppTargetIp,unsigned int &dwAppTargetPort) = 0; 
    virtual void SetMessageId(string strMessageId){};
    virtual void SetResult(bool bResult){};
    virtual void SetErrorCode(unsigned int dwErrorCode){};
	virtual void SetPages(unsigned int dwPages){};
	virtual void SetTargetFullPathFileName(string strTargetFullPathFileName){};
	virtual void SetCount(unsigned int dwCount){};
	virtual void SetDuration(unsigned int dwDuration){};
	virtual void SetResolution(string strResolution){};
	virtual void SetAudioCode(string strAudioCode){};
	virtual void SetVideoCode(string strVideoCode){};
	virtual void SetAudioTrack(string strAudioTrack){};
	virtual void SetVideoType(string strVideoType){};
	virtual void SetFrameRate(unsigned int dwFrameRate){};
	virtual void SetBitRate(unsigned int dwBitRate){};	
    virtual void SetAppTargetIp(string strAppTargetIp){};
    virtual void SetAppTargetPort(unsigned int dwAppTargetPort){};

};


class AppTaskMessage  :  public AppMessageBase
{
    string m_strFullPathFileName;
    string m_strTargetFullPathFileName;
    string m_strFilePrefix;

    string m_strMessageId;	
    string m_strIpaddr;
    unsigned int m_dwPort;
    unsigned int m_dwTsNo;
    unsigned int m_dwMessagePriority;
    TaskProperty m_TaskProperty;
    unsigned int m_dwRepeatTimes;
    UnionMessageParam m_unMessagePara;

	unsigned int m_dwTaskTimeTick;
	string m_taskType;
	string m_taskId;

	//Added 160419
	string m_tsIp;
	string m_tsId;
	string m_tsResult;
public:
    AppTaskMessage();
	AppTaskMessage(const AppTaskMessage& m_appTaskMessage);

    unsigned int GetPriority();
    void SetPriority(unsigned int dwPriority);
    AppTaskMessage* GetNextMessage();

    //bool operator<( AppTaskMessage*  a);
	AppTaskMessage& operator=(const AppTaskMessage& other);

    string GetMessageId();
    void SetMessageId(string strMessageId);
    string GetFullPathFileName();
    void SetFullPathFileName(string str);
    string GetTargetFullPathFileName();
    void SetTargetFullPathFileName(string str);
    string GetFilePrefix();
    void SetFilePrefix(string str);
    string GetIpAddr();
    void SetIpAddr(string strIpAddr);
    unsigned int GetPort();
    void SetPort(unsigned int dwPort);	
    unsigned int GetTsNum();
    void SetTsNum(unsigned int dwTsNum);	
    TaskProperty GetTaskProperty(){return m_TaskProperty;}
    void SetTaskProperty();
    void SetTaskProperty(TaskProperty property);
    unsigned int GetRepeatedTimes();
    void SetRepeatedTimes(unsigned int dwRepeatedTimes);
    UnionMessageParam GetMessagePara();
    void SetMessagePara(UnionMessageParam unMessagePara);	
	void SetTaskTimeInfo(unsigned int dwTaskTimeInfo);
	unsigned int GetTaskTimeInfo();
    bool Initialize();
    virtual bool Encode(string &json);
    virtual AppMessageBase* Decode(const string &json) ;    
    bool GetAppTargetInfo(string &strAppTargetIp,unsigned int &dwAppTargetPort) ;
	string GetTaskType();
	string GetTaskId();

	//Added by wwz 160419
	string GetTsIp();
    void SetTsIp(string tsIp);
	string GetTsId();
	void SetTsId(string tsId);
	string GetTsResult();
	void SetTsResult(string tsResult);
};   


class AppResponseMessage  :  public AppMessageBase
{
    unsigned int m_dwErrorCode;
    string m_strErrorDetail;
public:
    AppResponseMessage();
    virtual bool Encode(string &json);
    virtual AppMessageBase* Decode(const string &json) ;
    bool GetAppTargetInfo(string &strAppTargetIp,unsigned int &dwAppTargetPort) ;   
    void SetErrorInfo(unsigned int dwErrorcode, string strErrorDetail);
};

class AppVideoTransResultMessage  :  public AppMessageBase
{
	//{"command":" video_trans_result ", "id":"FFFFFFFFFFFFFF", "file":"\\10.1.200.101\somepath\somefile.trans.mp4", " status":true, "code":0}
    string m_stringMessageId;
    string m_strTargetFullPathFileName;
    bool m_bResult;
    unsigned int m_dwErrorCode;
    string m_strAppTargetIp;
    unsigned int m_dwAppTargetPort;	
public:
    AppVideoTransResultMessage();
    void SetMessageId(string strMessageId);
	void SetTargetFullPathFileName(string strTargetFullPathFileName);
    void SetResult(bool bResult);
    void SetErrorCode(unsigned int dwErrorCode);
    void SetAppTargetIp(string strAppTargetIp);
    void SetAppTargetPort(unsigned int dwAppTargetPort);	
    virtual bool Encode(string &json);
    virtual AppMessageBase* Decode(const string &json) ; 
    bool GetAppTargetInfo(string &strAppTargetIp,unsigned int &dwAppTargetPort);    
};

#if 0
class AppPrintScreenProgressMessage  :  public AppMessageBase
{
//{"command":" print_screen_progress","id":"FFFFFFFFFFFFFF",  "progress":50}
    string m_stringMessageId;
    unsigned int m_dwProgress;
public:
    AppPrintScreenProgressMessage();
    virtual bool Encode(string &json);
    virtual AppMessageBase* Decode(const string &json) ; 
    bool GetAppTargetInfo(string &strAppTargetIp,unsigned int &dwAppTargetPort);    
};
#endif

class AppVideoCaptureResultMessage  :  public AppMessageBase
{
	//{"command":"print_screen_result","id":"FFFFFFFFFFFFFF",  "count":50,  "status":true, "code":0}
    string m_stringMessageId;
    unsigned int m_dwCount;
    bool m_bResult;
    unsigned int m_dwErrorCode;
    string m_strAppTargetIp;
    unsigned int m_dwAppTargetPort;	
public:
    AppVideoCaptureResultMessage();
    void SetMessageId(string strMessageId);
    void SetResult(bool bResult);
	void SetCount(unsigned int dwCount);
    void SetErrorCode(unsigned int dwErrorCode);
    void SetAppTargetIp(string strAppTargetIp);
    void SetAppTargetPort(unsigned int dwAppTargetPort);	
    virtual bool Encode(string &json);
    virtual AppMessageBase* Decode(const string &json) ; 
    bool GetAppTargetInfo(string &strAppTargetIp,unsigned int &dwAppTargetPort);
};

class AppVideoInfoResultMessage  :  public AppMessageBase
{
	//{"commmand":" video_message_result","id":"FFFFFFFFFFFFFFFF","duration":"00:48:09","resolution":"1024*768","framerate":8,"audiocode":"aac","videocode":"h264","bitrate":128,"audiotrack":"双声道","videotype":"2d","status":true, "code":0}

    string m_stringMessageId;	
	string m_strResolution;
	string m_strAudioCode;
	string m_strVideoCode;
	string m_strAudioTrack;
	string m_strVideoType;
    string m_strAppTargetIp;
	unsigned int m_dwDuration;
	unsigned int m_dwFrameRate;
	unsigned int m_dwBitRate;
	unsigned int m_dwAppTargetPort;	
	bool m_bResult;
	unsigned int m_dwErrorCode;
public:
    AppVideoInfoResultMessage();
    void SetMessageId(string strMessageId);
	void SetResult(bool bResult);
	void SetErrorCode(unsigned int dwErrorCode);
	void SetDuration(unsigned int dwDuration);
	void SetResolution(string strResolution);
	void SetAudioCode(string strAudioCode);
	void SetVideoCode(string strVideoCode);
	void SetAudioTrack(string strAudioTrack);
	void SetVideoType(string strVideoType);
	void SetFrameRate(unsigned int dwFrameRate);
	void SetBitRate(unsigned int dwBitRate);
    void SetAppTargetIp(string strAppTargetIp);
    void SetAppTargetPort(unsigned int dwAppTargetPort);	
    virtual bool Encode(string &json);
    virtual AppMessageBase* Decode(const string &json) ; 
    bool GetAppTargetInfo(string &strAppTargetIp,unsigned int &dwAppTargetPort);
};




class AppVideoCutResultMessage  :  public AppMessageBase
{
	//{"command":"video_cut_result", "id":"FFFFFFFFFFFFFF", "targetFile":"\\10.1.200.101\data\somefolder\somefile1.mp4", "status":"true", "code":0}

	AppMessageType m_AppMessageType;
	string m_stringMessageId;	
	string m_strAppTargetIp;
	string m_strTargetFullPathFileName;
	unsigned int m_dwAppTargetPort;	
	bool m_bResult;
	unsigned int m_dwErrorCode;
public:
    AppVideoCutResultMessage();
	void SetTargetFullPathFileName(string strTargetFullPathFileName);
    void SetMessageId(string strMessageId);
	void SetResult(bool bResult);
	void SetErrorCode(unsigned int dwErrorCode);
    void SetAppTargetIp(string strAppTargetIp);
    void SetAppTargetPort(unsigned int dwAppTargetPort);	
    virtual bool Encode(string &json);
    virtual AppMessageBase* Decode(const string &json) ; 
    bool GetAppTargetInfo(string &strAppTargetIp,unsigned int &dwAppTargetPort);
};

class AppVideoMergeResultMessage  :  public AppMessageBase
{
	//{"command":"video_merge_result", "id":"FFFFFFFFFFFFFF",	 "status":"true", "code":0}

    string m_stringMessageId;
    string m_strAppTargetIp;
    bool m_bResult;
    unsigned int m_dwErrorCode;
    unsigned int m_dwAppTargetPort;	
public:
    AppVideoMergeResultMessage();
    void SetMessageId(string strMessageId){m_stringMessageId = strMessageId;};
    void SetResult(bool bResult){m_bResult = bResult;};
    void SetErrorCode(unsigned int dwErrorCode){m_dwErrorCode = dwErrorCode;};
    void SetAppTargetIp(string strAppTargetIp){m_strAppTargetIp = strAppTargetIp;};
    void SetAppTargetPort(unsigned int dwAppTargetPort){m_dwAppTargetPort = dwAppTargetPort;};	
    virtual bool Encode(string &json);
    virtual AppMessageBase* Decode(const string &json){return this;} ; 
    bool GetAppTargetInfo(string &strAppTargetIp,unsigned int &dwAppTargetPort){	strAppTargetIp = m_strAppTargetIp;dwAppTargetPort = m_dwAppTargetPort;return true;};
};


class AppDocTransResultMessage  :  public AppMessageBase
{
	//{"command":" doc_trans_result","id":"FFFFFFFFFFFFFF", "countpage":5,"status":true, "code":0}
    string m_stringMessageId;
    bool m_bResult;
    unsigned int m_dwErrorCode;
	unsigned int m_dwPages;
    string m_strAppTargetIp;
    unsigned int m_dwAppTargetPort;
public:
    AppDocTransResultMessage();
    void SetMessageId(string strMessageId);
    void SetResult(bool bResult);
    void SetErrorCode(unsigned int dwErrorCode);
	void SetPages(unsigned int dwPages);
    void SetAppTargetIp(string strAppTargetIp);
    void SetAppTargetPort(unsigned int dwAppTargetPort);
    virtual bool Encode(string &json);
    virtual AppMessageBase* Decode(const string &json) ; 
    bool GetAppTargetInfo(string &strAppTargetIp,unsigned int &dwAppTargetPort);
};

class AppDynamicPptTransResultMessage  :  public AppMessageBase
{
	//{"command":" dynamic_ppt_trans_result","id":"FFFFFFFFFFFFFF", "countpage":5,"status":true, "code":0}
    string m_stringMessageId;
    bool m_bResult;
    unsigned int m_dwErrorCode;
	unsigned int m_dwPages;
    string m_strAppTargetIp;
    unsigned int m_dwAppTargetPort;
public:
    AppDynamicPptTransResultMessage();
    void SetMessageId(string strMessageId);
    void SetResult(bool bResult);
    void SetErrorCode(unsigned int dwErrorCode);
	void SetPages(unsigned int dwPages);
    void SetAppTargetIp(string strAppTargetIp);
    void SetAppTargetPort(unsigned int dwAppTargetPort);
    virtual bool Encode(string &json);
    virtual AppMessageBase* Decode(const string &json) ; 
    bool GetAppTargetInfo(string &strAppTargetIp,unsigned int &dwAppTargetPort);
};

class AppServerStatusQueryTask  :  public AppMessageBase
{
public:
    AppServerStatusQueryTask(){m_AppMessageType = APP_MESSAGE_SERVER_STATUS_TASK;};
    virtual bool Encode(string &json); 
    virtual AppMessageBase* Decode(const string &json) ;    
    virtual bool GetAppTargetInfo(string &strAppTargetIp,unsigned int &dwAppTargetPort); 

};

class AppTaskProgressReq  :  public AppMessageBase
{
	string m_taskType;
	string m_taskId;
public:
	AppTaskProgressReq(){m_AppMessageType = APP_MESSAGE_TASK_PROGRESS;};
	virtual bool Encode(string &json); 
	virtual AppMessageBase* Decode(const string &json) ; 
	virtual bool GetAppTargetInfo(string &strAppTargetIp,unsigned int &dwAppTargetPort);
	string GetTaskType();
	string GetTaskId();
};

class AppTaskProgressResult  :  public AppMessageBase
{
	string m_taskType;
	string m_taskId;
	string m_waitQueuePosition;
	string m_taskExecuteprogress;
public:
    AppTaskProgressResult(){m_AppMessageType = APP_MESSAGE_TASK_PROGRESS_RESULT;};
	void SetTaskType(string type);
	void SetTaskId(string id);
	void SetWaitQueuePosition(string pos);
	void SetTaskExecuteprogress(string progress);
	virtual bool GetAppTargetInfo(string &strAppTargetIp,unsigned int &dwAppTargetPort) ;
    virtual bool Encode(string &json); 
    virtual AppMessageBase* Decode(const string &json) ;    

};

class AppServerStatusResult  :  public AppMessageBase
{
    TransitionServerList m_strDocServerList;
    TransitionServerList m_strDocOfficeServerList;
    TransitionServerList m_strPptServerList;
//    TransitionServerList m_strPrintServerList;
    TransitionServerList m_strVideoServerList;
	TransitionServerList m_strWpsServerList;
public:
    AppServerStatusResult();
    void SetDocServerList(TransitionServerList DocTsList);
	void SetDocOfficeServerList(TransitionServerList DocOfficeTsList);
    void SetPptServerList(TransitionServerList PptTsList);
    void SetVideoServerList(TransitionServerList VideoTsList);  
	void SetDocWpsServerList(TransitionServerList WpsTsList);  
    virtual bool Encode(string &json); 
    virtual AppMessageBase* Decode(const string &json) ;    
    virtual bool GetAppTargetInfo(string &strAppTargetIp,unsigned int &dwAppTargetPort); 

};

#if 0
class AppErrorCodeMessage  :  public AppMessageBase
{
};
#endif


class RegisterInfo : public TsMessageBase
{
	// ip:xxxxxxxxx; id:xxxx;servertype:DOC
	string m_strIp;
	string m_strId;
	TsType m_TsType;
	TsProperty m_TsProperty;
public:
    RegisterInfo();
    string GetTsIp();
    string GetTsId();
    TsType GetTsType();
	TsProperty GetTsProperty(){return m_TsProperty;}
	void SetTsProperty(TsProperty property){m_TsProperty = property;}
    void SetTsIp(string strIp);
    void SetTsId(string strId);
    void SetTsType(TsType tsType);
    virtual bool Encode(string &json);
    virtual TsMessageBase* Decode(const string &json);
};


class RegisterResponse : public TsMessageBase
{
    //result:"Success/Fail",errorcode:1/2/3/…,errordetail:"…"
    bool m_bResult;
    unsigned int m_dwErrorCode;
    string m_strErrorDetail;
public:
    RegisterResponse();
    void SetResult(bool bResult);
    void SetErrorCode(unsigned int dwErrorCode);
    void SetErrorDetail(string strErrorDetail);
    virtual bool Encode(string &json);
    virtual TsMessageBase* Decode(const string &json);    
};


//{"command":" video_trans_result ", "id":"FFFFFFFFFFFFFF", "file":"\\10.1.200.101\somepath\somefile.trans.mp4", " status":true, "code":0}
//{type:"TransResult",servertype:"DYNAMIC_PPT",result="Success/Fail",id:"FFFFFFFFFFFFFF",file:"filename",targetfolder:"\\10.1.200.101\somefolder\",timeused:"0",pages="19",errorcode:"4004";errordetail:"not found"}
//{"command":" print_screen_progress","id":"FFFFFFFFFFFFFF",  "progress":50}
//{"command":"print_screen_result","id":"FFFFFFFFFFFFFF",  "count":50,  "status":true, "code":0}
//{"command":" doc_trans_result","id":"FFFFFFFFFFFFFF",  "status":true, "code":0}
class TaskResult : public TsMessageBase
{
protected:
    TsTaskResultType m_tsTaskResultType;
    string  m_strMessageId;
    string  m_strSrcFile;
    string  m_strTargetFolder;

public:
    TaskResult();
    void SetTsTaskResultType(TsTaskResultType type);
    TsTaskResultType GetTsTaskResultType();
    string GetMessageId();
    void SetMessageId(string strMessageId); 
    string GetSrcFile();
    void SetSrcFile(string strSrcFile);
    string GetTargetFolder();
    void SetTargetFolder(string strTargetFolder);    
    virtual bool GetResult(){return TRUE;};
    virtual  void SetResult(bool bResult){};
    virtual string GetErrorDetail(){return NULL;};
    virtual void SetErrorDetail(string strErrorDetail){};
    virtual unsigned int GetTimeUsed(){return 0;}
    virtual void SetTimeUsed(unsigned int dwTimeUsed){};
    virtual unsigned int GetPages(){return 0;};
    virtual void SetPages(unsigned int dwPages){};
    virtual unsigned int GetErrorCode(){return 0;};
    virtual void SetErrorCode(unsigned int dwErrorCode){};
	virtual void SetCount(unsigned int dwCount){};
	virtual unsigned int GetCount(){return 0;};	
    virtual string GetDuration(){return NULL;};
    virtual void SetDuration(string strDuration){};
    virtual string GetResolution(){return NULL;};
    virtual void SetResolution(string strResolution){};
    virtual string GetFrameRate(){return NULL;};
    virtual void SetFrameRate(string strFrameRate){};
    virtual string GetAudioCode(){return NULL;};
    virtual void SetAudioCode(string strAudioCode){};
    virtual void SetVideoCode(string strVideoCode){};
    virtual string GetVideoCode(){return NULL;};
    virtual void SetBitRate(string strBitRate){};
    virtual string GetBitRate(){return NULL;};
    virtual void SetAudioTrack(string strAudioTrack){};
    virtual string GetAudioTrack(){return NULL;};
    virtual void SetVideoType(string strVideoType){};
    virtual string GetVideoType(){return NULL;};		
    virtual bool Encode(string &json){return TRUE;};
    virtual TsMessageBase* Decode(const string &json){return this;}; 
};


class TaskProgressInfo : public TaskResult
{
	string m_strTaskId;
	string m_strTaskType;
	string m_progress;
public:
	TaskProgressInfo();
	string GetTaskId();
	string GetTaskType();
	string GetProgress();

	void SetTaskId(string taskId);
	void SetTaskType(string taskType);
	void SetProgress(string progress);

	virtual bool Encode(string &json);
	virtual TsMessageBase* Decode(const string &json);
};


//{type:"TransResult",result="Success/Fail",id:"FFFFFFFFFFFFFF",file:"filename",targetfolder:"\\10.1.200.101\somefolder\",timeused:"0",pages="19",errorcode:"4004";errordetail:"not found"}
class DocTaskResult : public TaskResult
{
    bool    m_bResult;
    unsigned int     m_dwTimeUsed;
    unsigned int     m_dwPages;
    unsigned int     m_dwErrorCode;
    string m_strErrorDetail;
public:
    DocTaskResult();
    bool GetResult();
    void SetResult(bool bResult);
    string GetErrorDetail();
    void SetErrorDetail(string strErrorDetail);
    unsigned int GetTimeUsed();
    void SetTimeUsed(unsigned int dwTimeUsed);
    unsigned int GetPages();
    void SetPages(unsigned int dwPages);
    unsigned int GetErrorCode();
    void SetErrorCode(unsigned int dwErrorCode);
    virtual bool Encode(string &json);
    virtual TsMessageBase* Decode(const string &json);  
};
	
//{type:"TransResult",result="Success/Fail",id:"FFFFFFFFFFFFFF",file:"filename",targetfolder:"\\10.1.200.101\somefolder\",timeused:"0",pages="19",errorcode:"4004";errordetail:"not found"}
class DocOfficeTaskResult : public TaskResult
{
	bool	m_bResult;
	unsigned int 	m_dwTimeUsed;
	unsigned int 	m_dwPages;
	unsigned int 	m_dwErrorCode;
	string m_strErrorDetail;
public:
	DocOfficeTaskResult();
	bool GetResult();
	void SetResult(bool bResult);
	string GetErrorDetail();
	void SetErrorDetail(string strErrorDetail);
	unsigned int GetTimeUsed();
	void SetTimeUsed(unsigned int dwTimeUsed);
	unsigned int GetPages();
	void SetPages(unsigned int dwPages);
	unsigned int GetErrorCode();
	void SetErrorCode(unsigned int dwErrorCode);
	virtual bool Encode(string &json);
	virtual TsMessageBase* Decode(const string &json);	
};

//{type:"TransResult",result="Success/Fail",id:"FFFFFFFFFFFFFF",file:"filename",targetfolder:"\\10.1.200.101\somefolder\",timeused:"0",pages="19",errorcode:"4004";errordetail:"not found"}
class DocWpsTaskResult : public TaskResult
{
	bool	m_bResult;
	unsigned int 	m_dwTimeUsed;
	unsigned int 	m_dwPages;
	unsigned int 	m_dwErrorCode;
	string m_strErrorDetail;
public:
	DocWpsTaskResult();
	bool GetResult(){return m_bResult;};
	void SetResult(bool bResult){m_bResult = bResult;};
	string GetErrorDetail(){return m_strErrorDetail;};
	void SetErrorDetail(string strErrorDetail){m_strErrorDetail = strErrorDetail;};
	unsigned int GetTimeUsed(){return m_bResult;};
	void SetTimeUsed(unsigned int dwTimeUsed){m_dwTimeUsed = dwTimeUsed;};
	unsigned int GetPages(){return m_dwPages;};
	void SetPages(unsigned int dwPages){m_dwPages = dwPages;};
	unsigned int GetErrorCode(){return m_dwErrorCode;};
	void SetErrorCode(unsigned int dwErrorCode){m_dwErrorCode = dwErrorCode;};
	virtual bool Encode(string &json){return true;};
	virtual TsMessageBase* Decode(const string &json);	
};

class PptTaskResult : public TaskResult
{
    bool    m_bResult;
    unsigned int     m_dwTimeUsed;
    unsigned int     m_dwPages;
    unsigned int     m_dwErrorCode;
    string m_strErrorDetail;
public:
    PptTaskResult();
    bool GetResult();
    void SetResult(bool bResult);
    string GetErrorDetail();
    void SetErrorDetail(string strErrorDetail);
    unsigned int GetTimeUsed();
    void SetTimeUsed(unsigned int dwTimeUsed);
    unsigned int GetPages();
    void SetPages(unsigned int dwPages);
    unsigned int GetErrorCode();
    void SetErrorCode(unsigned int dwErrorCode);
    virtual bool Encode(string &json);
    virtual TsMessageBase* Decode(const string &json);  
};

//{"type":"TransResult","result":"Success/Fail","servertype":"Transfer","id":"FFFFFFFFFFFFFF", "file":"filename","targetfile":"\\10.1.200.101\somepath\somefile.trans.mp4","errorcode":"4004";"errordetail":"not found"}
class VideoTransResult : public TaskResult
{
    bool    m_bResult;
    unsigned int     m_dwErrorCode;
    string m_strErrorDetail;
public:
    VideoTransResult();
    bool GetResult();
    void SetResult(bool bResult);
    string GetErrorDetail();
    void SetErrorDetail(string strErrorDetail);
    unsigned int GetErrorCode();
    void SetErrorCode(unsigned int dwErrorCode);

    virtual bool Encode(string &json);
    virtual TsMessageBase* Decode(const string &json);
};

//{"type":"TransResult","result":"Success/Fail","servertype":"CapVideo","id":"FFFFFFFFFFFFFF"," "file":"filename","targetfolder":"\\10.1.200.101\data\somefolder\somefile","errorcode":"4004";"errordetail":"not found"}
class VideoCaptureResult : public TaskResult
{
    bool    m_bResult;
	unsigned int     m_dwCount;
    unsigned int     m_dwErrorCode;
    string m_strErrorDetail;
public:
    VideoCaptureResult();
    bool GetResult();
    void SetResult(bool bResult);
	void SetCount(unsigned int dwCount);
	unsigned int GetCount();
    string GetErrorDetail();
    void SetErrorDetail(string strErrorDetail);
    unsigned int GetErrorCode();
    void SetErrorCode(unsigned int dwErrorCode);
    virtual bool Encode(string &json);
    virtual TsMessageBase* Decode(const string &json);
};

class VideoCutResult : public TaskResult
{
    bool    m_bResult;
	unsigned int     m_dwCount;
    unsigned int     m_dwErrorCode;
    string m_strErrorDetail;
public:
    VideoCutResult();
    bool GetResult();
    void SetResult(bool bResult);
	void SetCount(unsigned int dwCount);
	unsigned int GetCount();
    string GetErrorDetail();
    void SetErrorDetail(string strErrorDetail);
    unsigned int GetErrorCode();
    void SetErrorCode(unsigned int dwErrorCode);
    virtual bool Encode(string &json);
    virtual TsMessageBase* Decode(const string &json);
};

//{"type":"TransResult","result":"Success/Fail","servertype":"VideoMerge","id":"FFFFFFFFFFFFFF", "file":["\\10.1.200.101\data\somefold\somefile1.mp4","\\10.1.200.101\data\somefold\somefile2.flv", "\\10.1.200.101\data\somefold\somefile3.rmvb"],"targetFile":"\\10.1.200.101\somepath\somefile.trans.mp4","errorcode":"4004";"errordetail":"not found"}
class VideoMergeResult : public TaskResult
{
    bool    m_bResult;
    unsigned int     m_dwErrorCode;
    string m_strErrorDetail;
public:
    VideoMergeResult();
    bool GetResult(){return m_bResult;};
    void SetResult(bool bResult){m_bResult = bResult;};
    string GetErrorDetail(){return m_strErrorDetail;};
    void SetErrorDetail(string strErrorDetail){m_strErrorDetail = strErrorDetail;};
    unsigned int GetErrorCode(){return m_dwErrorCode;};
    void SetErrorCode(unsigned int dwErrorCode){m_dwErrorCode = dwErrorCode;};
    virtual bool Encode(string &json);
    virtual TsMessageBase* Decode(const string &json);
};

//{"type":"TransResult","servertype":"Message","id":"FFFFFFFFFFFFFF","file":"filename","duration":"00:48:09";"resolution":"1024*768","framerate":"8","audiocode":"aac","videocode":"h264","bitrate":"128","audiotrack":"双声道","videotype":"2d"}
class VideoInfoResult : public TaskResult
{
    bool    m_bResult;
    unsigned int     m_dwErrorCode;
	string m_strErrorDetail;
	string  m_strDuration;
	string  m_strResolution;
	string  m_strFrameRate;
	string  m_strAudioCode;
	string  m_strVideoCode;
	string  m_strBitRate;
	string  m_strAudioTrack;
	string  m_strVideoType;
public:
    VideoInfoResult();
    string GetDuration();
    void SetDuration(string strDuration);
    string GetResolution();
    void SetResolution(string strResolution);
    string GetFrameRate();
    void SetFrameRate(string strFrameRate);
    string GetAudioCode();
    void SetAudioCode(string strAudioCode);
    void SetVideoCode(string strVideoCode);
    string GetVideoCode();
    void SetBitRate(string strBitRate);
    string GetBitRate();
    void SetAudioTrack(string strAudioTrack);
    string GetAudioTrack();
    void SetVideoType(string strVideoType);
    string GetVideoType();	
    bool GetResult();
    void SetResult(bool bResult);	
    string GetErrorDetail();
    void SetErrorDetail(string strErrorDetail);
    unsigned int GetErrorCode();
    void SetErrorCode(unsigned int dwErrorCode);	
    virtual bool Encode(string &json);
    virtual TsMessageBase* Decode(const string &json);
};

#if 0
class VideoPrintProgressResult : public TaskResult
{

public:
    VideoPrintProgressResult(){};

    virtual bool Encode(string &json){return TRUE;};
    virtual TsMessageBase* Decode(const string &json){return this;};
};
#endif

class ResultResponse : public TsMessageBase
{
	//{"type":"ResultResponse", "servertype":" CapVideo ","id":"FFFFFFFFFFFFFF"}
	string m_strMessageId;
	string m_strServerType;
public:
    ResultResponse();
    void SetMessageId(string strMessageId);
    string GetMessageId();
    void SetServerType(string strServerType);
    string GetServerType();	
    virtual bool Encode(string &json);
    virtual TsMessageBase* Decode(const string &json);    

};

class TransitionRequest : public TsMessageBase
{
	//{type:"Transfer",id:"FFFFFFFFFFFFFF",file:"\\10.1.200.101\somepath\somefile.doc",targetfolder:"\\10.1.200.101\somefolder\",prefix:"FFFFFFFFF",transtype:"SWF"}
    string  m_strMessageId;
    string  m_strSrcFile;
    string  m_strTargetFolder;
    string  m_strPrefix;
    TransType m_TransType;
public:
    TransitionRequest();
    void SetMessageId(string strMessageId);
    string GetMessageId();
    void SetSrcFile(string strSrcFile);
    string GetSrcFile();
    void SetTargetFolder(string strTargetFolder);
    string GetTargetFolder();
    void SetPrefix(string strPrefix);
    string GetPrefix();
    void SetTransType(TransType tType);
    TransType GetTransType();
    virtual bool Encode(string &json);
    virtual TsMessageBase* Decode(const string &json);    

};

class TransVideoRequest : public TsMessageBase
{
	//"type":"Transfer","id":"FFFFFFFFFFFFFF","file":"\\10.1.200.101\somepath\somefile.mp4", "targetfolder":"\\10.1.200.101\somepath\somefile.trans.mp4","format": "1",	"bitrate"："8388608"，"resolution"："width*height"}
    string  m_strMessageId;
    string  m_strSrcFile;
    string  m_strTargetFolder;
	string  m_strFormat;
	string  m_strBitRate;
	string  m_strResolution;
public:
    TransVideoRequest();
    void SetMessageId(string strMessageId);
    string GetMessageId();
    void SetSrcFile(string strSrcFile);
    string GetSrcFile();
    void SetTargetFolder(string strTargetFolder);
    string GetTargetFolder();
    void SetFormat(string strFormat);
    string GetFormat();
    void SetBitRate(string strBitRate);
    string GetBitRate();
    void SetResolution(string strResolution);
    string GetResolution();
    virtual bool Encode(string &json);
    virtual TsMessageBase* Decode(const string &json);    

};

class CapVideoRequest : public TsMessageBase
{
	//	{"type":" CapVideo ","id":"FFFFFFFFFFFFFF","file":"\\10.1.200.101\data\somefold\somefile. mp4", "targetfolder":"\\10.1.200.101\data\somefolder\somefile ","prefix": "FFFFFFFFF",	"start"："10"，"count"："50"，"interval"："5"}
    string  m_strMessageId;
    string  m_strSrcFile;
    string  m_strTargetFolder;
    string  m_strPrefix;
	string  m_strStart;
	string  m_strCount;
	string  m_strInterval;
public:
    CapVideoRequest();
    void SetMessageId(string strMessageId);
    string GetMessageId();
    void SetSrcFile(string strSrcFile);
    string GetSrcFile();
    void SetTargetFolder(string strTargetFolder);
    string GetTargetFolder();
    void SetPrefix(string strPrefix);
    string GetPrefix();
    void SetStart(string strStart);
    string GetStart();    
	void SetCount(string strCount);
    string GetCount();
    void SetInterval(string strInterval);
    string GetInterval();
    virtual bool Encode(string &json);
    virtual TsMessageBase* Decode(const string &json);    

};


class CutVideoRequest : public TsMessageBase
{
	//	{"type":" CapVideo ","id":"FFFFFFFFFFFFFF","file":"\\10.1.200.101\data\somefold\somefile. mp4", "targetfolder":"\\10.1.200.101\data\somefolder\somefile ","prefix": "FFFFFFFFF",	"start"："10"，"count"："50"，"interval"："5"}
    string  m_strMessageId;
    string  m_strSrcFile;
    string  m_strTargetFolder;
	unsigned int m_startTimeMs;
	unsigned int m_endTimeMs;
public:
    CutVideoRequest();
    void SetMessageId(string strMessageId);
    string GetMessageId();
    void SetSrcFile(string strSrcFile);
    string GetSrcFile();
    void SetTargetFolder(string strTargetFolder);
    string GetTargetFolder();

    void SetStartTime(unsigned int startTimeMs);
    unsigned int GetStartTime();    
	void SetEndTime(unsigned int startTimeMs);
	unsigned int GetEndTime();    

    virtual bool Encode(string &json);
    virtual TsMessageBase* Decode(const string &json);    

};

class MergeVideoRequest : public TsMessageBase
{
	//{"type":"VideoMerge","id":"FFFFFFFFFFFFFF","file":["\\10.1.200.101\data\somefold\somefile1.mp4","\\10.1.200.101\data\somefold\somefile2.flv", "\\10.1.200.101\data\somefold\somefile3.rmvb"], " targetFile":"\\10.1.200.101\somepath\somefile.trans.mp4" }
    string  m_strMessageId;
    string  m_strSrcFile;
    string  m_strTargetFolder;
	string  m_strFileIndex;
public:
    MergeVideoRequest(){m_TsMessageType = TS_MESSAGE_VIDEO_MERGE;	m_strMessageId = "";m_strSrcFile = "";m_strTargetFolder="";};
    void SetMessageId(string strMessageId){m_strMessageId = strMessageId;};
    string GetMessageId(){return m_strMessageId;};
    void SetSrcFile(string strSrcFile){m_strSrcFile = strSrcFile;};
    string GetSrcFile(){return m_strSrcFile;};
    void SetFileIndex(string strFileIndex){m_strFileIndex = strFileIndex;};
    string GetFileIndex(){return m_strFileIndex;};	
    void SetTargetFolder(string strTargetFolder){m_strTargetFolder = strTargetFolder;};
    string GetTargetFolder(){return m_strTargetFolder;};	
    virtual bool Encode(string &json);
    virtual TsMessageBase* Decode(const string &json);    

};

class GetVideoInfoRequest : public TsMessageBase
{
	//{"type":" Message ","id":"FFFFFFFFFFFFFF","file":"\\10.1.200.101\data\somefold\somefile. mp4"}
    string     m_strMessageId;
    string  m_strSrcFile;
public:
    GetVideoInfoRequest();
    void SetMessageId(string strMessageId);
    string GetMessageId();
    void SetSrcFile(string strSrcFile);
    string GetSrcFile();
    virtual bool Encode(string &json);
    virtual TsMessageBase* Decode(const string &json);    

};

class TsAcceptTask : public TsMessageBase
{
	//{type:"AcceptTask",id:"FFFFFFFFFFFFFF"}
    string m_strMessageId;
	string m_strServerType;
public:
    void SetMessageId(string strMessageId);
    TsAcceptTask(){m_TsMessageType = TS_MESSAGE_ACCEPT_TASK; m_strMessageId = "";};
    virtual bool Encode(string &json);
    virtual TsMessageBase* Decode(const string &json);    

};

class SvrLinkCheck : public TsMessageBase
{
public:
    SvrLinkCheck(){m_TsMessageType = TS_MESSAGE_SVRLINK_CHECK;};
    virtual bool Encode(string &json);
    virtual TsMessageBase* Decode(const string &json);    

};

class LinkResponse : public TsMessageBase
{

	//{type:"LinkResponse",ip:"10.2.1.101",id:"1234",servertype:"DYNAMIC_PPT"}
    string m_strIp;
    string m_strId;    
    TsType m_TsType;
public:
    LinkResponse();
    void SetIp(string strIp);
    void SetId(string strId);
    virtual bool Encode(string &json);
    virtual TsMessageBase* Decode(const string &json);

};

class TsMessageCodec
{
public:
    bool Encode(TsMessageBase &obj, string &json);
    TsMessageBase* Decode(const string &json);
};

class AppMessageCodec
{
public:
    bool Encode(AppMessageBase &obj, string &json);
    AppMessageBase* Decode(const string &json);
};

#endif//_MESSAGE_ASSEMBLE_H

