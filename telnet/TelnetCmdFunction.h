#ifndef _TELNETCMD_H_
#define _TELNETCMD_H_


//void ShowServerState(TsQueue & ts_queue,TsState ts_state = TsAllState);


//server state
void ShowDocLinuxServerState();
void ShowDocOfficeServerState();
void ShowDocWpsServerState();
void ShowDynamicPptServerState();
void ShowVideoServerState();
void ShowVideoReconnectServerState();


//runing server state
void ShowDocLinuxRunServerState();
void ShowDocWpsRunServerState();
void ShowDocOfficeRunServerState();
void ShowDynamicPptRunServerState();
void ShowVideoRunServerState();
void ShowAllRunServerState();


//running queue
void ShowDocOfficeRunningQueue();
void ShowDocWpsRunningQueue();
void ShowDocLinuxRunningQueue();
void ShowPptRunningQueue();
void ShowVideoRunningQueue();
void ShowAllRunQueue();


//wait queue
void ShowVideoWaitQueue();


//free server state
void ShowDynamicPptFreeServerState();
void ShowVideoFreeServerState();
void ShowDocLinuxFreeServerState();
void ShowDocOfficeFreeServerState();
void ShowDocWpsFreeServerState();
void ShowAllFreeServerState();


//other
void ShowSessionManagerDataStruct();

#endif

