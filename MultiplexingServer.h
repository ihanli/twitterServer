#ifndef MULTIPLEXINGSERVER_H
#define MULTIPLEXINGSERVER_H

#include <map>
#include "SocketBase.h"

#define MAXCLIENTS 10			// how many clients the server can handle
#define BUFFERSIZE 140			// maximum lenghth of the tweets

//The MultiplexingServer class handles everything from accepting clients to I/O multiplexing.

class MultiplexingServer
{
	public:
		MultiplexingServer();
		~MultiplexingServer();

	protected:
		map<unsigned int, SOCKET> clients;
		char* clientMessage;

		void configServer(const unsigned short port = 3000);
		void clientListener(void);
		void sendToClient(const SOCKET* client, const char* message);
		void receive(const SOCKET* clientSocket);
		virtual void commandInterpreter(char* command[], const SOCKET clientSocket) = 0;

	private:
		SOCKET requestSocket;
		SOCKADDR_IN localhost;
		FD_SET actionFlag;
		SocketBase socketCreator;

		void closeSockets(void);
		void acceptClient(void);
		void setClientToOffline(SOCKET* clientSocket);
		void setClientToOnline(void);
		void closeRequestSocket(void) const;
};

#endif // MULTIPLEXINGSERVER_H
