#ifndef __CDYCONNBASEIMP_H__
#define __CDYCONNBASEIMP_H__

#include <event2/util.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include "ConnBase.h"
using namespace std;

namespace cdy{

class ConnectorImp
{
	string m_peerAddr;
	short m_peerPort;
	IConnectorAcceptorSink *m_sink;
	
	evutil_socket_t m_fd;
	struct event_base *m_evBase;
	struct event *m_connectEvent;
	Connector *m_wrapper;
	bool m_connecting;
public:
	ConnectorImp(const string &host, short port, IConnectorAcceptorSink *sink, Connector *wrapper);
	~ConnectorImp();

	//@param timeout means the connect timeout value in millesecond.
	int Connect(long timeout);
	
	void Cancel();
	
	void OnConnectResult(evutil_socket_t sock, int err, string &peerAddr, short peerPort);
};

class AcceptorImp
{
	string m_addr;
	short m_port;
	IConnectorAcceptorSink *m_sink;
	
	evutil_socket_t m_fd;
	struct event_base *m_evBase;
	struct event *m_connectEvent;
	Acceptor *m_wrapper;
public:
	AcceptorImp(const std::string &addr, short port, IConnectorAcceptorSink *sink, Acceptor *wrapper);
	
	int StartListen();
	
	void Stop();
	
	void OnConnectResult(evutil_socket_t sock, int err, string &peerAddr, short peerPort);
};

class ConnectionImp : public Connection
{
public:
	ConnectionImp(evutil_socket_t fd, struct event_base *evBase);
	~ConnectionImp();
	
	virtual void SetConnectionSink(IConnectionSink *sink) { this->m_sink = sink; }
	
	virtual int Send(const char *data, int len);
	
	virtual void Close();

	virtual const std::string& GetPeerAddress();

	virtual short GetPeerPort();

	void OnRead(evutil_socket_t sock);
	
	void OnWrite(evutil_socket_t sock);

	void SetPeerAddress(const string addr);

	void SetPeerPort(short port);
protected:
	IConnectionSink *m_sink;
	string m_peerAddr;
	short m_peerPort;
	evutil_socket_t m_fd;
	struct event_base *m_evBase;
	struct event *m_readEvent;
	struct event *m_writeEvent;
	
	static const int MAX_READ_LEN = 2048;
	char m_readBuffer[2048];
	struct evbuffer *m_writeBuffer;
	bool m_closing;
};

};
#endif //__CDYCONNBASEIMP_H__

