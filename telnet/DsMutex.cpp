/*****************************************************************************
ģ����      : DispServer
�ļ���      : mutex.cpp
����ļ�    : 
�ļ�ʵ�ֹ���: �ź�����ʵ��
����        : �޽���
�汾        : 1.0
-----------------------------------------------------------------------------
�޸ļ�¼:
����(D/M/Y)     �汾     �޸���      �޸�����
18/11/2014      1.0     �޽���       ����
******************************************************************************/
#include "DsMutex.h"

#include <iostream>
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <fstream>
#include <errno.h>

unsigned int g_dwDsSemCount = 0;
TDsSemStat	g_atDsSemStat[MAX_SEM_PER_PROCESS] = {0};
unsigned int g_dwDsSemTotalCount = 0;
unsigned int g_dwMaxSemCount = 0;
u64 g_qwSemTakeTotalCycle = 0;
u64 g_qwSemTakeMaxCycle = 0;


/*====================================================================
  ��������SemBCreate
  ���ܣ�����һ����Ԫ�ź���
  �㷨ʵ�֣�����ѡ�
  ����ȫ�ֱ�����
  �������˵����phSema: ���ص��ź������

  ����ֵ˵�����ɹ�����true��ʧ�ܷ���FALSE
  ====================================================================*/
API BOOL32 SemBCreate(SEMHANDLE *phSema)
{
    if(phSema == NULL)
    {
	return FALSE;
    }


        if(phSema == NULL)
        {
            return FALSE;
        }

		TDsCond* ptDsCond = ( TDsCond* )malloc(sizeof(TDsCond));
        if (ptDsCond == NULL)
        {
            return FALSE;
        }
        
        pthread_cond_init(&ptDsCond->m_tCond, NULL);
        pthread_mutex_init(&ptDsCond->m_tMutex, NULL);
        ptDsCond->m_dwCurrentCount = 1;
        ptDsCond->m_dwMaxCount = 1;
        *phSema = (SEMHANDLE)ptDsCond;
        if( g_dwDsSemCount < MAX_SEM_PER_PROCESS )
		{
			memset( &g_atDsSemStat[g_dwDsSemCount] , 0 , sizeof(TDsSemStat) );
			g_atDsSemStat[g_dwDsSemCount].m_tSem = (SEMHANDLE)ptDsCond;
			g_dwDsSemCount++;                  
		}
		g_dwDsSemTotalCount++;
		if( g_dwMaxSemCount < g_dwDsSemTotalCount )
		{
			g_dwMaxSemCount = g_dwDsSemTotalCount;
		}
        return true;
}

/*====================================================================
  ��������SemCCreate
  ���ܣ����������ź���
  �㷨ʵ�֣�����ѡ�
  ����ȫ�ֱ�����
  �������˵����phSema: �ź���������ز����� 
  uInitCount: ��ʼ������
  uMaxCount: ������

  ����ֵ˵�����ɹ�����true��ʧ�ܷ���FALSE.
  ====================================================================*/
API BOOL32 SemCCreate(SEMHANDLE *phSema, unsigned int dwInitCount, unsigned int dwMaxCount)
{
    if(phSema == NULL)
    {
	return FALSE;
    }

       // s32 nRet;
        
        if(phSema == NULL)
        {
            return FALSE;
        }
        
        TDsCond* ptDsCond = ( TDsCond* )malloc(sizeof(TDsCond));
        if (ptDsCond == NULL)
        {
            return FALSE;
        }
        
        pthread_cond_init(&ptDsCond->m_tCond, NULL);
        pthread_mutex_init(&ptDsCond->m_tMutex, NULL);
        ptDsCond->m_dwCurrentCount = dwInitCount;
        ptDsCond->m_dwMaxCount = dwMaxCount;
        *phSema = (SEMHANDLE)ptDsCond;
        if( g_dwDsSemCount < MAX_SEM_PER_PROCESS )
		{
			memset( &g_atDsSemStat[g_dwDsSemCount] , 0 , sizeof(TDsSemStat) );
			g_atDsSemStat[g_dwDsSemCount].m_tSem = (SEMHANDLE)ptDsCond;
			g_dwDsSemCount++;
		}
		g_dwDsSemTotalCount++;
		if( g_dwMaxSemCount < g_dwDsSemTotalCount )
		{
			g_dwMaxSemCount = g_dwDsSemTotalCount;
		}
        return true;

}

