/******************************************************************************
模块名  ： DispServer Telnet
文件名  ： DsTelServer.cpp
相关文件：
文件实现功能：实现Telnet线程
******************************************************************************/

#include "DsTeleServer.h"

#include "Error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <malloc.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/times.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include "DsMutex.h"

#include "TelnetCmdFunction.h"


#define TEL_SEM_TAKE(hSem) {if (pthread_self() != g_tTelSrv.m_dwThreadId) SemTake(hSem);}
#define TEL_SEM_GIVE(hSem) {if (pthread_self() != g_tTelSrv.m_dwThreadId) SemGive(hSem);}



TTelServer g_tTelSrv = {0};

static u16 telClientRecv();         /* telnet recv for select */
static u16 telClientSend();         /* telnet send for select */
static u16 telClientClose();        /* telnet close client */
static u16 telAccept();             /* telnet accept client */

static void* telProc(void* param);  /* telnet procedure */
static u16 telSelect();             /* telnet select */

static void SendIAC(s8 cmd, s8 opt);
static s8 remove_iac(u8 c);

static void RunCmd(s8 *szCmd);
static void CmdParse(s8* pchCmd, u32 dwCmdLen);
static s64 WordParse(s8* word);
static s8 *GetBaseModName(s8 *pchModName);
static void PromptShow(void);
static void RegCommands();

/*
extern void ShowDocLInuxServerState();
extern void ShowDocOfficeRunServerState();
extern void ShowDocOfficeServerState();
extern void ShowDocLInuxRunServerState();
extern void ShowDocWpsRunServerState();
extern void ShowDocWpsServerState();


extern void ShowAllRunServerState();
extern void ShowDynamicPptRunServerState();
extern void ShowDynamicPptServerState();
extern void ShowVideoRunServerState();
extern void ShowVideoReconnectServerState();
extern void ShowVideoServerState();
extern void ShowDocOfficeRunningQueue();
extern void ShowDocWpsRunningQueue();

extern void ShowDocLinuxRunningQueue();
extern void ShowPptRunningQueue();
extern void ShowVideoRunningQueue();
extern void ShowVideoWaitQueue();

extern void ShowAllRunQueue();
extern void ShowDynamicPptFreeServerState();
extern void ShowVideoFreeServerState();
extern void ShowDocLinuxFreeServerState();
extern void ShowDocOfficeFreeServerState();
extern void ShowDocWpsFreeServerState();
extern void ShowAllFreeServerState();

extern void ShowSessionManagerDataStruct();
*/

static TCmdTable g_tCmdTable[CMD_TABLE_SIZE];
static int	 g_iCmdIndex = 0;

/*====================================================================
  函数名：DsTelRegCommand
  功能：注册可以在Telnet下执行的函数
  算法实现：
  引用全局变量：
  输入参数说明：
  	szName	- Telnet下输入的命令
	pfFunc	- 对应的函数指针
	szUsage	- 命令的帮助信息
  返回值说明：无
  ====================================================================*/

API void DsTelRegCommand(const char* name, void* func, const char* usage)
{
    if (g_iCmdIndex < CMD_TABLE_SIZE)
    {
		strncpy(g_tCmdTable[g_iCmdIndex].name, name, CMD_NAME_SIZE);
		strncpy(g_tCmdTable[g_iCmdIndex].usage, usage, CMD_USAGE_SIZE);
		g_tCmdTable[g_iCmdIndex].cmd = (UniformFunc)func;
		g_iCmdIndex++;
    }
}

API UniformFunc FindCommand(const char* name)
{
    s32 i;
    for(i = 0; i < g_iCmdIndex; i++)
    {
	    if (strcmp(g_tCmdTable[i].name, name) == 0)
	        return g_tCmdTable[i].cmd;
    }
    return NULL;
}

API void DsPrintf(char *szFormat, ...)
{
    va_list pvList;
    s8 msg[DS_MAX_LOG_MSG_LEN];
    u32 actLen = 0;
    u16 wRet = 0; 

    if(szFormat == NULL)
    {
        return;
    }


    va_start(pvList, szFormat);	
    actLen = vsprintf(msg, szFormat, pvList); 
    va_end(pvList);
    if (actLen <= 0 || actLen >= DS_MAX_LOG_MSG_LEN)
    {
        return;
    }

    wRet = DsTelePrint(msg);
    if (RET_E_TELNET_NOT_CONNECT == wRet)
    {
        printf("%s", msg);
    }

}


