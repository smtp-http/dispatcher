#ifndef __TASK_COMM_H__
#define __TASK_COMM_H__


typedef enum{
	DocTask = 0,
	VideoTask,
	DpptTask,
	NoTsTask
}TaskType;

typedef enum
{
	TASK_HEAVY = 0,
	TASK_LIGHT = 1
}TaskProperty;

typedef enum{
	SERVER_LINUX = 0,
	SERVER_WPS = 1,
	SERVER_OFFICE = 2,
	
	SERVER_NOAPPOINTED =255
}DocTsType;

typedef enum{
	ERROR_TASKTIMEOUT = 37
	
}ErrorCode;

#endif

