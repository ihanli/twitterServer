/*###############################
# FH Salzburg                   #
# WS2010 MMT-B2009              #
# Multimediale Netzwerke        #
# Uebungsprojekt                #
# Fritsch Andreas, Hanli Ismail #
# Sun, 28.11.2010 22:00         #
###############################*/

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

	errorCode = select(1000, &actionFlag, NULL, NULL, 0);

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
				//FIXME: escaping character for command

				receive(&clients[i]);

				makeSubString(command);

				commandInterpreter(command, clients[i]);

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

void TwitterServer::commandInterpreter(char* command[], const SOCKET clientSocket)
{
	string messageForClient;

	if(!strcmp(command[0], "login"))
	{
		logInTweeter(clientSocket, command[1]);
	}
	else if(loggedIn(clientSocket))
	{
		if(!strcmp(command[0], "logout"))
		{
			logOutTweeter(clientSocket);
		}
		else if(!strcmp(command[0], "whoami?"))
		{
			sendNameOfTweeter(clientSocket);
		}
		else if(!strcmp(command[0], "follow"))
		{
			followTweeter(clientSocket, command[1]);
		}
		else
		{
			newTweet(clientSocket, clientMessage);
		}
	}
	else
	{
		try
		{
			sendToClient(&clientSocket, "you have to login!");
		}
		catch(string failure)
		{
			printf("%s", failure.c_str());
		}
	}
}

SOCKET TwitterServer::getSocketByTweeter(const string name)
{
	map<SOCKET, string>::iterator it = tweeter.begin();

	while(it != tweeter.end())
	{
		if(it->second == name)
		{
			return it->first;
		}

		it++;
	}

	return INVALID_SOCKET;
}

void TwitterServer::followTweeter(const SOCKET follower, const string followedTweeter)
{
	string messageForClient;
	SOCKET tweeterSocket = getSocketByTweeter(followedTweeter);

	if(tweeterSocket != INVALID_SOCKET && loggedIn(tweeterSocket))
	{
		abonnement.insert( pair<SOCKET, SOCKET>(follower, tweeterSocket) );

		messageForClient = tweeter[follower] + " is now following " + tweeter[tweeterSocket] + "!";
	}
	else
	{
		messageForClient = "the tweeter you want to follow, doesn't exist or is not logged in!";
	}

	try
	{
		sendToClient(&follower, messageForClient.c_str());
	}
	catch(string failure)
	{
		printf("%s", failure.c_str());
	}
}

bool TwitterServer::loggedIn(const SOCKET clientSocket)
{
	if(tweeter.find(clientSocket) == tweeter.end())
	{
		return false;
	}
	else
	{
		return true;
	}
}

void TwitterServer::newTweet(const SOCKET clientSocket, const string text)
{
	multimap<SOCKET, SOCKET>::iterator it = abonnement.begin();

	try
	{
		tweet.insert( pair<string, string>(tweeter[clientSocket], text) );

		sendToClient(&clientSocket, tweet.find(tweeter[clientSocket])->second.c_str());

		while(it != abonnement.end())
		{
			if(it->second == clientSocket)
			{
				sendToClient(&it->first, tweet.find(tweeter[clientSocket])->second.c_str());
			}

			it++;
		}
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
	else
	{
		command[1] = NULL;
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
		for(int j = 0;j < MAXCLIENTS;j++)
		{
			if(clients[j] == INVALID_SOCKET)
			{
				clients[j] = accept(requestSocket, NULL, NULL);

				if(clients[j] == INVALID_SOCKET)
				{
					throw exceptionTexter("\nFAIL: Couldn't connect client! (socket ", clients[j]);
				}
				else
				{
					printf("\nSUCCESS: Connected with client! (socket %d)", clients[j]);
					j = MAXCLIENTS;
					i = numberOfClients;
				}
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