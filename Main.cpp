/*****************************************************************************
ģ����      : DispServer
�ļ���      : main.cpp
����ļ�    : 
�ļ�ʵ�ֹ���: ��������ʵ��
����        : �޽���
�汾        : 1.0
-----------------------------------------------------------------------------
�޸ļ�¼:
����(D/M/Y)     �汾     �޸���      �޸�����
17/11/2014      1.0     �޽���       ����
******************************************************************************/

    
#include <iostream>

#include <stdio.h>
    
#include <pthread.h>
#include <unistd.h>  
#include <ctype.h>

#include <QueueManager.h>
#include <TsNode.h>
#include <DispTask.h>
#include <ConfigManager.h>
#include <InterfaceApi.h>
#include <fstream>

//#include <log4cxx/logger.h>
//#include <log4cxx/logstring.h>
//#include <log4cxx/propertyconfigurator.h>
#include "Log4cxxWrapper.h"

#include "EventLooper.h"
#include "DsTeleServer.h"
#include <cctype> 
#include <string>  
#include <algorithm>  

using namespace std; 
using namespace cdy;
//using namespace log4cxx;
//LoggerPtr g_Dslogger;

DsManage g_cDsManage;
//u32 g_dwAppPort = 6665;
//u32 g_dwTsPort = 6666;

extern void DsTestAll();
extern bool GetTsPriorityFromConfig();
extern bool GetTaskTimedOutMinutesFromConfig();

int main(int argc, const char* argv[])
{
    pthread_t hDispTaskId;
    pthread_t hLinkTaskId;
    

	printf(" # Name      : CODYY Dispatch Server\n");
	printf(" # Version   : CODYY-Dispserver %s\n", VER_DISPSERVER);
	printf(" # Copyright : 2015 - 2018 CODYY, All Rights Reserved\n\n");

    // ��ȡ�����ļ�
    PropertyConfigurator::configure("log4cxx.cfg");

    // ����logger
//    g_Dslogger = Logger::getLogger("Main.cpp:main");
    LoggerWrapper dsLog= LoggerWrapper::GetInstance();    

    //��ʼ����Ϣ�ַ�������
    g_cDsManage.Initialize();

    if (!ConfigManager::LoadConfig())
    {
        dsLog.Log(TRUE,LOGGER_FATAL,"load config failed , [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);       
        return -1;
    }

    g_cDsManage.SetAppListenPort((unsigned int)ConfigManager::GetAppPort());
    g_cDsManage.SetTsListenPort((unsigned int)ConfigManager::GetTsPort());
    g_cDsManage.SetAppListenIp(ConfigManager::GetIp());
    g_cDsManage.SetTsListenIp(ConfigManager::GetIp());

    if(TRUE != GetTaskTimedOutMinutesFromConfig())
    {
        dsLog.Log(TRUE,LOGGER_FATAL,"Get timedout minutes config fail , [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
        return -1;
    }
    
    if(TRUE != GetTsPriorityFromConfig())
    {
        dsLog.Log(TRUE,LOGGER_FATAL,"Get priority config fail , [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
        return -1;
    }
    
    //����libevent������Ӧ�ú�ת�������������˿�

    if(TRUE != (SessionManager::GetInstance()).StartListenForAppServer(g_cDsManage))
    {
        dsLog.Log(TRUE,LOGGER_FATAL,"Create listen for App fail , [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
        return -1;
    }

    if(TRUE != (SessionManager::GetInstance()).StartListenForTS(g_cDsManage))
    {
        dsLog.Log(TRUE,LOGGER_FATAL,"Create listen for ts fail , [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);    
        return -1;
    }

//    DsTestAll();

#ifndef TEL_DISABLE
    /* Telnetģ���ʼ�� */
    telnetInit(TRUE,ConfigManager::GetTelenetPort());
#endif

    (SessionManager::GetInstance()).NewTimer( 10, DsManage::OnTimer );

    EventLooper::GetInstance().RunEventLoop();

    while(FALSE ==g_cDsManage.IsQuitingDs())
    {


    }

	(SessionManager::GetInstance()).KillTimer();

    return 0;
}

