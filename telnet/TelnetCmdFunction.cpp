#include "TelnetCmdFunction.h"
#include <stdio.h>
#include "TsQueue.h"
#include "TaskQueue.h"
#include "DsTeleServer.h"


extern  TsQueue GVideoTsQueue;
extern  TsQueue GDocOpenofficeTsQueue;
extern  TsQueue GDocWpsTsQueue;
extern  TsQueue GDocOfficeTsQueue;
extern  TsQueue GDpptTsQueue;


extern  TaskWaitQueue GDocTaskWaitQueue;
extern  TaskWaitQueue GVideoTaskWaitQueue;
extern  TaskWaitQueue GDpptTaskWaitQueue;
extern  TaskWaitQueue GNoTsTaskTaskWaitQueue;


extern  TaskRunQueue GDocTaskRunQueue;
extern  TaskRunQueue GVideoTaskRunQueue;
extern  TaskRunQueue GDpptTaskRunQueue;
extern  TaskRunQueue GNoTsTaskRunQueue;





void ShowServerState(TsQueue * ts_queue,TsState ts_state= TsAllState)
{
    DsPrintf("TSID  State  MessageID      TSIP       Session\n");
    string strTaskId = "NULL";
    list<TransServer *> *Server_lst = ts_queue->GetTsLst();
    
    list<TransServer *>::iterator itor;
    for(itor = Server_lst->begin(); itor != Server_lst->end(); itor++)
    {
        TransServer * transserver = *itor;
        Task * taskinserver = transserver->GetTask();
        if((TsAllState == ts_state)||(ts_state == transserver->GetState()))
        {
            if(NULL != taskinserver)
            {
                strTaskId = taskinserver->GetTaskId();
            }
            DsPrintf("%s\t%d\t%s\t%s\t%x\n",(transserver->GetId()).c_str(),transserver->GetState(),
                strTaskId.c_str(),(transserver->GetIp()).c_str(),transserver->GetSession());
        }
    }
    return;
}


void ShowDocLinuxServerState()
{
    DsPrintf("\n\nServerType :DOC\n");
    ShowServerState(&GDocOpenofficeTsQueue);
    return;
}

void ShowDocOfficeServerState()
{
    DsPrintf("\n\nServerType :Doc_Office\n");
    ShowServerState(&GDocOfficeTsQueue);
    return;
}

void ShowDocWpsServerState()
{
    DsPrintf("\n\nServerType :Doc_Wps\n");
    ShowServerState(&GDocWpsTsQueue);
    return;
}

void ShowVideoServerState()
{
    DsPrintf("\n\nServerType :Video \n");
    ShowServerState(&GVideoTsQueue);
    return;
}

void ShowDynamicPptServerState()
{
    DsPrintf("\n\nServerType :Dynamic PPT\n");
    ShowServerState(&GDpptTsQueue);
    return;
}




void ShowDocLinuxRunServerState()
{
    DsPrintf("\n\nServerType :DOC\n");
    ShowServerState(&GDocOpenofficeTsQueue,TsRunning);
    return;
}


void ShowDocOfficeRunServerState()
{
    DsPrintf("\n\nServerType :Doc_Office\n");
    ShowServerState(&GDocOfficeTsQueue,TsRunning);
    return;
}


void ShowDocWpsRunServerState()
{
    DsPrintf("\n\nServerType :Doc_Wps\n");
    ShowServerState(&GDocWpsTsQueue,TsRunning);
    return;
}

void ShowVideoRunServerState()
{
    DsPrintf("\n\nServerType :Video \n");
    ShowServerState(&GVideoTsQueue,TsRunning);
    return;
}

void ShowDynamicPptRunServerState()
{
    DsPrintf("\n\nServerType :Dynamic PPT\n");
    ShowServerState(&GDpptTsQueue,TsRunning);
    return;
}

void ShowAllRunServerState()
{
    ShowDocLinuxRunServerState();
    ShowDocOfficeRunServerState();
    ShowDocWpsRunServerState();
    ShowDynamicPptRunServerState();
    ShowVideoRunServerState();
    return;
}

