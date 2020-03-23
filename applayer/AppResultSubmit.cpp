#include "AppResultSubmit.h"
#include <iostream>

//////////////////////////////   AppResultSubmit   //////////////////////
AppResultSubmit::AppResultSubmit()
{
	next = NULL;
}

//////////////////////////////   DocAppResultSubmit   //////////////////////

DocAppResultSubmit::DocAppResultSubmit()
{

}

DocAppResultSubmit& DocAppResultSubmit::GetInstance()
{
	static  DocAppResultSubmit instance_;
	return instance_; 	
}

void DocAppResultSubmit::OnResultSubmit(int result,void *pTaskMessage)
{
	if(false){
		cout << "DocAppResultSubmit OnResultSubmit............ " <<endl;
		return;
	} else {
		AppResultSubmit::OnResultSubmit(result,pTaskMessage);
	}
}


//////////////////////////////   DpptAppResultSubmit   //////////////////////

DpptAppResultSubmit::DpptAppResultSubmit()
{

}

DpptAppResultSubmit& DpptAppResultSubmit::GetInstance()
{
	static  DpptAppResultSubmit instance_;
	return instance_; 	
}

void DpptAppResultSubmit::OnResultSubmit(int result,void *pTaskMessage)
{
	if(false){
		cout << "DpptAppResultSubmit OnResultSubmit............ " <<endl;
		return;
	} else {
		AppResultSubmit::OnResultSubmit(result,pTaskMessage);
	}
}


//////////////////////////////   VideoAppResultSubmit   //////////////////////

VideoAppResultSubmit::VideoAppResultSubmit()
{

}

VideoAppResultSubmit& VideoAppResultSubmit::GetInstance()
{
	static  VideoAppResultSubmit instance_;
	return instance_; 	
}

void VideoAppResultSubmit::OnResultSubmit(int result,void *pTaskMessage)
{
	if(false){
		cout << "VideoAppResultSubmit OnResultSubmit............ " <<endl;
		return;
	} else {
		AppResultSubmit::OnResultSubmit(result,pTaskMessage);
	}
}


//////////////////////////////   NoUseTsAppResultSubmit   //////////////////////

NoUseTsAppResultSubmit::NoUseTsAppResultSubmit()
{

}

NoUseTsAppResultSubmit& NoUseTsAppResultSubmit::GetInstance()
{
	static  NoUseTsAppResultSubmit instance_;
	return instance_; 	
}

void NoUseTsAppResultSubmit::OnResultSubmit(int result,void *pTaskMessage)
{
	if(false){
		cout << "NoUseTsAppResultSubmit OnResultSubmit............ " <<endl;
		return;
	} else {
		AppResultSubmit::OnResultSubmit(result,pTaskMessage);
	}
}
