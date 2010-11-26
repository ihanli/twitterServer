#include "TwitterServer.h"

TwitterServer::TwitterServer(const unsigned short port, const unsigned int size, const int af, const WORD version, const int type, const int protocol) :
               socketCreator(version, type, protocol), bufferSize(size)
{
    try
    {
        socketCreator.createSocket(&requestSocket, af);

        for(int i = 0;i < MAXCLIENTS;i++)
		{
			clients[i] = INVALID_SOCKET;
		}
    }
    catch(unsigned char* e)
    {
        printf("%c", *e);
    }
}

TwitterServer::~TwitterServer(void)
{
	closeSockets();
	WSACleanup();
}

void TwitterServer::configServer(const unsigned short port, const int af)
{
    int errorCode;

    memset(&localhost, 0, sizeof(SOCKADDR_IN));
    localhost.sin_family = af;
    localhost.sin_port = htons(port);
    localhost.sin_addr.s_addr = ADDR_ANY;

    printf("\nBinding socket...");

    errorCode = bind(requestSocket, (SOCKADDR*)&localhost, sizeof(SOCKADDR_IN));

    if(errorCode == SOCKET_ERROR)
    {
        closeRequestSocket();

        throw "\nFAIL: Unable to bind socket!";
    }
    else
    {
        printf("\nSUCCESS: Bound socket to port %u!", port);
    }

    printf("\nStarting listen mode...");

    errorCode = listen(requestSocket, 10);

    if(errorCode == SOCKET_ERROR)
    {
        closeRequestSocket();

        throw "\nFAIL: Unable to initiate listen mode!";
    }
    else
    {
        printf("\nSUCCESS: Listen mode started!");
    }
}

void TwitterServer::clientListener(void)
{
	int errorCode;

	FD_ZERO(&mainActionFlag);
	FD_SET(requestSocket, &mainActionFlag);

	for(int i = 0; i < MAXCLIENTS;i++)
	{
		if(clients[i] != INVALID_SOCKET)
		{
			FD_SET(clients[i], &mainActionFlag);
		}
	}

	errorCode = select(0, &mainActionFlag, NULL, NULL, 0);

	if(errorCode == SOCKET_ERROR)
	{
		throw "\nFAIL: Something went wrong with SELECT!";
	}

	if(FD_ISSET(requestSocket, &mainActionFlag))
	{
		for(int i = 0;i < MAXCLIENTS;i++)
		{
			if(clients[i] == INVALID_SOCKET)
			{
				try
				{
					clients[i] = acceptClient();
				}
				catch(const char* e)
				{
					printf("%c", *e);
				}

				break;
			}
		}
	}

	for(int i = 0;i < MAXCLIENTS;i++)
	{
		if(clients[i] != INVALID_SOCKET)
		{
			if(FD_ISSET(clients[i], &mainActionFlag))
			{
				try
				{
					//TODO: analyse commands from clients

					receive(&clients[i]);

					printf("\nClient said: %c\n", *clientMessage);

					try
					{
						sendToClient(&clients[i], clientMessage);
					}
					catch(const char* e)
					{
						printf("%c", *e);
					}
				}
				catch(const char* e)
				{
					printf("%c", *e);
					closesocket(clients[i]);
					clients[i] = INVALID_SOCKET;
				}
			}
		}
	}
}

int TwitterServer::acceptClient(void)
{
	int comSocket;

	comSocket = accept(requestSocket, NULL, NULL);

	if(comSocket == SOCKET_ERROR)
	{
		throw "\nFAIL: Couldn't connect client!";
	}
	else
	{
		printf("\nSUCCESS: Connected with client!");
	}

	return comSocket;
}

void TwitterServer::sendToClient(const SOCKET* client, const char* message)
{
    //TODO: check if whole message was sent

    int errorCode;

    errorCode = send(*client, message, bufferSize, 0);

    if(errorCode == SOCKET_ERROR)
    {
        throw "\nFAIL: Unable to send message!";
    }
}

void TwitterServer::receive(SOCKET* clientSocket)
{
    //TODO: check if we received the whole message

    int errorCode;

    errorCode = recv(*clientSocket, clientMessage, bufferSize, 0);

    if(errorCode == 0)
    {
        throw "\nFAIL: Lost connection to client!";
    }
    else if(errorCode == SOCKET_ERROR)
    {
        throw "\nFAIL: Unable to receive message!";
    }
}

bool TwitterServer::shutdownFlag(void) const { return shutdownServer; }

unsigned int TwitterServer::getBufferSize(void) const { return bufferSize; }

void TwitterServer::setBufferSize(unsigned int size){ bufferSize = size; }

void TwitterServer::closeRequestSocket(void) const
{
    closesocket(requestSocket);
    WSACleanup();
}

void TwitterServer::closeSockets(void)
{
    closesocket(requestSocket);
	FD_ZERO(&mainActionFlag);

	for(int i = 0;i < MAXCLIENTS;i++)
	{
		closesocket(clients[i]);
	}
}