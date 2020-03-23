#ifndef __TS_INTERFACE_H__
#define __TS_INTERFACE_H__

#include "json/json.h"
#include "InterfaceApi.h"



class ITsRegister
{
public:
	virtual void OnRegister(Json::Value& value,TsSession* session) = 0;
};

class ITsUpdateInfo
{
public:
	virtual void OnUpdateInfo(Json::Value& value,TsSession *session) = 0;
};


#endif
