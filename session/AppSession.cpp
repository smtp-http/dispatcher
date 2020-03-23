#include "InterfaceApiImp.h"


AppServerSession::AppServerSession(AppServerSessionImp *imp)
	: m_imp(imp)
{

}

AppServerSession::~AppServerSession()
{

}



void AppServerSession::SendMessage(string msg)
{
    if(m_imp != NULL){
        m_imp->SendMessage(msg);
    }
}



AppClientSession::AppClientSession(AppClientSessionImp *imp)
	: m_imp(imp)
{

}

AppClientSession::~AppClientSession()
{

}


void AppClientSession::SendMessage(string msg)
{
    if(m_imp != NULL){
        m_imp->SendMessage(msg);
    }
}

