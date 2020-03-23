#include "TransServer.h"

////////////////////////////////////// TransServer ///////////////////////////////////////////////
void TransServer::GivebackTs()
{
	m_tsDb->GiveBackTransServer(this);
}

////////////////////////////////////// DocTransServer ///////////////////////////////////////////////

DocTransServer::DocTransServer()
{
    m_tsDb = &DocTransServerDb::GetInstance();
}

DocTransServer::DocTransServer(string& id)
{
	SetId(id);
	m_serverType = DocTrans;
	//m_tsDb = &DocTransServerDb::GetInstance();
	DocTransServer();
}

/*
void DocTransServer::SetDocTransType(DocTransType docTransType)
{
    m_docTransType = docTransType;
}


DocTransType DocTransServer::GetDocTransType()
{
    return m_docTransType;
}

*/
void DocTransServer::SetDocTsType(DocTsType ts_type)
{
    m_docTsType = ts_type;
}
DocTsType DocTransServer::GetDocTsType()
{
    return m_docTsType;
}



////////////////////////////////////// VideoTransServer ///////////////////////////////////////////////

VideoTransServer::VideoTransServer()
{
    m_tsDb = &VideoTransServerDb::GetInstance();
    m_property = TASK_HEAVY;
}

VideoTransServer::VideoTransServer(string& id)
{
	SetId(id);
	m_serverType = VideoTrans;
	VideoTransServer();
}

void VideoTransServer::SetTsProperty(string servertypestr)
{
    if("VIDEO" == servertypestr)
    {
        m_property = TASK_HEAVY;
    }
    else if("VIDEO_LIGHT" == servertypestr)
    {
        m_property = TASK_LIGHT;
    }
}
TaskProperty VideoTransServer::GetTsProperty()
{
    return m_property;
}




////////////////////////////////////// DpptTransServer ///////////////////////////////////////////////

DpptTransServer::DpptTransServer()
{
    m_tsDb = &DpptTransServerDb::GetInstance();
}

DpptTransServer::DpptTransServer(string& id)
{
	SetId(id);
	m_serverType = DpptTrans;
	DpptTransServer();
}


