#ifndef __CDYEVENTLOOPER_H__
#define __CDYEVENTLOOPER_H__

#ifdef _WIN32
#include <Winsock2.h>
#else
#include <sys/time.h>
#endif

namespace cdy{

typedef long TimerID;

enum TimerFlag{
	TF_FIRE_ONCE,
	TF_FIRE_PERIODICALLY
};

//Callback interface of timer.
class ITimerUserSink
{
public:
	virtual void OnTimer(TimerID tid) = 0;
};

class EventLooper
{
protected:
	EventLooper();
public:
	static EventLooper& GetInstance();
	
	virtual void RunEventLoop();
	
	// stop the event loop after timeAfter milliseconds.
	virtual void StopEventLoop(long timeAfter);
	
	virtual TimerID ScheduleTimer(struct timeval *tv, TimerFlag flag, ITimerUserSink *sink);
	
	virtual void CancelTimer(TimerID tid);
};

};

#endif //__CDYEVENTLOOPER_H__