/*====================================================================
  ��������SemDelete
  ���ܣ�ɾ���ź���
  �㷨ʵ�֣�����ѡ�
  ����ȫ�ֱ�����
  �������˵����hSema: ��ɾ���ź����ľ��

  ����ֵ˵�����ɹ�����true��ʧ�ܷ���FALSE.
  ====================================================================*/
API BOOL32 SemDelete(SEMHANDLE hSema)
{
    if(hSema == NULL)
    {
	return FALSE;
    }

    if (NULL == hSema)
    {
        return FALSE;
    }
    
    TDsCond* ptDsCond = (TDsCond* )hSema;
    pthread_cond_destroy( &ptDsCond->m_tCond );
    pthread_mutex_destroy( &ptDsCond->m_tMutex );
    free(ptDsCond);
    if( g_dwDsSemCount > 0 )
	{
		for( unsigned int dwIndex = 0 ; dwIndex < g_dwDsSemCount ; dwIndex++ )
		{
			if( g_atDsSemStat[dwIndex].m_tSem == hSema )
			{
				g_atDsSemStat[dwIndex].m_tSem = NULL;
				g_atDsSemStat[dwIndex] = g_atDsSemStat[g_dwDsSemCount-1];
				g_dwDsSemCount--;
				break;
			}
		}
	}
    g_dwDsSemTotalCount--;
    return true;

}

/*====================================================================
  ��������SemTake
  ���ܣ��ź���p����
  �㷨ʵ�֣�����ѡ�
  ����ȫ�ֱ�����
  �������˵����hSema: �ź������

  ����ֵ˵�����ɹ�����true��ʧ�ܷ���FALSE.
  ====================================================================*/
API BOOL32  SemTake(SEMHANDLE hSema)
{
    if(hSema == NULL)
    {
	return FALSE;
    }

    if (NULL == hSema)
    {
        return FALSE;
    }
    TDsCond* ptDsCond = (TDsCond* )hSema;
    int nRet = 0;
    unsigned int dwCycleTimes = 0;
    pthread_mutex_lock( &ptDsCond->m_tMutex );
    while( ptDsCond->m_dwCurrentCount <= 0 )
    {
			nRet = pthread_cond_wait( &ptDsCond->m_tCond, &ptDsCond->m_tMutex );
			if( ptDsCond->m_dwCurrentCount <= 0 )
			{
				dwCycleTimes++;
			}
    }
    g_qwSemTakeTotalCycle += dwCycleTimes;
    if( g_qwSemTakeMaxCycle < dwCycleTimes )
	{
		g_qwSemTakeMaxCycle = dwCycleTimes;
	}
    BOOL32 bFind = FALSE;
    unsigned int dwIndex = 0;
	for( dwIndex = 0 ; dwIndex < g_dwDsSemCount ; dwIndex++ )
	{
		if( g_atDsSemStat[dwIndex].m_tSem == hSema )
		{
			bFind = true;
			break;
		}
	}
    if( 0 == nRet )
    {
    	if( true == bFind )
    	{
    		g_atDsSemStat[dwIndex].m_qwSemTakeSuccess++;
    	}
        if(ptDsCond->m_dwCurrentCount > 0)
        {        
    	    ptDsCond->m_dwCurrentCount--;
        }
    }
    pthread_mutex_unlock( &ptDsCond->m_tMutex );
    return ( 0 == nRet);

}


/*====================================================================
  ��������SemTakeByTime
  ���ܣ�����ʱ���ź���p����
  �㷨ʵ�֣�����ѡ�
  ����ȫ�ֱ�����
  �������˵����hSema: �ź������, 
  uTimeout: ��ʱ���(��msΪ��λ)

  ����ֵ˵�����ɹ�����true��ʧ�ܷ���FALSE.
  ====================================================================*/