/*====================================================================
函数名：telnetInit
功能：telnet初始化。
算法实现：（可选项）
引用全局变量：无
输入参数说明：

返回值说明：无
====================================================================*/
API u16 telnetInit(BOOL32 bTelnetEnable, u16 wPort)
{
    static BOOL32 m_bPowerOnInit = FALSE;
    u16 nRet = 0;
    if (!m_bPowerOnInit)
    {
        m_bPowerOnInit = TRUE;
        memset(&g_tTelSrv, 0, sizeof(g_tTelSrv));

        g_tTelSrv.m_achCommand[0] = '\0';

        g_tTelSrv.m_hTelClient = INVALID_SOCKET;
        g_tTelSrv.m_hTelListen = INVALID_SOCKET;
        g_tTelSrv.m_hTelSem = NULL;
        g_tTelSrv.m_dwSendLen = 0;
        /*g_tTelSrv.m_pachModTable[0] = "OspDll.dll";*/
        g_tTelSrv.m_dwCommandLen = 0;
    }

#ifndef TEL_DISABLE
    do
    {
        /* if port is 0, use default port */
        if (wPort != 0)
        {
            g_tTelSrv.m_hTelListen = DsTeleTcpOpen(0, wPort);
            if (g_tTelSrv.m_hTelListen != INVALID_SOCKET)
            {
                g_tTelSrv.m_wPort = wPort;
            }
        }
        
        if (g_tTelSrv.m_hTelListen == INVALID_SOCKET)
        {
            for (wPort = MIN_TELSVR_PORT; wPort < MAX_TELSVR_PORT; wPort++)
            {
                g_tTelSrv.m_hTelListen = DsTeleTcpOpen(0, wPort); /* server's port */
                if (g_tTelSrv.m_hTelListen != INVALID_SOCKET) 
                {
                    g_tTelSrv.m_wPort = wPort;
                    break;
                }			
            }
        }
        
        if (g_tTelSrv.m_hTelListen == INVALID_SOCKET)
        {
            nRet = RET_E_TELNET_BIND_FAIL;
            break;
        }
        
        DsTeleTcpListen(g_tTelSrv.m_hTelListen, 1);
        
        if (!SemBCreate(&g_tTelSrv.m_hTelSem))
        {
            nRet = RET_E_TELNET_SEM_FAIL;
            break;
        }
        
        /* 创建与Telnet客户端通信的任务 */
        g_tTelSrv.m_hTelSrvTask = DsTeleTaskCreate((void *)(u32)telProc, "TelenetServer", DS_TELEECHO_TASKPRI,
            DS_TELEECHO_STACKSIZE, 0, 0, &g_tTelSrv.m_dwThreadId);
        
        if (NULL == g_tTelSrv.m_hTelSrvTask)
        {
            nRet = RET_E_TELNET_TASK_FAIL;
            break;
        }
    } while(0);
    


#endif /* TEL_DISABLE */

    DsTeleSetPrompt("DS");
    RegCommands();

    return nRet;
}



/*      Telnet Implement, if define TEL_DISABLE, following code is not include    */
#ifndef TEL_DISABLE

API BOOL32 DsTeleTaskWaitEnd(pthread_t hTask)
{
    void *temp;

    return ( 0 == pthread_join(hTask, &temp) );
}

/*====================================================================
函数名：telnetEnd
功能：结束telnet终端。
算法实现：（可选项）
引用全局变量：无
输入参数说明：

返回值说明：无
====================================================================*/
API u16 telnetEnd()
{
	/* exit thread */
	g_tTelSrv.m_bExit = TRUE;
    if (NULL != g_tTelSrv.m_hTelSrvTask)
    {
	    /*DsTeleTaskTerminate(g_tTelSrv.m_hTelSrvTask);*/
        DsTeleTaskWaitEnd(g_tTelSrv.m_hTelSrvTask);
	    g_tTelSrv.m_hTelSrvTask = NULL;
    }
	if (NULL != g_tTelSrv.m_hTelSem)
    {
	    SemDelete(g_tTelSrv.m_hTelSem);
    }
	if (INVALID_SOCKET != g_tTelSrv.m_hTelListen)
    {
	    DsTeleSockClose(g_tTelSrv.m_hTelListen);
        g_tTelSrv.m_hTelListen = INVALID_SOCKET;
    }
	if (INVALID_SOCKET != g_tTelSrv.m_hTelClient)
    {
	    DsTeleSockClose(g_tTelSrv.m_hTelClient);
        g_tTelSrv.m_hTelClient = INVALID_SOCKET;
    }
	g_tTelSrv.m_achCommand[0] = '\0';
    g_tTelSrv.m_dwSendLen = 0;
    g_tTelSrv.m_dwCommandLen = 0;
	return 0;
}

/*====================================================================
函数名：telProc
功能：telnet线程实现。
算法实现：（可选项）
引用全局变量：无
输入参数说明：

返回值说明：无
====================================================================*/
static void* telProc(void* param)
{

    if (NULL == g_tTelSrv.m_hTelSem)
    {
        return NULL;
    }	

    SemTake(g_tTelSrv.m_hTelSem);

    while (!g_tTelSrv.m_bExit)
    {
        static u32 st_dwCount;
        st_dwCount++;
        if (st_dwCount % 100 == 0)
        {
#ifdef _TELDEBUGPRINT_
            printf("telnet select:\n"); 
#endif
        }
        
        telSelect();
    }

    return NULL;
}


