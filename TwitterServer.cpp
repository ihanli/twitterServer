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
	char* command[2];

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
				//FIXME: tweeter is not allowed to tweet, if not logged in
				//FIXME: escaping character for command

				receive(&clients[i]);

				makeSubString(command);

				if(!strcmp(command[0], "login"))
				{
					logInTweeter(clients[i], command[1]);
				}
				else if(!strcmp(command[0], "logout"))
				{
					logOutTweeter(clients[i]);
				}
				else if(!strcmp(command[0], "whoami?"))
				{
					sendNameOfTweeter(clients[i]);
				}
				else
				{
					newTweet(clients[i], clientMessage);
				}

				delete [] command[1];
				delete [] command[0];
			}
			catch(const char* failure)
			{
				printf("%s", failure);
				setClientToOffline(&clients[i]);
			}
		}
	}
}

void TwitterServer::newTweet(const SOCKET clientSocket, const string text)
{
	tweet.insert( pair<string, string>(tweeter[clientSocket], text) );

	try
	{
		sendToClient(&clientSocket, tweet.find(tweeter[clientSocket])->second.c_str());
	}
	catch(string failure)
	{
		printf("%s", failure.c_str());
	}
}

void TwitterServer::makeSubString(char* command[])
{
	unsigned int space;

	space = strcspn(clientMessage, " ");

	command[0] = new char[space];
	strncpy(command[0], clientMessage, space);
	command[0][space] = '\0';

	if(space <= strlen(clientMessage))
	{
		command[1] = new char[strlen(clientMessage) - space];

		for(unsigned int pos = space;pos < strlen(clientMessage);pos++)
		{
			command[1][pos - space] = clientMessage[pos];
		}

		//command[1][strlen(clientMessage)] = '\0';
	}
}

void TwitterServer::logOutTweeter(const SOCKET clientSocket)
{
	tweeter.erase(clientSocket);

	try
	{
		sendToClient(&clientSocket, "oooh, the bird flew away!");
	}
	catch(string failure)
	{
		printf("%s", failure.c_str());
	}
}

void TwitterServer::sendNameOfTweeter(const SOCKET clientSocket)
{
	try
	{
		sendToClient(&clientSocket, tweeter[clientSocket].c_str());
	}
	catch(string failure)
	{
		printf("%s", failure.c_str());
	}
}

void TwitterServer::logInTweeter(const SOCKET clientSocket, const string name)
{
	tweeter[clientSocket] = name;

	try
	{
		sendToClient(&clientSocket, "hello bird!");
	}
	catch(string failure)
	{
		printf("%s", failure.c_str());
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

void TwitterServer::receive(const SOCKET* clientSocket)
{
    //TODO: check if we received the whole message

    int errorCode;

    errorCode = recv(*clientSocket, clientMessage, BUFFERSIZE, 0);

    if(errorCode == 0)
    {
        throw exceptionTexter("\nFAIL: Lost connection to client! (Error Code: ", errorCode);
    }
    else if(errorCode == SOCKET_ERROR || !strcmp("offline", clientMessage))
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