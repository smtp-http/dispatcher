#include <cassert>

#ifdef _WIN32
#include <WS2tcpip.h>
#else
 #include <arpa/inet.h>
//#include <sys/types.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
#endif
#include <errno.h>
#include <memory.h>

#include "ConnBaseImp.h"
#include "EventLooperImp.h"

using namespace cdy;

Connector::Connector(const std::string &host, short port, IConnectorAcceptorSink *sink)
	: m_imp(new ConnectorImp(host, port, sink, this))
{
}
	
Connector::~Connector()
{
	m_imp->Cancel();
	delete m_imp;
	m_imp = NULL;
}

//@param timeout means the connect timeout value in millesecond.
int Connector::Connect(long timeout)
{
	if (m_imp)
		return m_imp->Connect(timeout);
	return -1;
}
	
void Connector::Cancel()
{
	if (m_imp)
		m_imp->Cancel();
}

Acceptor::Acceptor(const std::string &addr, short port, IConnectorAcceptorSink *sink)
	: m_imp(new AcceptorImp(addr, port, sink, this))
{
}

Acceptor::~Acceptor()
{
	delete m_imp;
	m_imp = NULL;
}

int Acceptor::StartListen()
{
	if (m_imp)
		return m_imp->StartListen();
	return -1;
}
	
void Acceptor::Stop()
{
	if (m_imp)
		m_imp->Stop();
}

Connection::~Connection()
{
}

bool StrToAddress(const string &str, short port, struct sockaddr_in *addr)
{
	if (!addr)
		return false;
		
    addr->sin_family = AF_INET;  
    addr->sin_port = htons(port);  
#ifdef WIN32  
    addr->sin_addr.S_un.S_addr = inet_addr(str.c_str());  
#else  
    addr->sin_addr.s_addr = inet_addr(str.data());  
#endif  
    memset(addr->sin_zero, 0x00, 8);
	return true;
}

int SockAddrConvert(sockaddr_storage *addr, socklen_t len, string &str, short &port)
{
	char ipstr[INET6_ADDRSTRLEN];

	if (addr->ss_family == AF_INET){
		struct sockaddr_in *s = (struct sockaddr_in *)addr;
		port = ntohs(s->sin_port);
		inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof(ipstr));
	} else { // AF_INET6
		struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
		port = ntohs(s->sin6_port);
		inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof(ipstr));
	}

	str.assign(ipstr);
	return 0;
}

int GetPeerAddress(evutil_socket_t sock, string &saddr, short &port)
{
	struct sockaddr_storage addr;
	socklen_t sockLen = sizeof(addr);

	if (getpeername(sock, (struct sockaddr*)&addr, &sockLen) != 0){
		int err = evutil_socket_geterror(sock);
		return err;
	}

	return SockAddrConvert(&addr, sockLen, saddr, port);
}

static void on_connect(evutil_socket_t sock, short events, void * args)
{
	ConnectorImp *connector = (ConnectorImp*)args;
	string addr;
	short port = 0;
	int err = 0;
	if (events & EV_TIMEOUT)
		err = -1;
	else
		err = GetPeerAddress(sock, addr, port);

	if (connector){
		connector->OnConnectResult(sock, err, addr, port);
	}
}

static void on_accept(evutil_socket_t sock, short events, void * args)
{
	AcceptorImp *acceptor = (AcceptorImp*)args;
	string saddr;
	short port = 0;
	sockaddr_storage addr;
#ifdef WIN32  
	int addrLen = sizeof(addr);
#else
	socklen_t addrLen = sizeof(addr);
#endif
	
	int err = 0;
	if (events & EV_TIMEOUT)
		err = -1;
	if (acceptor){
		evutil_socket_t newSock = accept(sock, (sockaddr*)&addr, &addrLen);
		err = GetPeerAddress(newSock, saddr, port);
		acceptor->OnConnectResult(newSock, err, saddr, port);
	}
}

static void on_read(evutil_socket_t sock, short events, void * args)
{
	ConnectionImp *conn = (ConnectionImp*)args;
	if (events & EV_READ){
		if (conn){
			conn->OnRead(sock);
		}
	}
}