/*====================================================================
函数名：telSelect
功能：telnet线程监听网络数据。
算法实现：（可选项）
引用全局变量：无
输入参数说明：

返回值说明：无
====================================================================*/
static u16 telSelect()
{
    fd_set  tRdSet;
    fd_set  tWrSet;

    struct timeval *pTv;
    struct timeval tv;
    u32 dwWaitMilliSec = 100;
    s32 nReadyNum;
    if (INVALID_SOCKET == g_tTelSrv.m_hTelListen)
    {
	    /*DsTelePrintf(1,0,"control socket is -1,no action return,asf timer\n");*/
        return RET_E_TELNET_SERVER_FAIL;
    }
    FD_ZERO(&tRdSet);
    FD_ZERO(&tWrSet);
    FD_SET(g_tTelSrv.m_hTelListen, &tRdSet);
    /*FD_SET(g_tTelSrv.m_hTelListen, &tWrSet);*/
    if (INVALID_SOCKET != g_tTelSrv.m_hTelClient)
    {
	    FD_SET(g_tTelSrv.m_hTelClient, &tRdSet);
	    if (g_tTelSrv.m_dwSendLen > 0)
        {
	        FD_SET(g_tTelSrv.m_hTelClient, &tWrSet);
        }
    }
    tv.tv_sec = dwWaitMilliSec/1000;
    tv.tv_usec = (dwWaitMilliSec%1000)*1000;
    pTv = &tv;
	
    SemGive(g_tTelSrv.m_hTelSem);
    nReadyNum = select(FD_SETSIZE, &tRdSet, &tWrSet, NULL, pTv);
	
    SemTake(g_tTelSrv.m_hTelSem);
    if (nReadyNum < 0)
    {
	    printf("asflib select error\n");
        return 0;
    }
    if (INVALID_SOCKET != g_tTelSrv.m_hTelClient)
    {
		/*检查读写sock是否有新的数据  */
        if (FD_ISSET(g_tTelSrv.m_hTelClient, &tRdSet))
        {
            s32 nBytes = DsTelesockBytesAvailable(g_tTelSrv.m_hTelClient);
            if (nBytes <= 0)
            {
                telClientClose();
				return 0;
            }
            else
            {
               telClientRecv ();
            }
        }
	if (g_tTelSrv.m_hTelClient!= INVALID_SOCKET)
        {         
            if(FD_ISSET(g_tTelSrv.m_hTelClient, &tWrSet))
            {
                telClientSend();
            }
        }
    }
    /*检查是否有新的连接进入  */
    if (INVALID_SOCKET != g_tTelSrv.m_hTelListen)
    {	
        if (FD_ISSET(g_tTelSrv.m_hTelListen, &tRdSet))
        {
            telAccept();
        }		
    }
    return 0;
}

/*====================================================================
函数名：telClientRecv
功能：telnet读取一字节网络数据并回显到屏幕上。
算法实现：（可选项）
引用全局变量：无
输入参数说明：

返回值说明：无
====================================================================*/
static u16 telClientRecv()
{
    s32 nRet;
    u8 byInputChar;
    nRet = recv(g_tTelSrv.m_hTelClient, (u8*)&byInputChar, 1, 0);
  
    if (nRet <= 0)
    {
        if (ERRNOGET != EWOULDBLOCK)
        {
            telClientClose();
        }
        return 0;
    }

    byInputChar = remove_iac(byInputChar);
    /* 解析用户输入, 回显到Telnet客户端屏幕上, 对于命令给出适当的响应 */

    switch (byInputChar)
    {
    case 13:        /* '\r' -> "\r\n"*/
        telSend("\r\n", 2);
        if (g_tTelSrv.m_dwCommandLen > 0)
        {
            CmdParse((s8*)g_tTelSrv.m_achCommand, g_tTelSrv.m_dwCommandLen);
            g_tTelSrv.m_dwCommandLen = 0;
        }
        if(g_tTelSrv.m_hTelClient != INVALID_SOCKET)
        {
            PromptShow();
        }
        break;
        
    case 10:        /* '\n' : nop*/
        break;

    case 8:         /* Backspace in WinXP*/
    case 127:       /* Backspace in Linux*/
        {
            u8 abyBuff[] = {8, ' ', 8};
            
            if (g_tTelSrv.m_dwCommandLen > 0)
            {
                telSend((s8*)abyBuff, sizeof(abyBuff));
                g_tTelSrv.m_dwCommandLen--;
            }
        } break;

    default:
        telSend((s8*)&byInputChar, 1);
        if (g_tTelSrv.m_dwCommandLen < MAX_COMMAND_LENGTH)
        {				
            g_tTelSrv.m_achCommand[g_tTelSrv.m_dwCommandLen++] = byInputChar;	
        }
        else
        {
            telSend("\r\n", 2);
            CmdParse((s8*)g_tTelSrv.m_achCommand, g_tTelSrv.m_dwCommandLen);	
            g_tTelSrv.m_dwCommandLen = 0;
            if(g_tTelSrv.m_hTelClient != INVALID_SOCKET)
            {
                PromptShow();
            }            
        }
    }
    
    return 0;
}

/*====================================================================
函数名：telSend
功能：上层主动向telnet服务器sock发送数据,如果一次没发完写入缓存,并交由下层select到后由telcliensend继续发。
算法实现：（可选项）
引用全局变量：无
输入参数说明：

返回值说明：无
====================================================================*/
API u16 telSend(s8* pchBuf, u32 dwLen)
{
    s32 nByteSend = 0;
	if (INVALID_SOCKET == g_tTelSrv.m_hTelClient)
    {
        return RET_E_TELNET_NOT_CONNECT;
    }

    if (NULL == pchBuf)
    {
        return RET_E_TELNET_PARAM_ERROR;
    }


    if (g_tTelSrv.m_dwSendLen == 0)
    {
        s32 nRet;
        nRet = send(g_tTelSrv.m_hTelClient, pchBuf, dwLen, 0);
        if (nRet < 0)
        {
            if (ERRNOGET != EWOULDBLOCK)
            {
                telClientClose();
                return RET_E_TELNET_SOCK_ERROR;
            }
        }
        else
        {
            nByteSend = nRet;
        }
    }
    
    if (nByteSend < (s32)dwLen)
    {
        /* modify send buff, need take semphone */
        u16 wRet = 0;
        s32 nRemain;

        TEL_SEM_TAKE(g_tTelSrv.m_hTelSem);

        nRemain = dwLen - nByteSend;
       
        if (g_tTelSrv.m_dwSendLen + nRemain > sizeof(g_tTelSrv.m_achSendBuff))
        {
            /* overflow*/
            wRet = RET_E_TELNET_SEND_OVERFLOW;
        }
        else
        {
            memcpy(g_tTelSrv.m_achSendBuff + g_tTelSrv.m_dwSendLen, pchBuf+nByteSend, nRemain);
            g_tTelSrv.m_dwSendLen += nRemain;
        }

        TEL_SEM_GIVE(g_tTelSrv.m_hTelSem);
        
        return wRet;
    }

    return 0;
}

