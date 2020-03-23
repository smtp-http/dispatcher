#ifndef __APP_RESULT_SUBMIT_H__
#define __APP_RESULT_SUBMIT_H__

#include "InterfaceApi.h"
#include "Log4cxxWrapper.h"

class AppResultSubmit : public IAppResultSubmit
{
	AppResultSubmit* next;
public:
	AppResultSubmit();
	virtual void OnResultSubmit(int result,void *pTaskMessage)
	{
		if(pTaskMessage == NULL)
			return;
		
		LoggerWrapper log= LoggerWrapper::GetInstance();
		if(next != NULL)
			next->OnResultSubmit(result,pTaskMessage);
		else {   // this command does not exist;
			//printf("error");
			log.Log(true,LOGGER_ERROR,"AppResultSubmit next null! [%s][%s][%d]\n",__FILE__,__PRETTY_FUNCTION__,__LINE__);  
		}
	}
	void setNext(AppResultSubmit *n)
	{
		next = n;
	}
	void add(AppResultSubmit *n)
	{
		if (next)
			next->add(n);
		else
			next = n;
	}
	
};

class DocAppResultSubmit : public AppResultSubmit
{
	DocAppResultSubmit();
public:
	static DocAppResultSubmit& GetInstance();
	virtual void OnResultSubmit(int result,void *pTaskMessage);
};


class DpptAppResultSubmit : public AppResultSubmit
{
	DpptAppResultSubmit();
public:
	static DpptAppResultSubmit& GetInstance();
	virtual void OnResultSubmit(int result,void *pTaskMessage);
};


class VideoAppResultSubmit : public AppResultSubmit
{
	VideoAppResultSubmit();
public:
	static VideoAppResultSubmit& GetInstance();
	virtual void OnResultSubmit(int result,void *pTaskMessage);
};

class NoUseTsAppResultSubmit : public AppResultSubmit
{
	NoUseTsAppResultSubmit();
public:
	static NoUseTsAppResultSubmit& GetInstance();
	virtual void OnResultSubmit(int result,void *pTaskMessage);
};




#endif

