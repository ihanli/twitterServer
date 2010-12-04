#include "MultiplexingServer.h"

MultiplexingServer::MultiplexingServer():
					socketCreator()
{
	try
	{
		socketCreator.createSocket(&requestSocket, AF_INET);	// set up the listening socket
		FD_ZERO(&actionFlag);									// set status flag to zero
		FD_SET(requestSocket, &actionFlag);						// set up status flag for listen socket

		for(int i = 0;i < MAXCLIENTS;i++)						// set all client socktets to invalid
		{
			clients[i] = INVALID_SOCKET;
		}
	}
	catch(string failure)										// if something fails print error
	{
		printf("%s", failure.c_str());
	}
}

MultiplexingServer::~MultiplexingServer()
{
	closeSockets();						// close all socktes
	WSACleanup();
}

void MultiplexingServer::configServer(const unsigned short port)
{
	int errorCode;

	memset(&localhost, 0, sizeof(SOCKADDR_IN));		// reset all to zero
	localhost.sin_family = AF_INET;					// set tcp/ip protocol
	localhost.sin_port = htons(port);				// set port
	localhost.sin_addr.s_addr = ADDR_ANY;			// set standard ip address

	printf("\nBinding socket...");

	errorCode = bind(requestSocket, (SOCKADDR*)&localhost, sizeof(SOCKADDR_IN));	// bind listen socket, get error code

	if(errorCode == SOCKET_ERROR)
	{
		closeRequestSocket();		// if something goes wrong, close socket

		throw exceptionTexter("\nFAIL: Unable to bind socket! (Error Code: ", errorCode);	// throw message with error code
	}
	else
	{
		printf("\nSUCCESS: Bound socket to port %u!", port);
	}

	printf("\nStarting listen mode...");

	errorCode = listen(requestSocket, 10);		// start listening, get error code

	if(errorCode == SOCKET_ERROR)
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

	if(FD_ISSET(requestSocket, &actionFlag))
	{
		try
		{
			acceptClient();	// accept client
		}
		catch(string failure)			// check for errors
		{
			printf("%s", failure.c_str());
		}
	}

	for(int i = 0;i < MAXCLIENTS;i++)
	{
		if(clients[i] != INVALID_SOCKET && FD_ISSET(clients[i], &actionFlag))	// if client is valid and set online
		{
			try
			{
				//FIXME: escaping character for command

				receive(&clients[i]);		// receive messages

				commandInterpreter(command, clients[i]);	// interpret commands

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
	for(int j = 0;j < MAXCLIENTS;j++)	// go through all open sockets
	{
		if(clients[j] == INVALID_SOCKET)	// if the current socket is invalid
		{
			clients[j] = accept(requestSocket, NULL, NULL);		// accept client here

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

	int errorCode;		// sends a message to a client/tweeter

	errorCode = send(*client, message, BUFFERSIZE, 0);

	if(errorCode == SOCKET_ERROR)
	{
		throw exceptionTexter("\nFAIL: Unable to send message! (Error Code: ", errorCode);
	}
}

void MultiplexingServer::receive(const SOCKET* clientSocket)
{
	//TODO: check if we received the whole message

	int errorCode;

	errorCode = recv(*clientSocket, clientMessage, BUFFERSIZE, 0); // receive a message

	if(errorCode == 0)
	{
		throw exceptionTexter("\nFAIL: Lost connection to client! (Error Code: ", errorCode);
	}
	else if(errorCode == SOCKET_ERROR)// || !strcmp("offline", clientMessage))
	{
		throw exceptionTexter("\nClient went offline! (socket ", *clientSocket);
	}
}

void MultiplexingServer::setClientToOffline(SOCKET* clientSocket)
{
	closesocket(*clientSocket);			// close the client socket
	*clientSocket = INVALID_SOCKET;		// and set to invalid

	FD_ZERO(&actionFlag);				// set the status flag to zero
	FD_SET(requestSocket, &actionFlag);	// set the status flag for the request socket to zero
}

void MultiplexingServer::setClientToOnline(void)
{
	for(int i = 0; i < MAXCLIENTS;i++)	// go through all open sockets
	{
		if(clients[i] != INVALID_SOCKET)	// if not invalid
		{
			FD_SET(clients[i], &actionFlag);	// set status to active
		}
	}
}

void MultiplexingServer::closeRequestSocket(void) const
{
	closesocket(requestSocket);
	WSACleanup();
}

void MultiplexingServer::closeSockets(void)
{
	closesocket(requestSocket);
	FD_ZERO(&actionFlag);

	for(int i = 0;i < MAXCLIENTS;i++)	// close all open sockets
	{
		closesocket(clients[i]);
	}
}