static void on_write(evutil_socket_t sock, short events, void * args)
{
	ConnectionImp *conn = (ConnectionImp*)args;
	if (events & EV_WRITE){
		if (conn){
			conn->OnWrite(sock);
		}
	}
}

AcceptorImp::AcceptorImp(const std::string &addr, short port, IConnectorAcceptorSink *sink, Acceptor *wrapper)
	: m_addr(addr)
	, m_port(port)
	, m_sink(sink)
	, m_fd(-1)
	, m_evBase(NULL)
	, m_connectEvent(NULL)
	, m_wrapper(wrapper)
{
}

int AcceptorImp::StartListen()
{
	struct sockaddr_in addr;
	if (!StrToAddress(m_addr, m_port, &addr))
		return -1;
	
	m_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (evutil_make_socket_nonblocking(m_fd) != 0)
		return -1;
	if (evutil_make_listen_socket_reuseable(m_fd) != 0)
		return -1;

	if (bind(m_fd, (struct sockaddr*)&addr, sizeof(addr)) != 0)
		return -1;
		
	EventLooperImp &evLooper = (EventLooperImp&)EventLooper::GetInstance();
	m_evBase = evLooper.CurrentEvBase();
	m_connectEvent = event_new(m_evBase, m_fd, EV_READ | EV_PERSIST, on_accept, (void*)this);
	event_add(m_connectEvent, NULL);

	return listen(m_fd, 5);
}

void AcceptorImp::Stop()
{
    evutil_closesocket(m_fd);
    m_fd = -1;
    if (m_connectEvent)
    {
        event_del(m_connectEvent);
    }
    m_connectEvent = NULL;
    
}

void AcceptorImp::OnConnectResult(evutil_socket_t sock, int error, string &peerAddr, short peerPort)
{
	if (!m_sink)
		return;
	
	ConnectionImp *conn = NULL;
	if (!error){
		conn = new ConnectionImp(sock, m_evBase);
		conn->SetPeerAddress(peerAddr);
		conn->SetPeerPort(peerPort);
	}
	m_sink->OnConnection(conn, this->m_wrapper);
}

ConnectorImp::ConnectorImp(const string &host, short port, IConnectorAcceptorSink *sink, Connector *wrapper)
	: m_peerAddr(host)
	, m_peerPort(port)
	, m_sink(sink)
	, m_fd(-1)
	, m_evBase(NULL)
	, m_connectEvent(NULL)
	, m_wrapper(wrapper)
{
	EventLooperImp &el = (EventLooperImp&)EventLooper::GetInstance();
	m_evBase = el.CurrentEvBase();
}

ConnectorImp::~ConnectorImp()
{
	Cancel();

	if (m_fd != -1){
		evutil_closesocket(m_fd);
		m_fd = -1;
	}

	event_del(m_connectEvent);
	event_free(m_connectEvent);
	m_connectEvent = NULL;
}

int ConnectorImp::Connect(long timeout)
{
	struct timeval tv = {timeout/1000, (timeout%1000)*1000};
	struct sockaddr_in peerAddr;
	if (!StrToAddress(m_peerAddr, m_peerPort, &peerAddr))
		return -1;
	
	m_connecting = true;
	if(m_fd != -1){
		evutil_closesocket(m_fd);
		m_fd = -1;
	}
	
	if(m_connectEvent != NULL){
		event_del(m_connectEvent);
		event_free(m_connectEvent);
		m_connectEvent = NULL;
	}
	
	m_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	evutil_make_socket_nonblocking(m_fd); 
	
	m_connectEvent = event_new(m_evBase, m_fd, EV_WRITE, on_connect, (void*)this);
	event_add(m_connectEvent, &tv);
	connect(m_fd, (struct sockaddr*)&peerAddr, sizeof(peerAddr));
	return 0;
}

void ConnectorImp::Cancel()
{
	if (!m_connecting)
		return;

	m_sink = NULL;
	evutil_closesocket(m_fd);
	m_fd = -1;
	
	event_del(m_connectEvent);
	event_free(m_connectEvent);
	m_connectEvent = NULL;
}

