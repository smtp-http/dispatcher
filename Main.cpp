/*****************************************************************************
模块名      : DispServer
文件名      : main.cpp
相关文件    : 
文件实现功能: 主程序类实现
作者        : 罗健锋
版本        : 1.0
-----------------------------------------------------------------------------
修改记录:
日期(D/M/Y)     版本     修改人      修改内容
17/11/2014      1.0     罗健锋       创建
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

    // 读取配置文件
    PropertyConfigurator::configure("log4cxx.cfg");

    // 建立logger
//    g_Dslogger = Logger::getLogger("Main.cpp:main");
    LoggerWrapper dsLog= LoggerWrapper::GetInstance();    

    //初始化消息分发管理类
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
    
    //调用libevent创建对应用和转换服务器监听端口

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
    /* Telnet模块初始化 */
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

