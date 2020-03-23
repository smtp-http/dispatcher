#include "AppTest.h"
#include "AddTask.h"
#include "Message.h"
#include "CommandDb.h"
#include "TaskQueue.h"


//extern CommandDb GCommandDb;
extern TaskWaitQueue GDocTaskWaitQueue;
extern TaskWaitQueue GVideoTaskWaitQueue;
extern TaskWaitQueue GDpptTaskWaitQueue;
extern TaskWaitQueue GNoTsTaskTaskWaitQueue;


void AppTest::TestAddTask()
{
	cerr << "TestAddTask" << endl;
	//string command = "video_trans_task";
	AddDocTask &root = AddDocTask::GetInstance();
	AddVideoTask &two = AddVideoTask::GetInstance();
  	AddDpptTask &thr = AddDpptTask::GetInstance();
	root.add(&two);
	root.add(&thr);
	//thr.setNext(&root);

	string frame = "{\"command\":\"doc_trans_task\"}";
	
	Json::Value value;
	std::auto_ptr<Json::Reader> pJsonParser(new Json::Reader(Json::Features::strictMode())); 

	if(pJsonParser->parse(frame,value)){
		root.AddSpacificTask(value,NULL);
	}
cout << "************ ---------------------1 *************" << endl;
	frame =  "{\"command\":\"video_trans_task\"}";
	if(pJsonParser->parse(frame,value)){
		root.AddSpacificTask(value,NULL);
	}

	cout << "************ ---------------------2 *************" << endl;
	
	frame =  "{\"command\":\"dynamic_ppt_trans_task\"}";
	if(pJsonParser->parse(frame,value)){
		
		root.AddSpacificTask(value,NULL);
	}
cout << "************ ---------------------3 *************" << endl;
	frame =  "{\"command\":\"dhaoigd\"}";
	if(pJsonParser->parse(frame,value)){
		root.AddSpacificTask(value,NULL);
	}
	cout << "************ ---------------------4 *************" << endl;
	cout << "************ ok *************" << endl;
}

void AppTest::TestAppMessageAnalyzer()
{
	cerr << "TestAppMessageAnalyzer" << endl;
	AppMessageAnalyzer& analyzer = AppMessageAnalyzer::GetInstance();
	
	string frame = "{\"command\":\"doc_trans_task\"}";

	analyzer.OnNewFrame(NULL,frame);

	frame =  "{\"command\":\"video_trans_task\"}";
	analyzer.OnNewFrame(NULL,frame);

	frame =  "{\"command\":\"dynamic_ppt_trans_task\"}";
	analyzer.OnNewFrame(NULL,frame);

	frame =  "{\"command\":\"dhaoigd\"}";
	analyzer.OnNewFrame(NULL,frame);

	cout << "************ 1 *************" << endl;
	
	frame =  "{\"command\":\"server_status_task\"}";
	analyzer.OnNewFrame(NULL,frame);

	cout << "************ 2 *************" << endl;
}


void AppTest::TestCommandDb()
{
	vector<string>::iterator it;
	
	vector<string> v1;
	CommandDb::GetInstance().GetCommandNameInSpecificTaskType(VideoTask,v1);
	
	for (it = v1.begin(); it != v1.end(); ++ it){
		cout << *it << endl;
	}

	
	vector<string>  v2;
	CommandDb::GetInstance().GetCommandNameInSpecificTaskType(DocTask,v2);
	
	for (it = v2.begin(); it != v2.end(); ++ it){
		cout << *it << endl;
	}

	vector<string>  v3;
	CommandDb::GetInstance().GetCommandNameInSpecificTaskType(DpptTask,v3);
	
	for (it = v3.begin(); it != v3.end(); ++ it){
		cout << *it << endl;
	}

	vector<string>  v4;
	CommandDb::GetInstance().GetCommandNameInSpecificTaskType(NoTsTask,v4);
	
	for (it = v4.begin(); it != v4.end(); ++ it){
		cout << *it << endl;
	}

}


void AppTest::TestTaskWaitQueue(TaskWaitQueue* wq,string taskId)
{
	assert(wq->GetTaskInSpecId(taskId) != NULL);
	assert(wq->CheckSameTaskId(taskId) == true);
}


