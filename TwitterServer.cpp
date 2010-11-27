#include "TwitterServer.h"

TwitterServer::TwitterServer(const unsigned short port) :
               socketCreator()
{
    try
    {
        socketCreator.createSocket(&requestSocket, AF_INET);
        FD_ZERO(&actionFlag);
		FD_SET(requestSocket, &actionFlag);

        for(unsigned int i = 0;i < MAXCLIENTS;i++)
		{
			clients[i] = INVALID_SOCKET;
		}

//		clientMessage = NULL;
    }
    catch(string failure)
    {
        printf("%s", failure.c_str());
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

        throw exceptionTexter("\nFAIL: Unable to bind socket! (Error Code: ", errorCode);
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

        throw exceptionTexter("\nFAIL: Unable to initiate listen mode! (Error Code: ", errorCode);
    }
    else
    {
        printf("\nSUCCESS: Listen mode started!");
    }
}

void TwitterServer::clientListener(void)
{
	int errorCode;

	setClientToOnline();

	errorCode = select((unsigned int)requestSocket + MAXCLIENTS, &actionFlag, NULL, NULL, 0);

	if(errorCode == SOCKET_ERROR)
	{
		throw exceptionTexter("\nFAIL: Something went wrong with SELECT! (Error Code: ", errorCode);
	}

	if(FD_ISSET(requestSocket, &actionFlag))
	{
		try
		{
			acceptClient(errorCode);
		}
		catch(string failure)
		{
			printf("%s", failure.c_str());
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

				printf("\nClient said: %s\n", clientMessage);

				try
				{
					sendToClient(&clients[i], clientMessage);
				}
				catch(string failure)
				{
					printf("%s", failure.c_str());
				}
			}
			catch(string failure)
			{
				printf("%s", failure.c_str());
				setClientToOffline(&clients[i]);
			}
		}
	}
}

void TwitterServer::acceptClient(int numberOfClients)
{
	for(int i = 0;i < numberOfClients;i++)
	{
		if(clients[i] == INVALID_SOCKET)
		{
			clients[i] = accept(requestSocket, NULL, NULL);

			if(clients[i] == INVALID_SOCKET)
			{
				throw exceptionTexter("\nFAIL: Couldn't connect client! (socket ", clients[i]);
			}
			else
			{
				printf("\nSUCCESS: Connected with client! (socket %d)", clients[i]);
			}
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
        throw exceptionTexter("\nFAIL: Unable to send message! (Error Code: ", errorCode);
    }
}

void TwitterServer::receive(SOCKET* clientSocket)
{
    //TODO: check if we received the whole message

    int errorCode;

    errorCode = recv(*clientSocket, clientMessage, BUFFERSIZE, 0);

	clientMessage[errorCode] = '\0';

    if(errorCode == 0)
    {
        throw exceptionTexter("\nFAIL: Lost connection to client! (Error Code: ", errorCode);
    }
    else if(errorCode == SOCKET_ERROR || !strcmp("logout", clientMessage))
    {
        throw exceptionTexter("\nClient went offline! (socket ", *clientSocket);
    }
}

void TwitterServer::setClientToOffline(SOCKET* clientSocket)
{
	closesocket(*clientSocket);
	*clientSocket = INVALID_SOCKET;

	FD_ZERO(&actionFlag);
	FD_SET(requestSocket, &actionFlag);
}

void TwitterServer::setClientToOnline(void)
{
	for(int i = 0; i < MAXCLIENTS;i++)
	{
		if(clients[i] != INVALID_SOCKET)
		{
			FD_SET(clients[i], &actionFlag);
		}
	}
}

void TwitterServer::sendLogo(const SOCKET* clientSocket)
{
	try
	{
		sendToClient(clientSocket, "\n............. ................. ........");
		sendToClient(clientSocket, "\n.............................IIII.......");
		sendToClient(clientSocket, "\n..................ZZ~......?IIIIII?.....");
		sendToClient(clientSocket, "\n............:.I7=?+Z$$$7ZI7IIIII..,I:...");
		sendToClient(clientSocket, "\n............,7+?IIII$$$$IIIIIIII,.III7,.");
		sendToClient(clientSocket, "\n...............+?IIIIIIIIIIIIIIIIIII,...");
		sendToClient(clientSocket, "\n..................I7IIIIIIIIIIIIIIII....");
		sendToClient(clientSocket, "\n..................IIIIIIIIIII?+:,=I.....");
		sendToClient(clientSocket, "\n...............=?IIIIIIIIIII:::::::.....");
		sendToClient(clientSocket, "\n.,=........,=IIIIIIIIIIIII:::::,:,......");
		sendToClient(clientSocket, "\n....~?I??IIIIIIIIIIIIIIII:::,:::........");
		sendToClient(clientSocket, "\n.......+?IIIIIIIIIIIIIII:::::::.. ......");
		sendToClient(clientSocket, "\n..........~IIIIIIIIIIIII:::::...........");
		sendToClient(clientSocket, "\n................~?IIIIII=...............");
		sendToClient(clientSocket, "\n........................................");
		sendToClient(clientSocket, "\n............. ................. ........");
	}
	catch(string failure)
	{
		printf("%s", failure.c_str());
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