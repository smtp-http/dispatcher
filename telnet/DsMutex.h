#ifndef _MUTEX_H
#define _MUTEX_H

#include <string>
//#include <sockLib.h>
#include "../../../10-common/include/system/kdvtype.h"

#define SEMHANDLE			 pthread_cond_t*

#define API extern "C"

#define MAX_SEM_PER_PROCESS 1024

typedef struct
{
	pthread_cond_t m_tCond;
	pthread_mutex_t m_tMutex;
	int m_dwCurrentCount;
	unsigned int m_dwMaxCount;
}TDsCond;

typedef struct
{
	SEMHANDLE m_tSem;
	u64 m_qwSemTakeSuccess;
	u64 m_qwSemTakeByTimeSuccess;
	u64 m_qwSemTakeByTimeTimeOut;
	u64 m_qwSemGiveSuccess;
	u64 m_qwSemGiveOverflow;
}TDsSemStat;

API BOOL32 SemBCreate(SEMHANDLE *ptSemHandle);

API BOOL32 SemCCreate(SEMHANDLE *ptSemHandle,unsigned int initcount,unsigned int MaxSemCount);

API  BOOL32 SemDelete(SEMHANDLE tSemHandle );

API BOOL32	SemTake(SEMHANDLE tSemHandle );

API BOOL32 SemTakeByTime(SEMHANDLE tSemHandle, unsigned int dwtime);

API BOOL32 SemGive(SEMHANDLE tSemHandle );



#endif//_MUTEX_H