API BOOL32 SemTakeByTime(SEMHANDLE hSema, unsigned int dwTimeout)
{
    if(hSema == NULL)
    {
	return FALSE;
    }

    if (NULL == hSema)
    {
        return FALSE;
    }
    TDsCond* ptDsCond = (TDsCond* )hSema;
    int nRet = 0;
    struct timeval now;
    struct timespec timeout;
    gettimeofday(&now, NULL);

    timeout.tv_sec = now.tv_sec + dwTimeout/1000;
    int nTime = dwTimeout%1000;
    if(now.tv_usec+nTime*1000>1000000) 
    {
        timeout.tv_sec++;
        timeout.tv_nsec = (now.tv_usec+nTime*1000-1000000)*1000;
    }
    else
    {
        timeout.tv_nsec = (now.tv_usec + nTime*1000) * 1000;
    }

	unsigned int dwCycleTimes = 0;
	pthread_mutex_lock( &ptDsCond->m_tMutex );
    while( ptDsCond->m_dwCurrentCount <= 0 )
    {
    	nRet = pthread_cond_timedwait( &ptDsCond->m_tCond, &ptDsCond->m_tMutex, &timeout );
    	if( ETIMEDOUT == nRet )
				break;
		if( ptDsCond->m_dwCurrentCount <= 0 )
		{
			dwCycleTimes++;
		}
    }
    g_qwSemTakeTotalCycle += dwCycleTimes;
    if( g_qwSemTakeMaxCycle < dwCycleTimes )
	{
		g_qwSemTakeMaxCycle = dwCycleTimes;
	}
	BOOL32 bFind = FALSE;
    unsigned int dwIndex = 0;
    for( dwIndex = 0 ; dwIndex < g_dwDsSemCount ; dwIndex++ )
	{
		if( g_atDsSemStat[dwIndex].m_tSem == hSema )
		{
			bFind = true;
			break;
		}
	}
    if( 0 == nRet )
    {
    	if( true == bFind )
    	{
    		g_atDsSemStat[dwIndex].m_qwSemTakeByTimeSuccess++;
    	}
        if(ptDsCond->m_dwCurrentCount > 0)
        {        
    	    ptDsCond->m_dwCurrentCount--;
        }
    }
    else if( ETIMEDOUT == nRet )
	{
		if( true == bFind )
        {
    	    g_atDsSemStat[dwIndex].m_qwSemTakeByTimeTimeOut++;
        }
	}
    pthread_mutex_unlock( &ptDsCond->m_tMutex );

    return ( 0 == nRet );

}


/*====================================================================
  ��������SemGive
  ���ܣ��ź���v����
  �㷨ʵ�֣�����ѡ�
  ����ȫ�ֱ�����
  �������˵����hSema: �ź������

  ����ֵ˵�����ɹ�����true��ʧ�ܷ���FALSE.
  ====================================================================*/
API BOOL32 SemGive(SEMHANDLE hSema)
{
    if(hSema == NULL)
    {
	return FALSE;
    }


    if (NULL == hSema)
    {
        return FALSE;
    }
    TDsCond* ptDsCond = (TDsCond* )hSema;
    int nRet = 0;
    pthread_mutex_lock( &ptDsCond->m_tMutex );
    if( ptDsCond->m_dwMaxCount > ptDsCond->m_dwCurrentCount )
    {
        if( 0 == pthread_cond_signal( &ptDsCond->m_tCond ) )
		{
		    nRet = 0;
		}
		else
		{
		    nRet = 1;
		}
    }
    else
    {
		   	nRet = 2;
    }
    BOOL32 bFind = FALSE;
    unsigned int dwIndex = 0;
	for( dwIndex = 0 ; dwIndex < g_dwDsSemCount ; dwIndex++ )
	{
		if( g_atDsSemStat[dwIndex].m_tSem == hSema )
		{
			bFind = true;
			break;
		}
	}
    if( 0 == nRet )
    {
    	if( true == bFind )
    	{
    		g_atDsSemStat[dwIndex].m_qwSemGiveSuccess++;
    	}
    	ptDsCond->m_dwCurrentCount++;
    }
    else if( 2 == nRet )
	{
		if( true == bFind )
        {
    	    g_atDsSemStat[dwIndex].m_qwSemGiveOverflow++;
        }
	}
    pthread_mutex_unlock( &ptDsCond->m_tMutex );
    return ( 0 == nRet );

}