/*====================================================================
函数名：telClientSend
功能：下层select线程调用向telnet服务器sock发送数据。
算法实现：（可选项）
引用全局变量：
输入参数说明：

返回值说明：无
====================================================================*/
static u16 telClientSend()
{
    s32 nRet;
	if (g_tTelSrv.m_dwSendLen == 0)
    {
        return 0;
    }
    
    nRet = send(g_tTelSrv.m_hTelClient, g_tTelSrv.m_achSendBuff, g_tTelSrv.m_dwSendLen, 0);
    if (nRet < 0)
    {
        if (ERRNOGET != EWOULDBLOCK)
        {
            DsTeleSockClose(g_tTelSrv.m_hTelClient);
            g_tTelSrv.m_hTelClient = INVALID_SOCKET;
        }
        return 0;
    }

    if (nRet < (s32)g_tTelSrv.m_dwSendLen)
    {
        g_tTelSrv.m_dwSendLen -= nRet;
        memmove(g_tTelSrv.m_achSendBuff, g_tTelSrv.m_achSendBuff+nRet, g_tTelSrv.m_dwSendLen);
        return 0;
    }

    g_tTelSrv.m_dwSendLen = 0;
    
    return 0;
}

/*====================================================================
函数名：telClientClose
功能：关闭telnet客户端sock。
算法实现：（可选项）
引用全局变量：g_tTimerMgr
输入参数说明：

返回值说明：无
====================================================================*/
static u16 telClientClose()
{
	DsTeleSockClose(g_tTelSrv.m_hTelClient);
	g_tTelSrv.m_hTelClient = INVALID_SOCKET;
	return 0;
}

/*====================================================================
函数名：telAccept
功能：监听处理sock新连接。
算法实现：（可选项）
引用全局变量：g_tTimerMgr
输入参数说明：

返回值说明：无
====================================================================*/
static u16 telAccept()
{
    struct sockaddr_in tAddr;
    socklen_t nLen = (socklen_t)sizeof(tAddr);
    DSTELSOCKHANDLE hSockNew;
    memset(&tAddr, 0, sizeof(tAddr));

    hSockNew = accept(g_tTelSrv.m_hTelListen, (SOCKADDRPTR)&tAddr, &nLen);
 
    /* use the new client, need close old client */
    if (g_tTelSrv.m_hTelClient != INVALID_SOCKET) 
    {   
        DsTeleSockClose(g_tTelSrv.m_hTelClient);

        g_tTelSrv.m_hTelClient = INVALID_SOCKET;
    }

    g_tTelSrv.m_hTelClient = hSockNew;


    {
        /* 设置TELE属性，打印欢迎语句*/
        SendIAC(TELCMD_DO, TELOPT_ECHO);
        SendIAC(TELCMD_DO, TELOPT_NAWS);
        SendIAC(TELCMD_DO, TELOPT_LFLOW);
        SendIAC(TELCMD_WILL, TELOPT_ECHO);
        SendIAC(TELCMD_WILL, TELOPT_SGA);
        
        /* 输出欢迎画面 */
        DsTelePrint("*===============================================================\n");
        DsTelePrint("                Welcome to the telenet server of dispserver !\n");
        DsTelePrint("*===============================================================\n");
        
        /* 输出提示符 */
        PromptShow();
    }

    return 0;
}

/*====================================================================
函数名：remove_iac
功能：整数转换为ASCII码函数
算法实现：（可选项）
引用全局变量：
输入参数说明：
            
返回值说明：无.
====================================================================*/
/* telnet */
enum tel_state { tel_normal = 0, tel_nego = 1, tel_sub = 2 };

static s32 seen_iac = 0;
static enum tel_state state;
static s32 count_after_sb = 0;

static s8 remove_iac(u8 c)
{
    s8 ret = 0;
    if ((c == 255) && !seen_iac)    /* IAC*/
    {
        seen_iac = 1;
        return ret;
    }
    
    if (seen_iac)
    {
        switch(c)
        {
        case 251:
        case 252:
        case 253:
        case 254:
            if (state != tel_normal) {
                printf(" illegal negotiation.\n"); 
            }
            state = tel_nego;
            break;
        case 250:
            if (state != tel_normal){
                printf(" illegal sub negotiation.\n"); 
            }
            state = tel_sub;
            count_after_sb = 0;
            break;
        case 240:
            if (state != tel_sub) {
                printf(" illegal sub end.\n"); 
            }
            state = tel_normal;
            break;
        default:
            if (!((c > 240) && (c < 250) && (state == tel_normal)))
            {
                printf("illegal command.\n"); 
            }
            state = tel_normal;
        }
        seen_iac = 0;
        return 0;
    }
    
    switch (state)
    {
    case tel_nego:
        state = tel_normal;
        break;
    case tel_sub:
        count_after_sb++; /* set maximize sub negotiation length*/
        if (count_after_sb >= 100) state = tel_normal;
        break;
    default:
        ret = c;
    }
    return ret;
}