void AppTest::TestTaskWaitQueue()
{
	TaskWaitQueue queue;
	bool ret;

	TaskDocument t1("aaaaa",10);
	t1.SetPriority(10);
	TaskDocument t2("bbbbb",20);
	TaskDocument t3("ccccc",2);
	TaskDocument t4("ddddd",7);

	queue.PushTask(&t1);
	queue.PushTask(&t2);
	queue.PushTask(&t3);
	queue.PushTask(&t4);
	
	assert(queue.CheckSameTaskId("111") == false);
	assert(queue.CheckSameTaskId("aaaaa") == true);
	assert(queue.CheckSameTaskId("aaaaaa") == false);
	assert(queue.CheckSameTaskId("bbbbb") == true);
	assert(queue.CheckSameTaskId("ccccc") == true);
	assert(queue.CheckSameTaskId("ddddd") == true);
	assert(queue.CheckSameTaskId("mmmm") == false);



	Task* t;
	t = queue.GetTaskInSpecId("aaaaa");
	assert(t != NULL);
	assert(t->GetPriority() == 10);
	
	t = queue.GetTaskInSpecId("bbbbb");
	assert(t != NULL);
	assert(t->GetPriority() == 20);


	t = queue.GetTaskInSpecId("ccccc");
	assert(t != NULL);
	assert(t->GetPriority() == 2);


	t = queue.GetTaskInSpecId("ddddd");
	assert(t != NULL);
	assert(t->GetPriority() == 7);
	
	t = queue.GetTaskInSpecId("eeeee");
	assert(t == NULL);
	
	t = queue.GetTaskInSpecId("aaaaaaa");
	assert(t == NULL);

	t = queue.GetTaskInSpecId("mmmmmm");
	assert(t == NULL);

	t = queue.PopTask();
	assert(t != NULL);
	assert(t->GetPriority() == 20);

	t = queue.PopTask();
	assert(t != NULL);
	assert(t->GetPriority() == 10);

	t = queue.PopTask();
	assert(t != NULL);
	assert(t->GetPriority() == 7);

	t = queue.PopTask();
	assert(t != NULL);
	assert(t->GetPriority() == 2);

	t = queue.PopTask();
	assert(t == NULL);

}

void AppTest::TestTaskRunQueue()
{
	string id;

	TaskRunQueue queueVideo(VideoTask);

	Task* task1 = TaskVideoDb::GetInstance().GetTask();
	assert(task1);
	task1->SetTaskId("1234561");
	task1->SetProgress(1);
	
	Task* task2 = TaskVideoDb::GetInstance().GetTask();
	task2->SetTaskId("1234562");
	task2->SetProgress(2);
	
	Task* task3 = TaskVideoDb::GetInstance().GetTask();
	task3->SetTaskId("1234563");
	task3->SetProgress(3);
	
	Task* task4 = TaskVideoDb::GetInstance().GetTask();
	task4->SetTaskId("1234564");
	task4->SetProgress(4);

	
	queueVideo.PushTask(task1,NULL);
	queueVideo.PushTask(task2,NULL);
	queueVideo.PushTask(task3,NULL);
	queueVideo.PushTask(task4,NULL);

	assert(queueVideo.GetTaskNum() == 4);

	id = "1234563";
	assert(queueVideo.CheckSameTaskId(id) == true);
	assert((queueVideo.GetTaskInSpecId(id))->GetProgress() == 3);

	id = "1234569";
	assert(queueVideo.CheckSameTaskId(id) == false);

	Task* tk = queueVideo.GetTaskInSpecId(id);
	assert(tk == NULL);
	

	id = "1234562";
	tk = queueVideo.GetTaskInSpecId(id);
	assert(tk != NULL);
	assert(tk->GetProgress() == 2);

	assert(queueVideo.DelTask(id) == true);
	

	assert(queueVideo.GetTaskNum() == 3) ;

	id = "883y8";
	assert(queueVideo.DelTask(id) == false);

	assert(queueVideo.GetTaskNum() == 3) ;
	
}

void AppTest::TestTsMessageAnalyzer()
{
	cerr << "TestAppMessageAnalyzer" << endl;
	TsMessageAnalyzer& analyzer = TsMessageAnalyzer::GetInstance();

	string frame = "{\"type\":\"TransResult\",\"result\":\"Success/Fail\",\"servertype\":\"Transfer\",\"id\":\"FFFFFFFFFFFFFF\", \"file\":\"filename\",\"targetfolder\":\"\\10.1.200.101\somepath\somefile.trans.mp4\",\"errorcode\":\"4004\";\"errordetail\":\"not found\"}";


	analyzer.OnNewFrame(NULL,frame);

	frame =  "{\"command\":\"video_trans_task\"}";
	analyzer.OnNewFrame(NULL,frame);

	frame =  "{\"command\":\"dynamic_ppt_trans_task\"}";
	analyzer.OnNewFrame(NULL,frame);

	frame =  "{\"command\":\"dhaoigd\"}";
	analyzer.OnNewFrame(NULL,frame);

	
	frame =  "{\"command\":\"server_status_task\"}";
	analyzer.OnNewFrame(NULL,frame);

	cout << "************ TestTsMessageAnalyzer end *************" << endl;
}

void AppTest::TestDisptacher()
{
	Task* task = TaskVideoDb::GetInstance().GetTask();
	assert(task != NULL);
	task->SetTaskId("abcde11");

	VideoDisptacher* vdisp = &VideoDisptacher::GetInstance();
	
	vdisp->Submit(task);

	TestTaskWaitQueue(&GVideoTaskWaitQueue,"abcde11");
	
}

