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
		socketCreator.createSocket(&requestSocket, AF_INET);	// set up the listening socket
		FD_ZERO(&actionFlag);									// set status flag to zero
		FD_SET(requestSocket, &actionFlag);						// set up status flag for listen socket

		for(int i = 0;i < MAXCLIENTS;i++)						// set all client socktets to invalid
			clients[i] = INVALID_SOCKET;
	}
	catch(string failure)										// if something fails print error
	{
		printf("%s", failure.c_str());
	}
}

TwitterServer::~TwitterServer(void)		// destructor
{
	closeSockets();						// close all socktes
	WSACleanup();						// clean
}

void TwitterServer::configServer(const unsigned short port)
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
		printf("\nSUCCESS: Bound socket to port %u!", port);

	printf("\nStarting listen mode...");

	errorCode = listen(requestSocket, 10);		// start listening, get error code

	if(errorCode == SOCKET_ERROR)
	{
		closeRequestSocket();

		throw exceptionTexter("\nFAIL: Unable to initiate listen mode! (Error Code: ", errorCode);
	}
	else
		printf("\nSUCCESS: Listen mode started!");
}

void TwitterServer::clientListener(void)
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
			acceptClient(errorCode);	// accept client
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

				makeSubString(command);		// divide received string

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

void TwitterServer::commandInterpreter(char* command[], const SOCKET clientSocket)
{
	string messageForClient;

	if(!strcmp(command[0], ":login"))				// interpret commands and do related action
	{
		logInTweeter(clientSocket, command[1]);
	}
	else if(loggedIn(clientSocket))					// those commands only work when logged in
	{
		if(!strcmp(command[0], ":logout"))
		{
			logOutTweeter(clientSocket);
		}
		else if(!strcmp(command[0], ":whoami?"))
		{
			sendNameOfTweeter(clientSocket);
		}
		else if(!strcmp(command[0], ":follow"))
		{
			followTweeter(clientSocket, command[1]);
		}
		else if(!strcmp(command[0], ":pull"))
		{
			getAllTweets(clientSocket);
		}
		else if(!strcmp(command[0], ":tweet"))
		{
			newTweet(clientSocket, command[1]);
		}
		else
		{
			sendToClient(&clientSocket, "twitter: command not found!\n");
			sendToClient(&clientSocket, "ETX");
		}
	}
	else
	{
		try
		{
			sendToClient(&clientSocket, "twitter: you have to login!\n");
			sendToClient(&clientSocket, "ETX");
		}
		catch(string failure)
		{
			printf("%s", failure.c_str());
		}
	}
}

void TwitterServer::getAllTweets(const SOCKET clientSocket)
{
	getOwnTweets(clientSocket);
	getOtherTweets(clientSocket);
}

void TwitterServer::getOtherTweets(const SOCKET clientSocket)
{
	string followedTweeter;
	multimap<string, string>::iterator it;
	pair<multimap<string, string>::iterator, multimap<string,string>::iterator> otherTweets;

	otherTweets = abonnement.equal_range(tweeter[clientSocket]);

	if(otherTweets.first->first == tweeter[clientSocket])
	{
		for(it = otherTweets.first;it != otherTweets.second;++it)
		{
			getOwnTweets(getSocketByTweeter(it->second));
		}
	}
}

void TwitterServer::getOwnTweets(const SOCKET clientSocket)
{
	multimap<string, string>::iterator it;

	string formattedTweet;

	try
	{
		for(it = tweet.begin();it != tweet.end();it++)
		{
			if(it->first == tweeter[clientSocket])
			{
				formattedTweet = tweeter[clientSocket] + ": " + it->second + "\n";
				sendToClient(&clientSocket, formattedTweet.c_str());
			}
		}
	}
	catch(string failure)
	{
		printf("%s", failure.c_str());
	}

//	sendToClient(&clientSocket, "ETX");
}

SOCKET TwitterServer::getSocketByTweeter(const string name)
{
	map<SOCKET, string>::iterator it = tweeter.begin();

	while(it != tweeter.end())
	{
		if(it->second == name)
			return it->first;		// returns socket if tweeter was found

		it++;
	}
	return INVALID_SOCKET;
}