/*====================================================================
函数名：RunCmd
功能：解析用户输入, 给出适当的响应.
算法实现：（可选项）
引用全局变量：
输入参数说明：szCmd: 收到的命令字符串.
            
返回值说明：无.
====================================================================*/
typedef struct 
{
	s8 *paraStr;
	BOOL32 bInQuote;
	BOOL32 bIsChar;
}TRawPara;

API void DsTelhelp(void)
{
    s32 i;
    for (i = 0; i < g_iCmdIndex; i++)
    {
        DsPrintf("Command: %s\n", g_tCmdTable[i].name);
        DsPrintf("Usage:\n  %s\n", g_tCmdTable[i].usage);
    }
}

/*====================================================================
  函数名：RegCommands
  功能：注册OSAPI中需要在Telnet下执行的函数
  算法实现：（可选项）
  引用全局变量：
  输入参数说明：
  返回值说明：无
  ====================================================================*/
static void RegCommands()
{
    DsTelRegCommand("help", (void*)DsTelhelp, " - Display this help and exit");
    
    DsTelRegCommand("showlinuxall", (void*)ShowDocLinuxServerState, " - Display doc linux ts state");
    
    DsTelRegCommand("showlinux", (void*)ShowDocLinuxRunServerState, " - Display doc linux runing ts state");

    DsTelRegCommand("showlinuxfree", (void*)ShowDocLinuxFreeServerState, " - Display doc linux free ts state");

    DsTelRegCommand("showwpsall", (void*)ShowDocWpsServerState, " - Display doc wps ts state");
    
    DsTelRegCommand("showwps", (void*)ShowDocWpsRunServerState, " - Display doc wps runing ts state");

    DsTelRegCommand("showwpsfree", (void*)ShowDocWpsFreeServerState, " - Display doc wps free ts state");

    DsTelRegCommand("showofficeall", (void*)ShowDocOfficeServerState, " - Display doc office ts state");
    
    DsTelRegCommand("showoffice", (void*)ShowDocOfficeRunServerState, " - Display doc office runing ts state");

    DsTelRegCommand("showofficefree", (void*)ShowDocOfficeFreeServerState, " - Display doc office free ts state");

    DsTelRegCommand("showtsall", (void*)ShowAllRunServerState, " - Display  all runing ts state");
    
    DsTelRegCommand("showppt", (void*)ShowDynamicPptRunServerState, " - Display dynamic ppt runing ts state");

    DsTelRegCommand("showpptfree", (void*)ShowDynamicPptFreeServerState, " - Display dynamic ppt Free ts state");
    
    DsTelRegCommand("showvideoall", (void*)ShowVideoServerState, " - Display video ts state");

    DsTelRegCommand("showvideo", (void*)ShowVideoRunServerState, " - Display video runing ts state");
	
	DsTelRegCommand("showvideowr", (void*)ShowVideoReconnectServerState, " - Display video runing ts state");

    DsTelRegCommand("showvideofree", (void*)ShowVideoFreeServerState, " - Display video Free ts state");
    
    DsTelRegCommand("showvideoall", (void*)ShowVideoServerState, " - Display video ts state");

    DsTelRegCommand("showtsfree", (void*)ShowAllFreeServerState, " - Display  all free ts state");

    DsTelRegCommand("linuxrun", (void*)ShowDocLinuxRunningQueue, " - Display doc linux running queue state");    
    
    DsTelRegCommand("officerun", (void*)ShowDocOfficeRunningQueue, " - Display doc office running queue state");

    DsTelRegCommand("wpsrun", (void*)ShowDocWpsRunningQueue, " - Display doc wps running queue state");

    DsTelRegCommand("allrun", (void*)ShowAllRunQueue, " - Display all running queue state");    

    DsTelRegCommand("pptrun", (void*)ShowPptRunningQueue, " - Display dynamic ppt running queue state");    

    DsTelRegCommand("videorun", (void*)ShowVideoRunningQueue, " - Display video running queue state"); 
	
     DsTelRegCommand("videowait", (void*)ShowVideoWaitQueue, " - Display video wait queue state"); 
    DsTelRegCommand("sessioncheck", (void*)ShowSessionManagerDataStruct, " - Display session manager data struct");  


}

