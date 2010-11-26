#include "TwitterServer.h"

TwitterServer::TwitterServer(const unsigned short port) :
               socketCreator()
{
    try
    {
        socketCreator.createSocket(&requestSocket, AF_INET);
        FD_ZERO(&actionFlag);
		FD_SET(requestSocket, &actionFlag);

        for(int i = 0;i < MAXCLIENTS;i++)
		{
			clients[i] = INVALID_SOCKET;
		}
    }
    catch(const char* failure)
    {
        printf("%s", failure);
    }
}

TwitterServer::~TwitterServer(void)
{
	closeSockets();
	WSACleanup();
}

void TwitterServer::configServer(const unsigned short port)
{
    int errorCode;

    memset(&localhost, 0, sizeof(SOCKADDR_IN));
    localhost.sin_family = AF_INET;
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

	checkClientActivity();

	errorCode = select((unsigned int)requestSocket + MAXCLIENTS, &actionFlag, NULL, NULL, 0);

	if(errorCode == SOCKET_ERROR)
	{
		throw "\nFAIL: Something went wrong with SELECT!";
	}

	if(FD_ISSET(requestSocket, &actionFlag))
	{
		try
		{
			acceptClient();
		}
		catch(const char* failure)
		{
			printf("%s", failure);
		}
	}

	for(int i = 0;i < MAXCLIENTS;i++)
	{
		if(clients[i] != INVALID_SOCKET && FD_ISSET(clients[i], &actionFlag))
		{
			try
			{
				//TODO: analyse commands from clients

				receive(&clients[i]);

				if(!strcmp(clientMessage, "offline"))
				{
					printf("\nClient %d went offline!\n", clients[i]);
					setClientToOffline(&clients[i]);
				}

				printf("\nClient said: %c\n", *clientMessage);

				try
				{
					sendToClient(&clients[i], clientMessage);
				}
				catch(const char* failure)
				{
					printf("%s", failure);
				}
			}
			catch(const char* failure)
			{
				printf("%s", failure);
				setClientToOffline(&clients[i]);
			}
		}
	}
}

void TwitterServer::acceptClient(void)
{
	for(int i = 0;i < MAXCLIENTS;i++)
	{
		if(clients[i] == INVALID_SOCKET)
		{
			clients[i] = accept(requestSocket, NULL, NULL);

			if(clients[i] == INVALID_SOCKET)
			{
				throw "\nFAIL: Couldn't connect client!";
			}
			else
			{
				printf("\nSUCCESS: Connected with client!");
			}

			return;
		}
	}
}

void TwitterServer::sendToClient(const SOCKET* client, const char* message)
{
    //TODO: check if whole message was sent

    int errorCode;

    errorCode = send(*client, message, BUFFERSIZE, 0);

    if(errorCode == SOCKET_ERROR)
    {
        throw "\nFAIL: Unable to send message!";
    }
}

void TwitterServer::receive(SOCKET* clientSocket)
{
    //TODO: check if we received the whole message

    int errorCode;

    errorCode = recv(*clientSocket, clientMessage, BUFFERSIZE, 0);

    if(errorCode == 0)
    {
        throw "\nFAIL: Lost connection to client!";
    }
    else if(errorCode == SOCKET_ERROR)
    {
        throw "\nClient went offline!";
    }
}

void TwitterServer::setClientToOffline(SOCKET* clientSocket)
{
	closesocket(*clientSocket);
	*clientSocket = INVALID_SOCKET;

	FD_ZERO(&actionFlag);
	FD_SET(requestSocket, &actionFlag);
}

void TwitterServer::checkClientActivity(void)
{
	for(int i = 0; i < MAXCLIENTS;i++)
	{
		if(clients[i] != INVALID_SOCKET)
		{
			FD_SET(clients[i], &actionFlag);
		}
	}
}

void TwitterServer::closeRequestSocket(void) const
{
    closesocket(requestSocket);
    WSACleanup();
}

void TwitterServer::closeSockets(void)
{
    closesocket(requestSocket);
	FD_ZERO(&actionFlag);

	for(int i = 0;i < MAXCLIENTS;i++)
	{
		closesocket(clients[i]);
	}
}