void TwitterServer::followTweeter(const SOCKET follower, const string followedTweeter)
{
	string messageForClient;
	SOCKET tweeterSocket = getSocketByTweeter(followedTweeter);		// find out socket of followed tweeter

	if(tweeterSocket != INVALID_SOCKET && loggedIn(tweeterSocket))	// if that tweeter is logged in and valid
	{
		abonnement.insert( pair<string, string>(tweeter[follower], followedTweeter) );		// add abonement

		messageForClient = tweeter[follower] + " is now following " + followedTweeter + "!\n";		// success
	}
	else
		messageForClient = "the tweeter you want to follow, doesn't exist or is not logged in!\n";		// fail

	try
	{
		sendToClient(&follower, messageForClient.c_str());
		sendToClient(&follower, "ETX");
	}
	catch(string failure)
	{
		printf("%s", failure.c_str());
	}
}

bool TwitterServer::loggedIn(const SOCKET clientSocket)
{
	if(tweeter.find(clientSocket) == tweeter.end())		// check if a tweeter is logged in
		return false;

	else
		return true;
}

void TwitterServer::newTweet(const SOCKET clientSocket, const string text)
{
	multimap<string, string>::iterator it = abonnement.begin();
	string messageForClient;

	try
	{
		tweet.insert( pair<string, string>(tweeter[clientSocket], text) );		// save tweet

		messageForClient = tweeter[clientSocket] + ": " + text + "\n";

		sendToClient(&clientSocket, messageForClient.c_str());
		sendToClient(&clientSocket, "ETX");
	}
	catch(string failure)
	{
		printf("%s", failure.c_str());
	}
}

void TwitterServer::makeSubString(char* command[])
{
	unsigned int space = strcspn(clientMessage, " ");	// get ascii code of space

	command[0] = new char[space];
	strncpy(command[0], clientMessage, space);
	command[0][space] = '\0';

	if(space <= strlen(clientMessage))
	{
		command[1] = new char[strlen(clientMessage) - space];

		for(unsigned int pos = space + 1;pos < strlen(clientMessage);pos++)
		{
			command[1][pos - space - 1] = clientMessage[pos];
		}

		command[1][strlen(clientMessage) - space - 1] = '\0';
	}
	else
		command[1] = NULL;
}

void TwitterServer::logOutTweeter(const SOCKET clientSocket)
{
	tweeter.erase(clientSocket);	// erase tweeter socket from map

	try
	{
		sendToClient(&clientSocket, "twitter: oooh, the bird flew away!\n");
		sendToClient(&clientSocket, "ETX");
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
		sendToClient(&clientSocket, tweeter[clientSocket].c_str());		// whoami?
		sendToClient(&clientSocket, "ETX");
	}
	catch(string failure)
	{
		printf("%s", failure.c_str());
	}
}

void TwitterServer::logInTweeter(const SOCKET clientSocket, const string name)
{
	tweeter[clientSocket] = name;	// create tweeter

	try
	{
		sendToClient(&clientSocket, "twitter: hello bird!\n");
		sendToClient(&clientSocket, "ETX");
	}
	catch(string failure)
	{
		printf("%s", failure.c_str());
	}
}

void TwitterServer::acceptClient(int numberOfClients)
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

void TwitterServer::sendToClient(const SOCKET* client, const char* message)
{
	//TODO: check if whole message was sent

	int errorCode;		// sends a message to a client/tweeter

	errorCode = send(*client, message, BUFFERSIZE, 0);

	if(errorCode == SOCKET_ERROR)
		throw exceptionTexter("\nFAIL: Unable to send message! (Error Code: ", errorCode);
}

void TwitterServer::receive(const SOCKET* clientSocket)
{
	//TODO: check if we received the whole message

	int errorCode;

	errorCode = recv(*clientSocket, clientMessage, BUFFERSIZE, 0); // receive a message

	if(errorCode == 0)
		throw exceptionTexter("\nFAIL: Lost connection to client! (Error Code: ", errorCode);

	else if(errorCode == SOCKET_ERROR || !strcmp("offline", clientMessage))
		throw exceptionTexter("\nClient went offline! (socket ", *clientSocket);
}

void TwitterServer::setClientToOffline(SOCKET* clientSocket)
{
	closesocket(*clientSocket);			// close the client socket
	*clientSocket = INVALID_SOCKET;		// and set to invalid

	FD_ZERO(&actionFlag);				// set the status flag to zero
	FD_SET(requestSocket, &actionFlag);	// set the status flag for the request socket to zero
}

void TwitterServer::setClientToOnline(void)
{
	for(int i = 0; i < MAXCLIENTS;i++)	// go through all open sockets
	{
		if(clients[i] != INVALID_SOCKET)	// if not invalid
			FD_SET(clients[i], &actionFlag);	// set status to active
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

	for(int i = 0;i < MAXCLIENTS;i++)	// close all open sockets
	{
		closesocket(clients[i]);
	}
}