API void SemShow()
{
	unsigned int tickHigh = 0;
	unsigned int tickLow = 0;
	TDsCond* ptDsCond = NULL;
	unsigned int dwIndex = 0;
	
	if(1/* IsDsInitd() */)
	{
		printf( "\ng_dwDsSemCount(%d)\n" , g_dwDsSemCount );
		for( dwIndex = 0; dwIndex < g_dwDsSemCount ; dwIndex++ )
		{
			ptDsCond = (TDsCond*)g_atDsSemStat[dwIndex].m_tSem;
			pthread_mutex_lock( &ptDsCond->m_tMutex );
			if( ( g_atDsSemStat[dwIndex].m_qwSemTakeSuccess+g_atDsSemStat[dwIndex].m_qwSemTakeByTimeSuccess \
				== g_atDsSemStat[dwIndex].m_qwSemGiveSuccess ) &&
				( 0 == g_atDsSemStat[dwIndex].m_qwSemGiveOverflow ) )
			{
				pthread_mutex_unlock( &ptDsCond->m_tMutex );
				continue;
			}
			printf( "dwIndex(%d) " , dwIndex );
//			printf( "m_tSem(0x%x) " , g_atDsSemStat[dwIndex].m_tSem );
			printf( "m_dwCurrentCount(0x%x) " , ptDsCond->m_dwCurrentCount );
			printf( "m_dwMaxCount(0x%x)\n" , ptDsCond->m_dwMaxCount );
			
			tickHigh = (unsigned int)(g_atDsSemStat[dwIndex].m_qwSemTakeSuccess>>32);
			tickLow = (unsigned int)g_atDsSemStat[dwIndex].m_qwSemTakeSuccess;
			printf( "m_qwSemTakeSuccess(0x%x 0x%x) " , tickHigh , tickLow );
			
			printf( "\n");
			
			tickHigh = (unsigned int)(g_atDsSemStat[dwIndex].m_qwSemTakeByTimeSuccess>>32);
			tickLow = (unsigned int)g_atDsSemStat[dwIndex].m_qwSemTakeByTimeSuccess;
			printf( "m_qwSemTakeByTimeSuccess(0x%x 0x%x) " , tickHigh , tickLow );
			
			if( 0 < g_atDsSemStat[dwIndex].m_qwSemTakeByTimeTimeOut )
			{
				tickHigh = (unsigned int)(g_atDsSemStat[dwIndex].m_qwSemTakeByTimeTimeOut>>32);
				tickLow = (unsigned int)g_atDsSemStat[dwIndex].m_qwSemTakeByTimeTimeOut;
				printf( "m_qwSemTakeByTimeTimeOut(0x%x 0x%x) " , tickHigh , tickLow );
			}
			
			printf( "\n" );
			
			tickHigh = (unsigned int)(g_atDsSemStat[dwIndex].m_qwSemGiveSuccess>>32);
			tickLow = (unsigned int)g_atDsSemStat[dwIndex].m_qwSemGiveSuccess;
			printf( "m_qwSemGiveSuccess(0x%x 0x%x) " , tickHigh , tickLow );
			
			if( 0 < g_atDsSemStat[dwIndex].m_qwSemGiveOverflow )
			{
				tickHigh = (unsigned int)(g_atDsSemStat[dwIndex].m_qwSemGiveOverflow>>32);
				tickLow = (unsigned int)g_atDsSemStat[dwIndex].m_qwSemGiveOverflow;
				printf( "m_qwSemGiveOverflow(0x%x 0x%x)" , tickHigh , tickLow );
			}
			
			printf( "\n\n");
			pthread_mutex_unlock( &ptDsCond->m_tMutex );
		}
		tickHigh = (unsigned int)(g_qwSemTakeTotalCycle>>32);
		tickLow = (unsigned int)g_qwSemTakeTotalCycle;
		printf( "g_qwSemTakeTotalCycle(0x%x 0x%x) " , tickHigh , tickLow );
		
		tickHigh = (unsigned int)(g_qwSemTakeMaxCycle>>32);
		tickLow = (unsigned int)g_qwSemTakeMaxCycle;
		printf( "g_qwSemTakeMaxCycle(0x%x 0x%x)\n" , tickHigh , tickLow );
			
		printf( "g_dwDsSemTotalCount(%d) " , g_dwDsSemTotalCount );
		printf( "g_dwMaxSemCount(%d)\n" , g_dwMaxSemCount );
	}
	else
	{
		printf( "\ng_dwDsSemCount(%d)\n" , g_dwDsSemCount );
		for( dwIndex = 0; dwIndex < g_dwDsSemCount ; dwIndex++ )
		{
			ptDsCond = (TDsCond*)g_atDsSemStat[dwIndex].m_tSem;
			pthread_mutex_lock( &ptDsCond->m_tMutex );
			if( ( g_atDsSemStat[dwIndex].m_qwSemTakeSuccess+g_atDsSemStat[dwIndex].m_qwSemTakeByTimeSuccess \
				== g_atDsSemStat[dwIndex].m_qwSemGiveSuccess ) &&
				( 0 == g_atDsSemStat[dwIndex].m_qwSemGiveOverflow ) )
			{
				pthread_mutex_unlock( &ptDsCond->m_tMutex );
				continue;
			}
			printf( "dwIndex(%d) " , dwIndex );
//			printf( "m_tSem(0x%x) " , g_atDsSemStat[dwIndex].m_tSem );
			printf( "m_dwCurrentCount(0x%x) " , ptDsCond->m_dwCurrentCount );
			printf( "m_dwMaxCount(0x%x)\n" , ptDsCond->m_dwMaxCount );
			
			tickHigh = (unsigned int)(g_atDsSemStat[dwIndex].m_qwSemTakeSuccess>>32);
			tickLow = (unsigned int)g_atDsSemStat[dwIndex].m_qwSemTakeSuccess;
			printf( "m_qwSemTakeSuccess(0x%x 0x%x) " , tickHigh , tickLow );
			
			printf( "\n");
			
			tickHigh = (unsigned int)(g_atDsSemStat[dwIndex].m_qwSemTakeByTimeSuccess>>32);
			tickLow = (unsigned int)g_atDsSemStat[dwIndex].m_qwSemTakeByTimeSuccess;
			printf( "m_qwSemTakeByTimeSuccess(0x%x 0x%x) " , tickHigh , tickLow );
			
			if( 0 < g_atDsSemStat[dwIndex].m_qwSemTakeByTimeTimeOut )
			{
				tickHigh = (unsigned int)(g_atDsSemStat[dwIndex].m_qwSemTakeByTimeTimeOut>>32);
				tickLow = (unsigned int)g_atDsSemStat[dwIndex].m_qwSemTakeByTimeTimeOut;
				printf( "m_qwSemTakeByTimeTimeOut(0x%x 0x%x) " , tickHigh , tickLow );
			}
			
			printf( "\n" );
			
			tickHigh = (unsigned int)(g_atDsSemStat[dwIndex].m_qwSemGiveSuccess>>32);
			tickLow = (unsigned int)g_atDsSemStat[dwIndex].m_qwSemGiveSuccess;
			printf( "m_qwSemGiveSuccess(0x%x 0x%x) " , tickHigh , tickLow );
			
			if( 0 < g_atDsSemStat[dwIndex].m_qwSemGiveOverflow )
			{
				tickHigh = (unsigned int)(g_atDsSemStat[dwIndex].m_qwSemGiveOverflow>>32);
				tickLow = (unsigned int)g_atDsSemStat[dwIndex].m_qwSemGiveOverflow;
				printf( "m_qwSemGiveOverflow(0x%x 0x%x)" , tickHigh , tickLow );
			}
			
			printf( "\n\n");
			pthread_mutex_unlock( &ptDsCond->m_tMutex );
		}
		tickHigh = (unsigned int)(g_qwSemTakeTotalCycle>>32);
		tickLow = (unsigned int)g_qwSemTakeTotalCycle;
		printf( "g_qwSemTakeTotalCycle(0x%x 0x%x) " , tickHigh , tickLow );
		
		tickHigh = (unsigned int)(g_qwSemTakeMaxCycle>>32);
		tickLow = (unsigned int)g_qwSemTakeMaxCycle;
		printf( "g_qwSemTakeMaxCycle(0x%x 0x%x)\n" , tickHigh , tickLow );
			
		printf( "g_dwDsSemTotalCount(%d) " , g_dwDsSemTotalCount );
		printf( "g_dwMaxSemCount(%d)\n" , g_dwMaxSemCount );
	}
}