static void RunCmd(s8 *szCmd)
{  
    s32 i;
    s8 *cmd = szCmd;
    s64 para[10];	
    TRawPara atRawPara[10];
    s32 paraNum = 0;
    u8 count = 0;
    u8 chStartCnt = 0;
    s8 output[MAX_COMMAND_LENGTH];
    BOOL32 bStrStart = FALSE;
    BOOL32 bCharStart = FALSE;
    u32 cmdLen = strlen(szCmd)+1;
    memset(para, 0, sizeof(para));
    memset(atRawPara, 0, sizeof(TRawPara)*10);

    /* 解析参数、命令 */
    while( count < cmdLen )
    {	
    	switch(szCmd[count])
    	{
    	case '\'':
    		szCmd[count] = '\0';
    		if(!bCharStart)
    		{
    			chStartCnt = count;
    		}
    		else
    		{
    			if(count > chStartCnt+2)
    			{
    				DsTelePrint("input error.\n");
    				return;
    			}
    		}
    		bCharStart = !bCharStart;
    		break;

    	case '\"':
    		szCmd[count] = '\0';
    		bStrStart = !bStrStart;
    			break;

    	case ',':
    	case ' ':
    	case '\t':
    	case '\n':
    	case '(':
    	case ')':
            if( ! bStrStart )
    		{
    			szCmd[count] = '\0';
    		}
    		break;

    	default:
    		/* 如果本字符为有效字符，前一字符为NULL，表示旧单词结束，
    		   新单词开始 */
    		if(count > 0 && szCmd[count-1] == '\0')
    		{				
    			atRawPara[paraNum].paraStr = &szCmd[count];
    			if(bStrStart)
    			{
    				atRawPara[paraNum].bInQuote = TRUE;
    			}
    			if(bCharStart)
    			{
    				atRawPara[paraNum].bIsChar = TRUE;
    			}
    			if(++paraNum >= 10)
    				break;
    		}
    	}
    	count++;
    }

        if(bStrStart || bCharStart)
        {
        	DsTelePrint("input error.\n");
        	return;
        }

	for (count=0; count<10; count++)
	{
		if(atRawPara[count].paraStr == NULL)
		{
			para[count] = 0;
			continue;
		}

		if(atRawPara[count].bInQuote)
		{
			para[count] = (s64)(atRawPara[count].paraStr);
			continue;
		}

		if(atRawPara[count].bIsChar)
		{
                    para[count] = (s8)atRawPara[count].paraStr[0];
                    continue;
		}

		para[count] = WordParse(atRawPara[count].paraStr);
	}

	/* 先执行命令 */
    if ( strcmp("bye", cmd) == 0 )
    {
        DsTelePrint("\n  bye......\n");
        DsTeleSockClose(g_tTelSrv.m_hTelClient);
        g_tTelSrv.m_hTelClient = INVALID_SOCKET;        
	return;
    } 
    
#if 0 /* test DsTelePrint*/
    if ( strcmp("test", cmd) == 0)
    {
        DsTelePrint("");
        DsTelePrint("1");
        DsTelePrint("12");
        DsTelePrint("234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890");
        DsTelePrint("1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890");
        DsTelePrint("12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901");
        DsTelePrint("123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901");
        DsTelePrint("\r\n");
        DsTelePrint("\r\r");
        DsTelePrint("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    }
#endif

    if (strcmp("help", cmd) == 0)
    {
		DsTelhelp();
		return;
    }


    /* 查找用户注册的函数*/
    {
        UniformFunc func = FindCommand(szCmd);
        if (func != NULL)
        {		
            /*保证每个命令执行时，都是从新的一行开始指印的。*/
            s32 nRet;
            printf("\n"); 
            nRet = (*func)(para[0],para[1],para[2],para[3],para[4],para[5],para[6],para[7],para[8],para[9]);
            /*DsTelePrintf(TRUE, FALSE, "Return value: %d\n", ret);*/
            sprintf(output, "\nvalue = %d\n", nRet);
            DsTelePrint(output);
        } 
		else  
		{
            /*DsTelePrintf(TRUE, FALSE, "Function '%s' doesn't exist!\n", szCmd);*/
            sprintf(output, "Function '%s' doesn't exist!\n", szCmd);
            DsTelePrint(output);
        }
    }
}

/*====================================================================
函数名：CmdParse
功能：构造标准命令，去掉开头的无效字符，最后加上'\0'.
算法实现：（可选项）
引用全局变量：
输入参数说明：pchCmd: 命令串, 
              uCmdLen: 命令长度.
            
返回值说明：无.
====================================================================*/
static void CmdParse(s8* pchCmd, u32 dwCmdLen)
{  
    u8 count;
    s32 nCpyLen = 0;
    s8 command[MAX_COMMAND_LENGTH];
    /*去头*/
    for(count = 0; count < dwCmdLen; count++)
    {
        s8 chTmp;

        chTmp = pchCmd[count];

        if (isdigit(chTmp) || islower(chTmp) || isupper(chTmp))
        {
            break;
        }
    }

    nCpyLen = dwCmdLen-count;
    if(nCpyLen <= 0)
    {
        return;
    }

    memcpy(command, pchCmd+count, nCpyLen);   
    if (dwCmdLen < MAX_COMMAND_LENGTH)
    {
        command[nCpyLen] = '\0';
    }
    else
    {
        command[MAX_COMMAND_LENGTH-1] = '\0';
    }

    RunCmd(command);
}

/*====================================================================
函数名：WordParse
功能：转换命令参数.
算法实现：（可选项）
引用全局变量：
输入参数说明：word: 单个命令参数, 
                          
返回值说明：如果参数为整数, 返回该整数; 为普通字符串则返回该字符串指针.
====================================================================*/
static s64 WordParse(s8* word)
{
    s32 tmp;
    if(word == NULL) return 0;

    tmp = atoi(word);
    if(tmp == 0 && word[0] != '0')
    {
//    	return (s32)word;
        return (s64)word;
    }
    return tmp;
}


/*====================================================================
函数名：GetBaseModName
功能：在Windows下得到一个模块命的非路径部分作为缺省的提示符.
算法实现：（可选项）
引用全局变量：
输入参数说明：pchModName: 模块全名

返回值说明：模块名的基本部分.
====================================================================*/
static s8 *GetBaseModName(s8 *pchModName)
{
    s8 *sep1 = "\\";
    s8 *sep2 = ".";
    s8 *pchBaseModName=NULL;
    s8 *token=NULL;
    token = strtok(pchModName, sep1);
    while( token != NULL )
    {
        pchBaseModName = token;
        token = strtok(NULL, sep1);	  
    }   

    pchBaseModName = strtok( pchBaseModName, sep2);
    return pchBaseModName;
}

/*====================================================================
函数名：PromptShow
功能：在Telnet上显示提示符.
算法实现：（可选项）
引用全局变量：
输入参数说明：无

返回值说明：无.
====================================================================*/
static void PromptShow(void)
{
    s8 achModName[MAX_MODNAME_LEN];
    s8 *pchBaseModName = NULL;
    s8 prompt[MAX_PROMPT_LEN];
    s32 nRet;
    /* if user set prompt, use it */
    if(strlen(g_tTelSrv.m_achPrompt) > 0)
    {
            sprintf(prompt, "%s->", g_tTelSrv.m_achPrompt);
            DsTelePrint(prompt);
            return;
    }

    /* get default prompt */
    if (NULL != getcwd(achModName, sizeof(achModName)))
    {
        pchBaseModName = basename(achModName);
    }

    if(pchBaseModName != NULL)
    {
        sprintf(prompt, "%s->", pchBaseModName);
    }
    else
    {
        sprintf(prompt, "#");
    }

    DsTelePrint(prompt);
}

/*====================================================================
函数名：DsTelePrint
功能：在Telnet屏幕上打印以NULL结尾的字符串
算法实现：为了方便处理，"\r","\n"都转换为"\r\n"
          只允许输出可打印字符和三个控制字符：\r\n\t
   引用全局变量：
输入参数说明：pchMsg: 要打印的字符串
返回值说明：成功返回TRUE, 失败返回FALSE.
====================================================================*/
API u16 DsTelePrint(s8* pszMsg)
{
    BOOL32 bSendOK = FALSE;
    u32 dwStart = 0;
    u32 dwCount = 0;
    s16 OutputCount;
    s8 OutputBuff[MAX_COMMAND_LENGTH] = {0};
    
    if (pszMsg == NULL || g_tTelSrv.m_hTelClient == INVALID_SOCKET)
    {
        return RET_E_TELNET_NOT_CONNECT;
    }

    dwStart = 0;
    dwCount = strlen(pszMsg); 
    OutputCount = 0;
    
    while(dwStart < dwCount)
    {
        s8 c = pszMsg[dwStart];
        /* convert "\r","\n" to "\r\n" */
        if (( c == '\n' ) || (c == '\r'))
        {
            OutputBuff[OutputCount++] = '\r';
            OutputBuff[OutputCount++] = '\n';
        } 
        else 
        { 
            OutputBuff[OutputCount++] = c;
        }

        if (OutputCount >= (MAX_COMMAND_LENGTH - 1))
        {
            u16 wRet;
            wRet = telSend(OutputBuff, OutputCount);
            if (wRet != 0) return FALSE;
            OutputCount = 0;
        }
        dwStart++;
    }
    
    return telSend(OutputBuff, OutputCount);
}

static void SendIAC(s8 cmd, s8 opt)
{
    u8 buf[5];
    buf[0] = TELCMD_IAC;
    buf[1] = cmd;
    buf[2] = opt;
    telSend((s8*)buf, 3);
}

/*====================================================================
函数名：DsTeleSetPrompt
功能：设置Telnet提示符, Windows下适用; Linux下取模块名为缺省提示符.
算法实现：（可选项）
引用全局变量：g_Osp
输入参数说明：prompt: 新的提示符

  返回值说明：成功返回TRUE, 失败返回FALSE.
====================================================================*/
API BOOL32 DsTeleSetPrompt(const s8 *prompt)
{
	if(prompt == NULL || strlen(prompt) > MAX_PROMPT_LEN)
	{
		return FALSE;
	}

	if( NULL == strcpy(g_tTelSrv.m_achPrompt, prompt) )
	{
		return FALSE;
	}

	return TRUE;
}


/*====================================================================
函数名：DsTeleTcpOpen
功能：在指定地址和端口建立监听sock
算法实现：（可选项）
引用全局变量：
输入参数说明：无

返回值说明：返回监听的socket
====================================================================*/
API DSTELSOCKHANDLE DsTeleTcpOpen(u32 dwIpAddr, u16 wPort)
{

    DSTELSOCKHANDLE tSock = INVALID_SOCKET;
    SOCKADDR_IN tSvrINAddr;
    
    memset( &tSvrINAddr, 0, sizeof(tSvrINAddr) );
    
    tSvrINAddr.sin_family = AF_INET; 
    tSvrINAddr.sin_port = htons(wPort);
    tSvrINAddr.sin_addr.s_addr = dwIpAddr;
    
    tSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (tSock == INVALID_SOCKET)
    {
        printf("\nOsp: Tcp server can't create socket!\n");
        return INVALID_SOCKET;
    }
    
    /* set socket option */
    {
        struct linger Linger;
        s32 yes = TRUE;
        u32 on = TRUE;

        Linger.l_onoff = TRUE; 
        Linger.l_linger = 0;

        setsockopt(tSock, SOL_SOCKET, SO_LINGER, (s8*)&Linger, sizeof(Linger));
        setsockopt(tSock, IPPROTO_TCP, TCP_NODELAY, (s8*)&yes, sizeof(yes));
        setsockopt(tSock, SOL_SOCKET, SO_KEEPALIVE, (s8*)&yes, sizeof(yes));

        if (ioctl(tSock, FIONBIO, (ioctlOnPtrTypeCast)&on) < 0)
        {
            printf("ioctl FIONBIO error\n");
            DsTeleSockClose(tSock);
            return INVALID_SOCKET;
        }
        
        setsockopt(tSock, SOL_SOCKET, SO_REUSEADDR, (s8*)0, 0);
    }

    
    if (bind(tSock, (SOCKADDR *)&tSvrINAddr, sizeof(tSvrINAddr)) == SOCKET_ERROR)
    {
/*        printf("Osp: PassiveTcp: bind error!\n");*/
        DsTeleSockClose(tSock);
        return INVALID_SOCKET;
	}

    return tSock;
}

/*====================================================================
函数名：DsTeleTcpListen
功能：在指定sock上进行监听
算法实现：（可选项）
引用全局变量：
输入参数说明：无

返回值说明：0表示成功,RET_E_TELNET_LISTEN_FAIL表示失败
====================================================================*/
API s32 DsTeleTcpListen(DSTELSOCKHANDLE hSock, s32 nQueue)
{
    if ((listen(hSock, nQueue)) < 0)
    {
        perror ("LISTEN");
        return RET_E_TELNET_LISTEN_FAIL;
    }
    
    return 0;
}


/*====================================================================
函数名：DsTelesockBytesAvailable
功能：返回指定sock的每次recv时能够读取的输入缓存的大小
算法实现：（可选项）
引用全局变量：无
输入参数说明：

返回值说明：大于等于0为实际能读取的大小，小于0为相应的失败错误码
====================================================================*/
API s32 DsTelesockBytesAvailable(DSTELSOCKHANDLE hSocket)
{
    s32 nByte;
    s32 nRet = TRUE;
	nRet = ioctl(hSocket, FIONREAD, &nByte);    
    if (nRet < 0)
    {
        return nRet;
    }
    return nByte;
}


/*====================================================================
函数名：DsTeleSockClose
功能：关闭套接字
算法实现：（可选项）
引用全局变量：
输入参数说明：tSock--待关闭套接字的句柄（调用socket时返回）。

 返回值说明：成功返回TRUE，失败返回FALSE
====================================================================*/
API BOOL32 DsTeleSockClose(DSTELSOCKHANDLE hSock)
{
    if (hSock == INVALID_SOCKET)
    {
        return FALSE;
    }

    /* 不准用户关闭标准输入\输出 */
    if(hSock < 3)
    {
        return FALSE;
    }
    return ( 0 == close(hSock) );	
    
}


/*====================================================================
函数名：DsTeleTaskCreate
功能：创建并激活一个任务
算法实现：（可选项）
引用全局变量：
输入参数说明：pvTaskEntry: 任务入口，
              szName: 任务名，以NULL结束的字符串，
              uPriority: 任务优先级别，
              uStacksize: 任务堆栈大小，
              uParam: 任务参数，
			  uFlag: 创建标志，
			  puTaskID: 返回参数，任务ID.

  返回值说明：成功返回任务的句柄，失败返回NULL.
====================================================================*/
API pthread_t DsTeleTaskCreate(
	void *pvTaskEntry, 
	char *szName, 
	u8 byPriority, 
	u32 dwStacksize, 
	u32 dwParam, 
	u16 wFlag, 
	u32 *pdwTaskID)
{
    u32 dwTaskID;
    pthread_t  hTask;

    s32 nRet = 0;
    struct sched_param tSchParam;	
    pthread_attr_t tThreadAttr;
    s32 nSchPolicy;
    pthread_attr_init(&tThreadAttr);

    // 设置调度策略
    pthread_attr_getschedpolicy(&tThreadAttr, &nSchPolicy);
    nSchPolicy = SCHED_FIFO;
    pthread_attr_setschedpolicy(&tThreadAttr, nSchPolicy);

    // 设置优先级
    pthread_attr_getschedparam(&tThreadAttr, &tSchParam);
    byPriority = 255-byPriority;
    if(byPriority < 60)
    {
	byPriority = 60;
    }
    tSchParam.sched_priority = byPriority;
    pthread_attr_setschedparam(&tThreadAttr, &tSchParam);

    pthread_attr_setstacksize(&tThreadAttr, dwStacksize);

    nRet = pthread_create(&hTask, &tThreadAttr, (OSAPILINUXFUNC)((u32)pvTaskEntry), (void *)dwParam);
	
    if(nRet == 0)
    {
   if(pdwTaskID != NULL)
	{
        *pdwTaskID = (u32)hTask;
	}
    return hTask;
    }

    return 0;
}


#endif /* TEL_DISABLE */
