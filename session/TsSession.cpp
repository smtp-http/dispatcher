#include "InterfaceApiImp.h"


TsSession::TsSession()
	: m_imp(new TsSessionImp(this))
{

}

TsSession::~TsSession()
{
	if(m_imp){
		m_imp->Close();
		delete m_imp;
		m_imp = NULL;
	}
}


void TsSession::SendMessage(string msg)
{
	if(m_imp)
		m_imp->SendMessage(msg);
}
	
void TsSession::Close()
{
	if(m_imp)
		m_imp->Close();
}




TsClientSession::TsClientSession()
	: m_imp(new TsClientSessionImp(NULL,this))
{
	
}
TsClientSession::~TsClientSession()
{
	if(m_imp){
		m_imp->Close();
		delete m_imp;
		m_imp = NULL;
	}
}

void TsClientSession::SendMessage(string msg)
{
	if(m_imp)
		m_imp->SendMessage(msg);
}


