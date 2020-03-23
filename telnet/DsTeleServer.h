/*************************************************************
模块名	： DispServer TELNET
文件名	： DsTelServer.h
相关文件：
文件实现功能：Dispserver telenet server 的主要包含头文件 
******************************************************************************/
#include "system/kdvtype.h"
#include <pthread.h>


#ifndef DS_TELE_INC
#define DS_TELE_INC

#ifdef __cplusplus			/* for C++ */
#ifndef API
#define API  extern "C"	
#endif
#else						/* for C */
#ifndef API
#define API	
#endif
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET		  (-1)
#endif


/* Telnet server's port range */
#define MIN_TELSVR_PORT         2500
#define MAX_TELSVR_PORT         8000 /* 2800; for debug mulitnode */
#define  DS_MAX_LOG_MSG_LEN          6000    //每条日志的最大长度

#define ERRNOGET errno

typedef void* HMODULE;


#define DS_TELEECHO_TASKPRI          70
#define DS_TELEECHO_STACKSIZE        (400<<10)
#ifndef SOCKADDR_IN
#define SOCKADDR_IN 				 struct sockaddr_in
#endif
#ifndef SOCKADDR
#define SOCKADDR					 struct sockaddr
#endif
#ifndef SOCKET_ERROR
#define SOCKET_ERROR		  (-1)
#endif
#define ioctlOnPtrTypeCast s64

typedef struct sockaddr * SOCKADDRPTR;  /* AL - 09/17/98 */

typedef void *(*OSAPILINUXFUNC)(void *);


#define MAX_MODNAME_LEN 255//MAX_PATH
#define MAX_MOD_NUM     20

#define MAX_COMMAND_LENGTH      255
#define MAX_SEND_LENGTH         2048

#define MAX_PROMPT_LENGTH      32


#define TELCMD_WILL    (u8)251
#define TELCMD_WONT    (u8)252
#define TELCMD_DO      (u8)253
#define TELCMD_DONT    (u8)254
#define TELCMD_IAC     (u8)255

#define TELOPT_ECHO     (u8)1
#define TELOPT_SGA      (u8)3
#define TELOPT_LFLOW    (u8)33
#define TELOPT_NAWS     (u8)34

#define NEWLINE_CHAR    '\n'
#define BACKSPACE_CHAR  8
#define BLANK_CHAR      ' '
#define RETURN_CHAR     13
#define TAB_CHAR        9
#define DEL_CHAR        127
#define CTRL_S          19
#define CTRL_R          18
#define UP_ARROW        27
#define DOWN_ARROW      28
#define LEFT_ARROW      29
#define RIGHT_ARROW     30

#define CMD_NAME_SIZE		32
#define CMD_USAGE_SIZE		200
#define CMD_TABLE_SIZE		100

/* 终端提示符长度 */
#define MAX_PROMPT_LEN		21

#define DSTELSOCKHANDLE				 int


/* UniformFunc: definition for all functions those can invoked by user through telnet */
typedef int (*UniformFunc)(s32,s32,s32,s32,s32,s32,s32,s32,s32,s32);


typedef struct {
    char	name[CMD_NAME_SIZE];	/* Command Name; less than 30 bytes */
    UniformFunc cmd;			/* Implementation function */
    char	usage[CMD_USAGE_SIZE];	/* Usage message */
} TCmdTable;


typedef struct 
{
    u16        m_wPort;
    u8         m_achCommand[MAX_COMMAND_LENGTH];
    u32        m_dwCommandLen;
    
    s8         m_achPrompt[MAX_PROMPT_LENGTH];
    
    
    u8         m_achSendBuff[MAX_SEND_LENGTH];
    u32        m_dwSendLen;
    BOOL32     m_bExit;
    
    pthread_t m_hTelSrvTask;
    u32        m_dwThreadId;
    int      m_hTelListen;
    int      m_hTelClient;
    pthread_cond_t *  m_hTelSem;
    
} TTelServer;

API u16 DsTelePrint(s8* pszMsg);
API u16 telSend(s8* pchBuf, u32 dwLen);
API UniformFunc FindCommand(const char* name);
API u16 telnetInit(BOOL32 bTelnetEnable, u16 wTelnetPort);
API u16 telnetEnd();
API BOOL32 DsTeleSockClose(DSTELSOCKHANDLE tSock);
API BOOL32 DsTeleSetPrompt(const s8 *prompt);
API s32 DsTeleTcpListen(DSTELSOCKHANDLE hSock, s32 nQueue);
API s32 DsTelesockBytesAvailable(DSTELSOCKHANDLE hSocket);
API BOOL32 DsTeleSockClose(DSTELSOCKHANDLE hSock);
API DSTELSOCKHANDLE DsTeleTcpOpen(u32 dwIpAddr, u16 wPort);
API void DsPrintf(char *szFormat, ...);
API pthread_t DsTeleTaskCreate(
	void *pvTaskEntry, 
	char *szName, 
	u8 byPriority, 
	u32 dwStacksize, 
	u32 dwParam, 
	u16 wFlag, 
	u32 *pdwTaskID);


#endif /*DS_TELE_INC */

