//#include "appsim.h"
#include <stdio.h>
#include "EventLooper.h"
#include <iostream>
#include <vector>

//#include "TextTestResult.h"
#include "TestRunner.h"
#include "AppTest.h"
#include "InterfaceApi.h"
#include "ConfigManager.h"
#include "Log4cxxWrapper.h"
#include "Message.h"
#include "DsTeleServer.h"
#include "TsManager.h"
#include "SysStatus.h"
#include "DataPersistence.h"
using namespace cdy;



void test()
{
//	ConfigManager::GetAppPort();
//	SessionManager &sm = SessionManager::GetInstance();
	//sm.SendAppMessage("10.1.1.1.1",1111, "djaiogoidja",NULL);
//	AppServerSession* ass = new AppServerSession(NULL);
}



int main (int ac, char **av)
{
//	TestRunner runner;
//	runner.addTest ("AppTest", AppTest::suite ());
//	runner.run (ac, av);

	    //调用libevent创建对应用和转换服务器监听端口
    // 建立logger
	SysStatus &sysStatus = SysStatus::GetInstance();
	//sysStatus.SetDataPresistence(&DataPresistence::GetInstance(&sysStatus));
	sysStatus.SysStatusRestore();

	LoggerWrapper log = LoggerWrapper::GetInstance();    
	if(true != (SessionManager::GetInstance()).StartListenForAppServer(AppMessageAnalyzer::GetInstance()))
	{
		log.Log(true,LOGGER_FATAL,"Create listen for App fail , [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);
		return -1;
	}

	if(true != (SessionManager::GetInstance()).StartListenForTS(TsMessageAnalyzer::GetInstance(),TransServerLeave::GetInstance()))
	{
		log.Log(true,LOGGER_FATAL,"Create listen for ts fail , [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);    
		return -1;
	}



    /* Telnet模块初始化 */
	telnetInit(true,ConfigManager::GetTelenetPort());


    (SessionManager::GetInstance()).NewTimer(30, SysStatus::OnTimer);

    EventLooper::GetInstance().RunEventLoop();


	return 0;
}


