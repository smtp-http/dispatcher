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

#ifndef _CONFIG_MANAGER_H_
#define _CONFIG_MANAGER_H_

#include <string>

using namespace std;

class ConfigManager
{
public:
	static string GetIp();
	static unsigned int GetAppPort();
	static int GetDocTaskTimedOutMinutes();
	static int GetVideoTaskTimedOutMinutes();
	static int GetDynamicPptTaskTimedOutMinutes();	
	static unsigned int GetTsPort();
	static int GetTelenetPort();
	static bool LoadConfig();
	static bool SetAndSaveTsPriority(string strPriority);
	static string GetTsPriority();
	static int GetReconnectTimeoutTimes();
protected:

private:

};

#endif // _CONFIG_MANAGER_H_
