#include "MultiplexingServer.h"

MultiplexingServer::MultiplexingServer():
					socketCreator()
{
	try
	{
		socketCreator.createSocket(&requestSocket, AF_INET);	// set up the listening socket by giving the socket and protocol
		FD_ZERO(&actionFlag);									// set status flag to zero
		FD_SET(requestSocket, &actionFlag);						// set up status flag for listen socket

		for(int i = 0;i < MAXCLIENTS;i++)						// set all client sockets to invalid
			clients[i] = INVALID_SOCKET;
	}
	catch(string failure)										// if something fails print error
	{
		printf("%s", failure.c_str());
	}
}

MultiplexingServer::~MultiplexingServer()
{
	closeSockets();						// close all socktes and cleanup
	WSACleanup();
}

void MultiplexingServer::configServer(const unsigned short port)
{
	int errorCode;

	memset(&localhost, 0, sizeof(SOCKADDR_IN));		// reset the localhost address to zero
	localhost.sin_family = AF_INET;					// set the protocol, tcp/ip in this case
	localhost.sin_port = htons(port);				// set the port
	localhost.sin_addr.s_addr = ADDR_ANY;			// set standard ip address

	printf("\nBinding socket...");

	errorCode = bind(requestSocket, (SOCKADDR*)&localhost, sizeof(SOCKADDR_IN));	// bind the listen socket and get error code

	if(errorCode == SOCKET_ERROR)
	{
		closeRequestSocket();		// if something goes wrong, close socket

		throw exceptionTexter("\nFAIL: Unable to bind socket! (Error Code: ", errorCode);	// throw message with error code
	}
	else
		printf("\nSUCCESS: Bound socket to port %u!", port);


	printf("\nStarting listen mode...");

	errorCode = listen(requestSocket, 10);		// start listening and get error code

	if(errorCode == SOCKET_ERROR)				// our standard error handling
	{
		closeRequestSocket();

		throw exceptionTexter("\nFAIL: Unable to initiate listen mode! (Error Code: ", errorCode);
	}
	else
	{
		printf("\nSUCCESS: Listen mode started!");
	}
}

void MultiplexingServer::clientListener(void)
{
	int errorCode;
	char* command[2];	// command string for login, logout, follow

	setClientToOnline();		// set  client stati to online

	errorCode = select(1000, &actionFlag, NULL, NULL, 0);	// select client

	if(errorCode == SOCKET_ERROR)							// check for error
		throw exceptionTexter("\nFAIL: Something went wrong with SELECT! (Error Code: ", errorCode);

	if(FD_ISSET(requestSocket, &actionFlag))	// 
	{
		try
		{
			acceptClient();						// call our accept client function
		}
		catch(string failure)					// check for errors
		{
			printf("%s", failure.c_str());
		}
	}

	for(int i = 0;i < MAXCLIENTS;i++)	// iterate through all client sockets
	{
		if(clients[i] != INVALID_SOCKET && FD_ISSET(clients[i], &actionFlag))	// if current client is valid and set online
		{
			try
			{
				//FIXME: escaping character for command

				receive(&clients[i]);		// call our receive messages function

				commandInterpreter(command, clients[i]);	// call our interpret commands function

				delete [] command[1];
				delete [] command[0];
			}
			catch(const char* failure)		// on failure set client offline and print error
			{
				printf("%s", failure);
				setClientToOffline(&clients[i]);
			}
		}
	}
}

void MultiplexingServer::acceptClient(void)
{
	for(int j = 0;j < MAXCLIENTS;j++)	// go through all possible sockets
	{
		if(clients[j] == INVALID_SOCKET)	// if the current socket is invalid
		{
			clients[j] = accept(requestSocket, NULL, NULL);		// you can accept the client here

			if(clients[j] == INVALID_SOCKET)
				throw exceptionTexter("\nFAIL: Couldn't connect client! (socket ", clients[j]);

			else
			{
				printf("\nSUCCESS: Connected with client! (socket %d)", clients[j]);
				break;
			}
		}
	}
}

void MultiplexingServer::sendToClient(const SOCKET* client, const char* message)
{
	//TODO: check if whole message was sent

	int errorCode;

	// send a message to a client/tweeter by giving the client socket, message and size
	errorCode = send(*client, message, BUFFERSIZE, 0);

	if(errorCode == SOCKET_ERROR)
		throw exceptionTexter("\nFAIL: Unable to send message! (Error Code: ", errorCode);
}

void MultiplexingServer::receive(const SOCKET* clientSocket)
{
	//TODO: check if we received the whole message

	int errorCode;

	// receive a message by the client socket, content and size
	errorCode = recv(*clientSocket, clientMessage, BUFFERSIZE, 0);

	if(errorCode == 0)
		throw exceptionTexter("\nFAIL: Lost connection to client! (Error Code: ", errorCode);

	else if(errorCode == SOCKET_ERROR)// || !strcmp("offline", clientMessage))
		throw exceptionTexter("\nClient went offline! (socket ", *clientSocket);
}

void MultiplexingServer::setClientToOffline(SOCKET* clientSocket)
{
	closesocket(*clientSocket);			// close the given client socket
	*clientSocket = INVALID_SOCKET;		// and set it to invalid

	FD_ZERO(&actionFlag);				// set the status flag to zero
	FD_SET(requestSocket, &actionFlag);	// set the status flag for the request socket to zero to be available again
}

void MultiplexingServer::setClientToOnline(void)
{
	for(int i = 0; i < MAXCLIENTS;i++)			// go through all possible sockets
	{
		if(clients[i] != INVALID_SOCKET)		// if it is not invalid
			FD_SET(clients[i], &actionFlag);	// set the status to active
	}
}

void MultiplexingServer::closeRequestSocket(void) const
{
	closesocket(requestSocket);					// close the listen socket and cleanup
	WSACleanup();
}

void MultiplexingServer::closeSockets(void)
{
	closesocket(requestSocket);
	FD_ZERO(&actionFlag);

	for(int i = 0;i < MAXCLIENTS;i++)	// close all possible sockets
		closesocket(clients[i]);
}
