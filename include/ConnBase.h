#ifndef __CDYCONNBASE_H__
#define __CDYCONNBASE_H__

#include <string>

namespace cdy{

class Connection;
class IConnectorAcceptor;

//User should implemention this interface to get the new connection notify.
class IConnectorAcceptorSink
{
public:
	//@param conn is the new connection if succeed, else null.
	//@ca shows the Connector or Acceptor object which created the new connection.
	virtual void OnConnection(Connection *conn, IConnectorAcceptor *ca) = 0;
};

//User should implemention this interface to get the notification of data incoming
//or the buffer is not full so user can write more data, and the event of connection
//been disconnected.
class IConnectionSink
{
public:
	// called when new data income.
	virtual void OnData(const char *buf, int length, Connection *conn) = 0;
	// called when user can write data to network.
	virtual void OnWrite(Connection *conn) = 0;
	// called when the connection is broken, if user call Connection::Close(), 
	// will not get this callback.
	virtual void OnDisconnect(int reason, Connection *conn) = 0;
};

class IConnectorAcceptor
{
protected:
	int m_error;
public:
	IConnectorAcceptor() : m_error(0) {}
	virtual ~IConnectorAcceptor() {}
	
	virtual bool IsConnector() = 0;

	virtual int GetError() { return m_error; }
};

class Connector : public IConnectorAcceptor
{
public:
	Connector(const std::string &host, short port, IConnectorAcceptorSink *sink);

	~Connector();
	
	//@param timeout means the connect timeout value in millesecond.
	int Connect(long timeout);
	
	void Cancel();
	
	bool IsConnector() { return true; }
private:
	class ConnectorImp *m_imp;
};

class Acceptor : public IConnectorAcceptor
{
public:
	Acceptor(const std::string &addr, short port, IConnectorAcceptorSink *sink);

	~Acceptor();
	
	int StartListen();
	
	void Stop();
	
	virtual bool IsConnector() { return false; }
private:
	class AcceptorImp *m_imp;
};

class Connection
{
public:
	virtual ~Connection();
	
	virtual void SetConnectionSink(IConnectionSink *sink) = 0;
	
	virtual int Send(const char *data, int len) = 0;
	
	virtual void Close() = 0;

	virtual const std::string& GetPeerAddress() = 0;

	virtual short GetPeerPort() = 0;
};

};
#endif //__CDYCONNBASE_H__