void ConnectorImp::OnConnectResult(evutil_socket_t sock, int error, string &peerAddr, short peerPort)
{
	m_connecting = false;

	if (!m_sink)
		return;
	
	ConnectionImp *conn = NULL;
	if (!error){
		conn = new ConnectionImp(sock, m_evBase);
		conn->SetPeerAddress(peerAddr);
		conn->SetPeerPort(peerPort);
		m_fd = -1; //to avoid close it later.
	}
	m_sink->OnConnection(conn, this->m_wrapper);
}

ConnectionImp::ConnectionImp(evutil_socket_t fd, struct event_base *evBase)
	: m_sink(NULL)
	, m_peerPort(0)
	, m_fd(fd)
	, m_evBase(evBase)
	, m_closing(false)
{
	m_readEvent = event_new(m_evBase, m_fd, EV_READ | EV_PERSIST, on_read, (void*)this);
	m_writeEvent = event_new(m_evBase, m_fd, EV_WRITE, on_write, (void*)this);

	event_add(m_readEvent, NULL);
	//event_add(m_writeEvent, NULL);
	m_writeBuffer = evbuffer_new();
}

ConnectionImp::~ConnectionImp()
{
	Close();
}

int ConnectionImp::Send(const char *data, int len)
{
	if (m_closing)
		return -1;
		
	size_t left = evbuffer_get_length(m_writeBuffer);
	if (evbuffer_add(m_writeBuffer, data, len) != 0)
		return -1;
	
	int rt = 0;
	if (left != 0){
		rt = event_add(m_writeEvent, NULL);
	} else {
		rt = evbuffer_write(m_writeBuffer, m_fd);
	}
	return rt;
}

const std::string& ConnectionImp::GetPeerAddress()
{
	return m_peerAddr;
}

short ConnectionImp::GetPeerPort()
{
	return m_peerPort;
}

void ConnectionImp::OnRead(evutil_socket_t sock)
{
	assert (sock == m_fd);
	int sz = recv(sock, m_readBuffer, MAX_READ_LEN, 0);
	int err;
	if(sz <= 0){
#ifdef WIN32
		err = WSAGetLastError();
#else
		err = errno;
#endif
		if (err == EWOULDBLOCK || err == EAGAIN)
			return;

		IConnectionSink *sink = m_sink;
		Close();
		if (sink){
			sink->OnDisconnect(err, this);
		}
	} else {
		if (m_sink){
			m_sink->OnData(m_readBuffer, sz, this);
		}
	}
}

void ConnectionImp::OnWrite(evutil_socket_t sock)
{
	assert (sock == m_fd);
	size_t left = evbuffer_get_length(m_writeBuffer);
	
	if (left != 0){
		int sz = evbuffer_write(m_writeBuffer, m_fd);
		
		if (sz < 0){
			int err;
#ifdef WIN32
			err = WSAGetLastError();
#else
			err = errno;
#endif
			if (err != EWOULDBLOCK && err != EAGAIN){
				IConnectionSink *sink = m_sink;
				Close();
				if (sink){
					sink->OnDisconnect(err, this);
				}
			}
		} else if ((size_t)sz < left){
			event_add(m_writeEvent, NULL);
		}
	}
}

void ConnectionImp::Close()
{
	if (m_closing)
		return;
		
	m_closing = true;
	m_sink = NULL;
	evutil_closesocket(m_fd);
	
	m_fd = -1;
	event_del(m_readEvent);
	event_free(m_readEvent);
	m_readEvent = NULL;
	event_del(m_writeEvent);
	event_free(m_writeEvent);
	m_writeEvent = NULL;
	evbuffer_free(m_writeBuffer);
	m_writeBuffer = NULL;
}


void ConnectionImp::SetPeerAddress(const string addr)
{
	m_peerAddr.assign(addr);
}

void ConnectionImp::SetPeerPort(short port)
{
	m_peerPort = port;
}