void ShowDynamicPptFreeServerState()
{
    DsPrintf("\n\nServerType :Dynamic PPT\n");
    ShowServerState(&GDpptTsQueue,TsIdle);
    return;
}
void ShowVideoFreeServerState()
{
    DsPrintf("\n\nServerType :Video \n");
    ShowServerState(&GVideoTsQueue,TsIdle);
    return;
}
void ShowDocLinuxFreeServerState()
{
    DsPrintf("\n\nServerType :DOC\n");
    ShowServerState(&GDocOpenofficeTsQueue,TsIdle);
    return;
}
void ShowDocOfficeFreeServerState()
{
    DsPrintf("\n\nServerType :Doc_Office\n");
    ShowServerState(&GDocOfficeTsQueue,TsIdle);
    return;
}
void ShowDocWpsFreeServerState()
{
    DsPrintf("\n\nServerType :Doc_Wps\n");
    ShowServerState(&GDocWpsTsQueue,TsIdle);
    return;
}
void ShowAllFreeServerState()
{
    ShowDocLinuxFreeServerState();
    ShowDocOfficeFreeServerState();
    ShowDocWpsFreeServerState();
    ShowDynamicPptFreeServerState();
    ShowVideoFreeServerState();
}



void ShowVideoReconnectServerState()
{
}



void showDocRunningQueue(DocTsType show_doc_Ts_type)
{
    list<Task *>::const_iterator task_list_iter;
    Task* pTask;
    string strFullPathFileName="";
    string strTargetFullPathFileName="";
    string strFilePrefix="";
    string strMessageId="";	
    string strIpaddr="";
    string strTsId="";
    string transtype="";
    unsigned int    dwPort=0;
    unsigned int    dwMessagePriority=0;
    int TriedNum;

    string AppointedServerType;

    list<Task *> task_lst(GDocTaskRunQueue.GetTaskList());

    
    DsPrintf("MessageId   MessageAddr   Prefix      Ip          Port  TsId    Priority  AppointedServerType  Repeated  TransType   FullPathFileName            TargetFullPathFileName:\n");
    for (task_list_iter= task_lst.begin(); task_list_iter!=task_lst.end(); task_list_iter++)
    {
        pTask = *task_list_iter;
        DocTsType docTstype = ((DocTransServer*)(pTask->GetTransServer()))->GetDocTsType();
        if((NULL!=pTask)&&(show_doc_Ts_type == docTstype))
        {
            Json::Value cmdvalue = pTask->GetCmdValue();

            //parse FullPathFileName
            if(!cmdvalue["file"].isNull() && cmdvalue["file"].isString())
            {
                strFullPathFileName= cmdvalue["file"].asString();
        	}

            //parse TargetFullPathFileName
        	if(!cmdvalue["targetFolder"].isNull() && cmdvalue["targetFolder"].isString())
            {
                strTargetFullPathFileName= cmdvalue["targetFolder"].asString();
        	}

            //parse FilePrefix
            if(!cmdvalue["prefix"].isNull() && cmdvalue["prefix"].isString())
            {
                strFilePrefix= cmdvalue["prefix"].asString();
        	}
            
            //parse MessageId
            strMessageId = pTask->GetTaskId();
            
            //parse Ipaddr
            if(!cmdvalue["host"].isNull() && cmdvalue["host"].isString())
            {
                strIpaddr= cmdvalue["host"].asString();
        	}
            
            //parse Port
            if(!cmdvalue["port"].isNull() && cmdvalue["port"].isIntegral())
            {
                dwPort= cmdvalue["port"].asInt();
        	}
            //parse TsNo
            //parse TsId
            strTsId = (pTask->GetTransServer())->GetId();
            //parse MessagePriority
            if(!cmdvalue["priority"].isNull() && cmdvalue["priority"].isIntegral())
            {
                dwMessagePriority= cmdvalue["priority"].asInt();             
            }
            //parse unMessagePara
            TriedNum = pTask->m_triedTimes;

            
            //parse servertype
            if(!cmdvalue["docServerType"].isNull()) 
            {
                if(cmdvalue["docServerType"].isString())
                {
                    AppointedServerType = cmdvalue["docServerType"].asString();
                }
            }
            else
            {
                AppointedServerType = "Auto_Server";
            }

            if(!cmdvalue["type"].isNull() && cmdvalue["type"].isString())
            {
                
                transtype = cmdvalue["type"].asString();
            }
    
        }

        DsPrintf("%s        0x%x    %s  %s   %d   %s      %d            %s            %d         %s      %s   %s\n",
            strMessageId.c_str(),pTask,strFilePrefix.c_str(),strIpaddr.c_str(),dwPort,strTsId.c_str(),dwMessagePriority,
            AppointedServerType.c_str(),TriedNum,transtype.c_str(),strFullPathFileName.c_str(),strTargetFullPathFileName.c_str());

    }
}

