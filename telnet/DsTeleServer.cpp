/******************************************************************************
ģ����  �� DispServer Telnet
�ļ���  �� DsTelServer.cpp
����ļ���
�ļ�ʵ�ֹ��ܣ�ʵ��Telnet�߳�
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
  ��������DsTelRegCommand
  ���ܣ�ע�������Telnet��ִ�еĺ���
  �㷨ʵ�֣�
  ����ȫ�ֱ�����
  �������˵����
  	szName	- Telnet�����������
	pfFunc	- ��Ӧ�ĺ���ָ��
	szUsage	- ����İ�����Ϣ
  ����ֵ˵������
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
��������telnetInit
���ܣ�telnet��ʼ����
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�������
�������˵����

����ֵ˵������
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
        
        /* ������Telnet�ͻ���ͨ�ŵ����� */
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
��������telnetEnd
���ܣ�����telnet�նˡ�
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�������
�������˵����

����ֵ˵������
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
��������telProc
���ܣ�telnet�߳�ʵ�֡�
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�������
�������˵����

����ֵ˵������
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
��������telSelect
���ܣ�telnet�̼߳����������ݡ�
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�������
�������˵����

����ֵ˵������
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
		/*����дsock�Ƿ����µ�����  */
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
    /*����Ƿ����µ����ӽ���  */
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
��������telClientRecv
���ܣ�telnet��ȡһ�ֽ��������ݲ����Ե���Ļ�ϡ�
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�������
�������˵����

����ֵ˵������
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
    /* �����û�����, ���Ե�Telnet�ͻ�����Ļ��, ������������ʵ�����Ӧ */

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
��������telSend
���ܣ��ϲ�������telnet������sock��������,���һ��û����д�뻺��,�������²�select������telcliensend��������
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�������
�������˵����

����ֵ˵������
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
��������telClientSend
���ܣ��²�select�̵߳�����telnet������sock�������ݡ�
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����

����ֵ˵������
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
��������telClientClose
���ܣ��ر�telnet�ͻ���sock��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����g_tTimerMgr
�������˵����

����ֵ˵������
====================================================================*/
static u16 telClientClose()
{
	DsTeleSockClose(g_tTelSrv.m_hTelClient);
	g_tTelSrv.m_hTelClient = INVALID_SOCKET;
	return 0;
}

/*====================================================================
��������telAccept
���ܣ���������sock�����ӡ�
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����g_tTimerMgr
�������˵����

����ֵ˵������
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
        /* ����TELE���ԣ���ӡ��ӭ���*/
        SendIAC(TELCMD_DO, TELOPT_ECHO);
        SendIAC(TELCMD_DO, TELOPT_NAWS);
        SendIAC(TELCMD_DO, TELOPT_LFLOW);
        SendIAC(TELCMD_WILL, TELOPT_ECHO);
        SendIAC(TELCMD_WILL, TELOPT_SGA);
        
        /* �����ӭ���� */
        DsTelePrint("*===============================================================\n");
        DsTelePrint("                Welcome to the telenet server of dispserver !\n");
        DsTelePrint("*===============================================================\n");
        
        /* �����ʾ�� */
        PromptShow();
    }

    return 0;
}

