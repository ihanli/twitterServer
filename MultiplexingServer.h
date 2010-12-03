#ifndef MULTIPLEXINGSERVER_H
#define MULTIPLEXINGSERVER_H


class MultiplexingServer
{
	public:
		MultiplexingServer(const unsigned short port = 3000);
		~MultiplexingServer();
	protected:
	private:
		SOCKET requestSocket;
		SOCKADDR_IN localhost;
		FD_SET actionFlag;
		SocketBase socketCreator;
		char* clientMessage;
};

#endif // MULTIPLEXINGSERVER_H