void ShowDocLinuxRunningQueue()
{
    DsPrintf("\n\nDoc Running Queue State:\n");
    showDocRunningQueue(SERVER_LINUX);

}

void ShowDocOfficeRunningQueue()
{
    DsPrintf("\n\nDoc_Office Running Queue State:\n");
    showDocRunningQueue(SERVER_OFFICE);
}
void ShowDocWpsRunningQueue()
{
    DsPrintf("\n\nDoc_Wps Running Queue State:\n");
    showDocRunningQueue(SERVER_WPS);

}



void ShowPptRunningQueue()
{
    int i = 0;
    list<Task *>::const_iterator task_list_iter;
    Task* pTask;
    string strFullPathFileName="";
    string strTargetFullPathFileName="";
    string strFilePrefix="";
    string strMessageId="";	
    string strIpaddr="";
    string strTsId="";
    string transtype="";
    unsigned int    dwPort=0;
    unsigned int    dwMessagePriority=0;
    int TriedNum;

    string AppointedServerType;

    list<Task *> task_lst(GDpptTaskRunQueue.GetTaskList());
    
    DsPrintf("\n\nDynamic PPT Running Queue State:\n");
    DsPrintf("No  MessageAddr   MessageId     Prefix    Ip        Port  TsNo TsId  Priority  Repeated  TransType   FullPathFileName     TargetFullPathFileName:\n");
    for (task_list_iter= task_lst.begin(); task_list_iter!=task_lst.end(); task_list_iter++)
    {
        pTask = *task_list_iter;
        if(NULL!=pTask)
        {
            Json::Value cmdvalue = pTask->GetCmdValue();

            //parse FullPathFileName
            if(!cmdvalue["FullPathFileName"].isNull() && cmdvalue["FullPathFileName"].isString())
            {
                strFullPathFileName= cmdvalue["FullPathFileName"].asString();
        	}

            //parse TargetFullPathFileName
        	if(!cmdvalue["TargetFullPathFileName"].isNull() && cmdvalue["TargetFullPathFileName"].isString())
            {
                strTargetFullPathFileName= cmdvalue["TargetFullPathFileName"].asString();
        	}

            //parse FilePrefix
            if(!cmdvalue["FilePrefix"].isNull() && cmdvalue["FilePrefix"].isString())
            {
                strFilePrefix= cmdvalue["FilePrefix"].asString();
        	}
            
            //parse MessageId
            strMessageId = pTask->GetTaskId();
            
            //parse Ipaddr
            if(!cmdvalue["Ipaddr"].isNull() && cmdvalue["Ipaddr"].isString())
            {
                strIpaddr= cmdvalue["Ipaddr"].asString();
        	}
            
            //parse Port
            if(!cmdvalue["Port"].isNull() && cmdvalue["Port"].isIntegral())
            {
                dwPort= cmdvalue["Port"].asInt();
        	}
            //parse TsNo
            //parse TsId
            strTsId = (pTask->GetTransServer())->GetId();
            //parse MessagePriority
            if(!cmdvalue["priority"].isNull() && cmdvalue["priority"].isIntegral())
            {
                dwMessagePriority= cmdvalue["priority"].asInt();             
            }
            //parse unMessagePara
            TriedNum = pTask->m_triedTimes;

            
            //parse servertype
            if(!cmdvalue["docServerType"].isNull()) 
            {
                if(cmdvalue["docServerType"].isString())
                {
                    AppointedServerType = cmdvalue["docServerType"].asString();
                }
            }
            else
            {
                AppointedServerType = "Auto_Server";
            }

            if(!cmdvalue["type"].isNull() && cmdvalue["type"].isIntegral())
            {
                
                transtype = cmdvalue["type"].asString();
            }
    
        }

        
        DsPrintf("%d   0X%x    %s   %s  %s   %d    %s  %d        %d       %d      %s   %s\n",i,pTask,strMessageId.c_str(),
            strFilePrefix.c_str(),strIpaddr.c_str(),dwPort,strTsId.c_str(),dwMessagePriority,TriedNum,transtype,
            strFullPathFileName.c_str(),strTargetFullPathFileName.c_str());
        
        i++;
    }


}
void ShowVideoRunningQueue()
{

    list<Task *>::const_iterator task_list_iter;
    Task* pTask;
    
    int i=0;
    string strFullPathFileName="";
    string strTargetFullPathFileName="";
    string strFilePrefix="";
    string strMessageId=""; 
    string strIpaddr="";
    string strTsId="";
    
    unsigned int    dwPort=0;
    unsigned int    dwMessagePriority=0;
    unsigned int    TriedNum;
    unsigned int    VideoFormat=0;
    unsigned int    VideoBitrate=0;
    unsigned int    VideoResolutionWidth=0;
    unsigned int    VideoResolutionHeight=0;

    list<Task *> task_lst(GVideoTaskRunQueue.GetTaskList());

    
    DsPrintf("\n\n Video Running Queue State:\n");
    DsPrintf("No  MessageAddr   MessageId     Prefix    Ip        Port  TsId  Priority  Repeated  Format Bitrate ResolutionWidth    ResolutionHeight   FullPathFileName     TargetFullPathFileName:\n");
    for (task_list_iter= task_lst.begin(); task_list_iter!=task_lst.end(); task_list_iter++)
    {
        pTask = *task_list_iter;
        if(NULL!=pTask)
        {
            Json::Value cmdvalue = pTask->GetCmdValue();

            //parse FullPathFileName
            if(!cmdvalue["FullPathFileName"].isNull() && cmdvalue["FullPathFileName"].isString())
            {
                strFullPathFileName= cmdvalue["FullPathFileName"].asString();
        	}

            //parse TargetFullPathFileName
        	if(!cmdvalue["TargetFullPathFileName"].isNull() && cmdvalue["TargetFullPathFileName"].isString())
            {
                strTargetFullPathFileName= cmdvalue["TargetFullPathFileName"].asString();
        	}

            //parse FilePrefix
            if(!cmdvalue["FilePrefix"].isNull() && cmdvalue["FilePrefix"].isString())
            {
                strFilePrefix= cmdvalue["FilePrefix"].asString();
        	}
            
            //parse MessageId
            strMessageId = pTask->GetTaskId();
            
            //parse Ipaddr
            if(!cmdvalue["Ipaddr"].isNull() && cmdvalue["Ipaddr"].isString())
            {
                strIpaddr= cmdvalue["Ipaddr"].asString();
        	}
            
            //parse Port
            if(!cmdvalue["Port"].isNull() && cmdvalue["Port"].isIntegral())
            {
                dwPort= cmdvalue["Port"].asInt();
        	}
            //parse TsNo
            //parse TsId
            strTsId = (pTask->GetTransServer())->GetId();
            //parse MessagePriority
            if(!cmdvalue["priority"].isNull() && cmdvalue["priority"].isIntegral())
            {
                dwMessagePriority= cmdvalue["priority"].asInt();             
            }

            TriedNum = pTask->m_triedTimes;

            //parse video param
            if(!cmdvalue["Format"].isNull() && cmdvalue["Format"].isIntegral())
            {
                VideoFormat= cmdvalue["Format"].asInt();             
            }

            if(!cmdvalue["Bitrate"].isNull() && cmdvalue["Bitrate"].isIntegral())
            {
                VideoBitrate= cmdvalue["Bitrate"].asInt();             
            }

            if(!cmdvalue["ResolutionWidth"].isNull() && cmdvalue["ResolutionWidth"].isIntegral())
            {
                VideoResolutionWidth= cmdvalue["ResolutionWidth"].asInt();             
            }

            if(!cmdvalue["ResolutionHeight"].isNull() && cmdvalue["ResolutionHeight"].isIntegral())
            {
                VideoResolutionHeight= cmdvalue["ResolutionHeight"].asInt();             
            }
            
        }

        DsPrintf("%d   %x    %s   %s  %s   %d    %s  %d        %d       %d      %d      %d      %d      %s   %s\n",
            i,pTask,strMessageId.c_str(),strFilePrefix.c_str(),strIpaddr.c_str(),dwPort,strTsId.c_str(),dwMessagePriority,TriedNum,
            VideoFormat,VideoBitrate,VideoResolutionWidth,VideoResolutionHeight,strFullPathFileName.c_str(),strTargetFullPathFileName.c_str());

        i++;
    }


}
void ShowVideoWaitQueue()
{/*

    TaskPriorityQueue::const_iterator task_list_iter;
    TaskPriorityQueue pTask;
    
    int i=0;
    string strFullPathFileName="";
    string strTargetFullPathFileName="";
    string strFilePrefix="";
    string strMessageId=""; 
    string strIpaddr="";
    string strTsId="";
    
    unsigned int    dwPort=0;
    unsigned int    dwMessagePriority=0;
    unsigned int    TriedNum;
    unsigned int    VideoFormat=0;
    unsigned int    VideoBitrate=0;
    unsigned int    VideoResolutionWidth=0;
    unsigned int    VideoResolutionHeight=0;

    TaskPriorityQueue task_lst(GVideoTaskWaitQueue.GetTaskList);
    
    DsPrintf("\n\n Video Running Queue State:\n");
    DsPrintf("No  MessageAddr   MessageId     Prefix    Ip        Port  TsId  Priority  Repeated  Format Bitrate ResolutionWidth    ResolutionHeight   FullPathFileName     TargetFullPathFileName:\n");
    for (task_list_iter= task_lst.begin(); task_list_iter!=task_lst.end(); task_list_iter++)
    {
        pTask = *task_list_iter;
        if(NULL!=pTask)
        {
            Json::Value cmdvalue = pTask->GetCmdValue();

            //parse FullPathFileName
            if(!cmdvalue["FullPathFileName"].isNull() && cmdvalue["FullPathFileName"].isString())
            {
                strFullPathFileName= cmdvalue["FullPathFileName"].asString();
            }

            //parse TargetFullPathFileName
            if(!cmdvalue["TargetFullPathFileName"].isNull() && cmdvalue["TargetFullPathFileName"].isString())
            {
                strTargetFullPathFileName= cmdvalue["TargetFullPathFileName"].asString();
            }

            //parse FilePrefix
            if(!cmdvalue["FilePrefix"].isNull() && cmdvalue["FilePrefix"].isString())
            {
                strFilePrefix= cmdvalue["FilePrefix"].asString();
            }
            
            //parse MessageId
            strMessageId = pTask->GetTaskId();
            
            //parse Ipaddr
            if(!cmdvalue["Ipaddr"].isNull() && cmdvalue["Ipaddr"].isString())
            {
                strIpaddr= cmdvalue["Ipaddr"].asString();
            }
            
            //parse Port
            if(!cmdvalue["Port"].isNull() && cmdvalue["Port"].isIntegral())
            {
                dwPort= cmdvalue["Port"].asInt();
            }
            //parse TsNo
            //parse TsId
            strTsId = (pTask->GetTransServer())->GetId();
            //parse MessagePriority
            if(!cmdvalue["priority"].isNull() && cmdvalue["priority"].isIntegral())
            {
                dwMessagePriority= cmdvalue["priority"].asInt();             
            }

            TriedNum = pTask->m_triedTimes;

            //parse video param
            if(!cmdvalue["Format"].isNull() && cmdvalue["Format"].isIntegral())
            {
                VideoFormat= cmdvalue["Format"].asInt();             
            }

            if(!cmdvalue["Bitrate"].isNull() && cmdvalue["Bitrate"].isIntegral())
            {
                VideoBitrate= cmdvalue["Bitrate"].asInt();             
            }

            if(!cmdvalue["ResolutionWidth"].isNull() && cmdvalue["ResolutionWidth"].isIntegral())
            {
                VideoResolutionWidth= cmdvalue["ResolutionWidth"].asInt();             
            }

            if(!cmdvalue["ResolutionHeight"].isNull() && cmdvalue["ResolutionHeight"].isIntegral())
            {
                VideoResolutionHeight= cmdvalue["ResolutionHeight"].asInt();             
            }
            
        }

        DsPrintf("%d   %x    %s   %s  %s   %d    %s  %d        %d       %d      %d      %d      %d      %s   %s\n",
            i,pTask,strMessageId.c_str(),strFilePrefix.c_str(),strIpaddr.c_str(),dwPort,strTsId.c_str(),dwMessagePriority,TriedNum,
            VideoFormat,VideoBitrate,VideoResolutionWidth,VideoResolutionHeight,strFullPathFileName.c_str(),strTargetFullPathFileName.c_str());

        i++;
    }
*/

}


void ShowAllRunQueue()
{
}


void ShowSessionManagerDataStruct()
{
}