/*====================================================================
��������remove_iac
���ܣ�����ת��ΪASCII�뺯��
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����
            
����ֵ˵������.
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
��������RunCmd
���ܣ������û�����, �����ʵ�����Ӧ.
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����szCmd: �յ��������ַ���.
            
����ֵ˵������.
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
  ��������RegCommands
  ���ܣ�ע��OSAPI����Ҫ��Telnet��ִ�еĺ���
  �㷨ʵ�֣�����ѡ�
  ����ȫ�ֱ�����
  �������˵����
  ����ֵ˵������
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

    /* �������������� */
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
    		/* ������ַ�Ϊ��Ч�ַ���ǰһ�ַ�ΪNULL����ʾ�ɵ��ʽ�����
    		   �µ��ʿ�ʼ */
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

	/* ��ִ������ */
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


    /* �����û�ע��ĺ���*/
    {
        UniformFunc func = FindCommand(szCmd);
        if (func != NULL)
        {		
            /*��֤ÿ������ִ��ʱ�����Ǵ��µ�һ�п�ʼָӡ�ġ�*/
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
��������CmdParse
���ܣ������׼���ȥ����ͷ����Ч�ַ���������'\0'.
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����pchCmd: ���, 
              uCmdLen: �����.
            
����ֵ˵������.
====================================================================*/
static void CmdParse(s8* pchCmd, u32 dwCmdLen)
{  
    u8 count;
    s32 nCpyLen = 0;
    s8 command[MAX_COMMAND_LENGTH];
    /*ȥͷ*/
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
��������WordParse
���ܣ�ת���������.
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����word: �����������, 
                          
����ֵ˵�����������Ϊ����, ���ظ�����; Ϊ��ͨ�ַ����򷵻ظ��ַ���ָ��.
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
��������GetBaseModName
���ܣ���Windows�µõ�һ��ģ�����ķ�·��������Ϊȱʡ����ʾ��.
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����pchModName: ģ��ȫ��

����ֵ˵����ģ�����Ļ�������.
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
��������PromptShow
���ܣ���Telnet����ʾ��ʾ��.
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵������

����ֵ˵������.
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
��������DsTelePrint
���ܣ���Telnet��Ļ�ϴ�ӡ��NULL��β���ַ���
�㷨ʵ�֣�Ϊ�˷��㴦��"\r","\n"��ת��Ϊ"\r\n"
          ֻ��������ɴ�ӡ�ַ������������ַ���\r\n\t
   ����ȫ�ֱ�����
�������˵����pchMsg: Ҫ��ӡ���ַ���
����ֵ˵�����ɹ�����TRUE, ʧ�ܷ���FALSE.
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
��������DsTeleSetPrompt
���ܣ�����Telnet��ʾ��, Windows������; Linux��ȡģ����Ϊȱʡ��ʾ��.
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����g_Osp
�������˵����prompt: �µ���ʾ��

  ����ֵ˵�����ɹ�����TRUE, ʧ�ܷ���FALSE.
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
��������DsTeleTcpOpen
���ܣ���ָ����ַ�Ͷ˿ڽ�������sock
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵������

����ֵ˵�������ؼ�����socket
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
��������DsTeleTcpListen
���ܣ���ָ��sock�Ͻ��м���
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵������

����ֵ˵����0��ʾ�ɹ�,RET_E_TELNET_LISTEN_FAIL��ʾʧ��
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
��������DsTelesockBytesAvailable
���ܣ�����ָ��sock��ÿ��recvʱ�ܹ���ȡ�����뻺��Ĵ�С
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�������
�������˵����

����ֵ˵�������ڵ���0Ϊʵ���ܶ�ȡ�Ĵ�С��С��0Ϊ��Ӧ��ʧ�ܴ�����
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
��������DsTeleSockClose
���ܣ��ر��׽���
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����tSock--���ر��׽��ֵľ��������socketʱ���أ���

 ����ֵ˵�����ɹ�����TRUE��ʧ�ܷ���FALSE
====================================================================*/
API BOOL32 DsTeleSockClose(DSTELSOCKHANDLE hSock)
{
    if (hSock == INVALID_SOCKET)
    {
        return FALSE;
    }

    /* ��׼�û��رձ�׼����\��� */
    if(hSock < 3)
    {
        return FALSE;
    }
    return ( 0 == close(hSock) );	
    
}


/*====================================================================
��������DsTeleTaskCreate
���ܣ�����������һ������
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����pvTaskEntry: ������ڣ�
              szName: ����������NULL�������ַ�����
              uPriority: �������ȼ���
              uStacksize: �����ջ��С��
              uParam: ���������
			  uFlag: ������־��
			  puTaskID: ���ز���������ID.

  ����ֵ˵�����ɹ���������ľ����ʧ�ܷ���NULL.
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

    // ���õ��Ȳ���
    pthread_attr_getschedpolicy(&tThreadAttr, &nSchPolicy);
    nSchPolicy = SCHED_FIFO;
    pthread_attr_setschedpolicy(&tThreadAttr, nSchPolicy);

    // �������ȼ�
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
