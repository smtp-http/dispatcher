#ifndef __APP_TEST_H__
#define __APP_TEST_H__

#include "TestCase.h"
#include "TestSuite.h"
#include "TestCaller.h"

class TaskWaitQueue;


class AppTest: public TestCase
{
 public:
	AppTest (std::string name) : TestCase (name) {}
	void setUp ();
	static Test *suite ();

 protected:
	void TestAddTask();
	void TestAppMessageAnalyzer();
	void TestCommandDb();
	void TestTaskWaitQueue();
	void TestTaskWaitQueue(TaskWaitQueue *wq,string taskId);
	void TestTaskRunQueue();
	void TestTsMessageAnalyzer();
	void TestDisptacher();
	void TestTsMessageDb();
	void TestTsQueue();
	void TestTransServerDb();
	/*
	void TestMsgRecFromAPP();
	void TestMsgRecFromTs();
	*/

private:
	
	
};


#endif

