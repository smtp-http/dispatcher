/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * FileConvert
 * Copyright (C) yandaomin 2014 <>
 * 
 * FileConvert is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * FileConvert is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "../include/ConfigManager.h"
#include "../include/SimpleIni.h"
//#include "../include/log.h"

#define DEFAULT_CONFIGFILE "config.ini"
//#define DEFAULT_CONFIGFILE "/usr/src/OpenOffice/FileConvert/Debug/src/config.ini"

bool ConfigManager::LoadConfig(){
	CSimpleIni config;
	
	string strPath = DEFAULT_CONFIGFILE;
	if (config.LoadFile(strPath.c_str()) < 0)
	{
		return false;
	}
	return true;
}

bool ConfigManager::SetAndSaveTsPriority(string strPriority)
{
    CSimpleIni config;
    
    string strPath = DEFAULT_CONFIGFILE;
    if (config.LoadFile(strPath.c_str()) < 0)
    {
        return false;
    }

    if(config.SetValue("config", "TsPriority", strPriority.c_str()) < 0)
    {
        return false;
    }
    
    if(config.SaveFile(strPath.c_str()) < 0)
    { 
        return false;
    }

    return true;

}

string ConfigManager::GetTsPriority()
{
	CSimpleIni config;
	
	string strPath = DEFAULT_CONFIGFILE;
	if (config.LoadFile(strPath.c_str()) < 0)
	{
		return "";
	}

	string rlt = config.GetValue("config", "TsPriority", "[\"Linux\",\"Office\",\"Wps\"]");
  
	return rlt;
}

string ConfigManager::GetIp(){
	CSimpleIni config;
	
	string strPath = DEFAULT_CONFIGFILE;
	if (config.LoadFile(strPath.c_str()) < 0)
	{
		return "";
	}

	string rlt = config.GetValue("config", "IP", "127.0.0.1");
  
	return rlt;
}

int ConfigManager::GetDynamicPptTaskTimedOutMinutes(){
	CSimpleIni config;
	
	string strPath = DEFAULT_CONFIGFILE;
	if (config.LoadFile(strPath.c_str()) < 0)
	{
		return -1;
	}
	long	port = config.GetLongValue("config", "DynamicPptTaskTimedOutMinutes", 20);
	return port;
}

int ConfigManager::GetVideoTaskTimedOutMinutes(){
	CSimpleIni config;
	
	string strPath = DEFAULT_CONFIGFILE;
	if (config.LoadFile(strPath.c_str()) < 0)
	{
		return -1;
	}
	long	port = config.GetLongValue("config", "VideoTaskTimedOutMinutes", 120);
	return port;
}

int ConfigManager::GetDocTaskTimedOutMinutes(){
	CSimpleIni config;
	
	string strPath = DEFAULT_CONFIGFILE;
	if (config.LoadFile(strPath.c_str()) < 0)
	{
		return -1;
	}
	long	port = config.GetLongValue("config", "DocTaskTimedOutMinutes", 20);
	return port;
}


unsigned int ConfigManager::GetAppPort(){
	CSimpleIni config;
	
	string strPath = DEFAULT_CONFIGFILE;
	if (config.LoadFile(strPath.c_str()) < 0)
	{
		return -1;
	}
	unsigned int port = config.GetLongValue("config", "AppPort", 6665);
	return port;
}

int ConfigManager::GetTelenetPort(){
	CSimpleIni config;
	
	string strPath = DEFAULT_CONFIGFILE;
	if (config.LoadFile(strPath.c_str()) < 0)
	{
		return -1;
	}
	long	port = config.GetLongValue("config", "TelenetPort", 5999);
	return port;
}

unsigned int ConfigManager::GetTsPort(){
	CSimpleIni config;
	
	string strPath = DEFAULT_CONFIGFILE;
	if (config.LoadFile(strPath.c_str()) < 0)
	{
		return -1;
	}
	unsigned int port = config.GetLongValue("config", "TsPort", 6666);
	return port;
}

int ConfigManager::GetReconnectTimeoutTimes()
{
	CSimpleIni config;
	
	string strPath = DEFAULT_CONFIGFILE;
	if (config.LoadFile(strPath.c_str()) < 0)
	{
		return -1;
	}
	unsigned int times = config.GetLongValue("config", "ReconnectTimeoutTimes", 10);
	return times;
	
}

