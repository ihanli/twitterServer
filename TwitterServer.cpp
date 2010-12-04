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
			   MultiplexingServer()
{
//	try
//	{
//		socketCreator.createSocket(&requestSocket, AF_INET);	// set up the listening socket
//		FD_ZERO(&actionFlag);									// set status flag to zero
//		FD_SET(requestSocket, &actionFlag);						// set up status flag for listen socket
//
//		for(int i = 0;i < MAXCLIENTS;i++)						// set all client socktets to invalid
//			clients[i] = INVALID_SOCKET;
//	}
//	catch(string failure)										// if something fails print error
//	{
//		printf("%s", failure.c_str());
//	}
}

TwitterServer::~TwitterServer(void)		// destructor
{
//	closeSockets();						// close all socktes
//	WSACleanup();						// clean
}

void TwitterServer::setupServer(const unsigned short port)
{
	configServer(port);
}

void TwitterServer::run(void)
{
	clientListener();
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

void TwitterServer::commandInterpreter(char* command[], const SOCKET clientSocket)
{
	string messageForClient;

	makeSubString(command);		// divide received string

	if(!strcmp(command[0], ":in"))				// interpret commands and do related action
	{
		logInTweeter(clientSocket, command[1]);
	}
	else if(loggedIn(clientSocket))					// those commands only work when logged in
	{
		if(!strcmp(command[0], ":out"))
		{
			logOutTweeter(clientSocket);
		}
		else if(!strcmp(command[0], ":whoami?"))
		{
			sendNameOfTweeter(clientSocket);
		}
		else if(!strcmp(command[0], ":f"))
		{
			followTweeter(clientSocket, command[1]);
		}
		else if(!strcmp(command[0], ":p"))
		{
			getAllTweets(clientSocket);
		}
		else if(!strcmp(command[0], ":t"))
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
	sendToClient(&clientSocket, "ETX");
}

void TwitterServer::getOtherTweets(const SOCKET clientSocket)
{
	string followedTweeter;
	multimap<string, string>::iterator it;
	pair<multimap<string, string>::iterator, multimap<string,string>::iterator> otherTweets;

	if(!abonnement.empty())
	{
		otherTweets = abonnement.equal_range(tweeter[clientSocket]);

		if(otherTweets.first->first == tweeter[clientSocket] || otherTweets.first == abonnement.end())
		{
			for(it = otherTweets.first;it != otherTweets.second;++it)
			{
				getOwnTweets(getSocketByTweeter(it->second));
			}
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