void AppTest::TestTsQueue()
{
    TsQueue ts_queue;
    DocTransServer ts1;
    ts1.SetId("AAA1");
    ts1.SetIp("142.255.54.1");
    VideoTransServer ts2;
    ts2.SetId("AAA2");
    ts2.SetIp("142.255.54.2");
    DpptTransServer ts3;
    ts3.SetId("AAA3");
    ts3.SetIp("142.255.54.3");
    DocTransServer ts4;
    ts4.SetId("AAA4");
    ts4.SetIp("142.255.54.3");
    ts_queue.PushTs(&ts1);
    ts_queue.PushTs(&ts2);
    ts_queue.PushTs(&ts3);
    ts_queue.PushTs(&ts4);
    list<TransServer *> *Server_lst = ts_queue.GetTsLst();
    
    assert((*Server_lst).size() == 4);
    string id = "AAA1";
    string ip = "142.255.54.1";
    assert(ts_queue.CheckSameTs(id,ip) == true);

    id = "AAA2";
    ip = "142.255.54.1";
    assert(ts_queue.CheckSameTs(id,ip) == false);

    ts_queue.DelTs(&ts1);
    ts_queue.DelTs(&ts3);
    Server_lst = ts_queue.GetTsLst();
    assert((*Server_lst).size() == 2);
    ts_queue.DelTs("AAA5");
    Server_lst = ts_queue.GetTsLst();
    assert((*Server_lst).size() == 2);

}

void AppTest::TestTsMessageDb()
{

}

void AppTest::TestTransServerDb()
{
	TransServer* ts0 = DocTransServerDb::GetInstance().GetTransServer();
	assert(DocTransServerDb::GetInstance().GetDbSize() == 0);
	assert(DocTransServerDb::GetInstance().GiveBackTransServer(ts0));
	assert(DocTransServerDb::GetInstance().GetDbSize() == 1);
	TransServer* ts1 = DocTransServerDb::GetInstance().GetTransServer();
	assert(ts0 == ts1);
	assert(DocTransServerDb::GetInstance().GetDbSize() == 0);
	TransServer* ts2 = DocTransServerDb::GetInstance().GetTransServer();
	//assert(DocTransServerDb::GetInstance().GetDbSize() == 0);
	TransServer* ts3 = DocTransServerDb::GetInstance().GetTransServer();
	//assert(DocTransServerDb::GetInstance().GetDbSize() == 0);
	assert(DocTransServerDb::GetInstance().GiveBackTransServer(ts2) == true);
	assert(DocTransServerDb::GetInstance().GetDbSize() == 1);
	assert(DocTransServerDb::GetInstance().GiveBackTransServer(ts3) == true);
	assert(DocTransServerDb::GetInstance().GetDbSize() == 2);
	ts3 = DocTransServerDb::GetInstance().GetTransServer();
	assert(DocTransServerDb::GetInstance().GetDbSize() == 1);
	assert(DocTransServerDb::GetInstance().GiveBackTransServer(ts0) == true);
	assert(DocTransServerDb::GetInstance().GetDbSize() == 2);
	ts3 = DocTransServerDb::GetInstance().GetTransServer();
	assert(DocTransServerDb::GetInstance().GetDbSize() == 1);
	ts3 = DocTransServerDb::GetInstance().GetTransServer();
	assert(DocTransServerDb::GetInstance().GetDbSize() == 0);
}

/*
void AppTest::TestMsgRecFromAPP()
{
	(SessionManager::GetInstance()).StartListenForAppServer(AppMessageAnalyzer::GetInstance());
	EventLooper::GetInstance().RunEventLoop();
}

void AppTest::TestMsgRecFromTs()
{
	(SessionManager::GetInstance()).StartListenForTS(TsMessageAnalyzer::GetInstance(),TransServerLeave::GetInstance());
	EventLooper::GetInstance().RunEventLoop();
}
*/


void	AppTest::setUp ()
{

}

template <class T>
TestCaller<T>* makeTestCaller(char* name, void (T::*f)() )
{
	return new TestCaller<T>(name, f);
}

#define ADDTEST(NAME)  testSuite->addTest (makeTestCaller("NAME", &AppTest::NAME));

Test *AppTest::suite ()
{
	TestSuite *testSuite = new TestSuite ("AppTest");
//	ADDTEST(TestAddTask);
//	ADDTEST(TestAppMessageAnalyzer);
//	ADDTEST(TestCommandDb);
//	ADDTEST(TestTaskWaitQueue);
//	ADDTEST(TestTaskRunQueue);
//	ADDTEST(TestTsMessageAnalyzer);
//	ADDTEST(TestDisptacher);
//	ADDTEST(TestMsgRecFromAPP);
//	ADDTEST(TestMsgRecFromTs);

    ADDTEST(TestTsQueue);

	

	return testSuite;
}

