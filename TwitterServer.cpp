/*###############################
# FH Salzburg                   #
# WS2010 MMT-B2009              #
# Multimediale Netzwerke        #
# Uebungsprojekt                #
# Fritsch Andreas, Hanli Ismail #
# Sun, 28.11.2010 22:00         #
###############################*/

#include "TwitterServer.h"

TwitterServer::TwitterServer(const unsigned short port) : MultiplexingServer() {}

TwitterServer::~TwitterServer(void) {}

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
	unsigned int space = strcspn(clientMessage, " ");

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

	if(!strcmp(command[0], ":i"))				// interpret commands and do related action
	{
		logInTweeter(clientSocket, command[1]);
	}
	else if(loggedIn(clientSocket))					// those commands only work when logged in
	{
		if(!strcmp(command[0], ":o"))
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
//		else if(!strcmp(command[0], ":p"))
//		{
//			getAllTweets(clientSocket);
//		}
		else if(!strcmp(command[0], ":t"))
		{
			newTweet(clientSocket, command[1]);
		}
		else
		{
			sendToClient(&clientSocket, "twitter: command not found!\n");
		}
	}
	else
	{
		try
		{
			sendToClient(&clientSocket, "twitter: you have to login!\n");
		}
		catch(string failure)
		{
			printf("%s", failure.c_str());
		}
	}

	sendToClient(&clientSocket, "ETX");
}

//void TwitterServer::getAllTweets(const SOCKET clientSocket)
//{
//	getOwnTweets(clientSocket);
//	getOtherTweets(clientSocket);
//}
//
//void TwitterServer::getOtherTweets(const SOCKET clientSocket)
//{
//	string followedTweeter;
//	multimap<string, string>::iterator it;
//	pair<multimap<string, string>::iterator, multimap<string,string>::iterator> otherTweets;
//
//	if(!abonnement.empty())
//	{
//		otherTweets = abonnement.equal_range(tweeter[clientSocket]);
//
//		if(otherTweets.first->first == tweeter[clientSocket] || otherTweets.first == abonnement.end())
//		{
//			for(it = otherTweets.first;it != otherTweets.second;++it)
//			{
//				getOwnTweets(getSocketByTweeter(it->second));
//			}
//		}
//	}
//}

void TwitterServer::sendToFollowers(const SOCKET clientSocket, const string message)
{
	SOCKET followerSocket;
	multimap<string, string>::iterator it;
	pair<multimap<string, string>::iterator, multimap<string,string>::iterator> followers;

	if(!abonnement.empty())
	{
		followers = abonnement.equal_range(tweeter[clientSocket]);

		if(followers.first->first == tweeter[clientSocket] || followers.first != abonnement.end())
		{
			for(it = followers.first;it != followers.second;++it)
			{
				followerSocket = getSocketByTweeter(it->second);
				sendToClient(&followerSocket, message.c_str());
			}
		}
	}
}

//void TwitterServer::getOwnTweets(const SOCKET clientSocket)
//{
//	multimap<string, string>::iterator it;
//
//	string formattedTweet;
//
//	try
//	{
//		for(it = tweet.begin();it != tweet.end();it++)
//		{
//			if(it->first == tweeter[clientSocket])
//			{
//				formattedTweet = tweeter[clientSocket] + ": " + it->second + "\n";
//				sendToClient(&clientSocket, formattedTweet.c_str());
//			}
//		}
//	}
//	catch(string failure)
//	{
//		printf("%s", failure.c_str());
//	}
//}

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
		abonnement.insert( pair<string, string>(followedTweeter, tweeter[follower]) );		// add abonement

		messageForClient = "twitter: " + tweeter[follower] + " is now following " + followedTweeter + "!\n";		// success
	}
	else
		messageForClient = "twitter: the tweeter you want to follow, doesn't exist or is not logged in!\n";		// fail

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
	if(tweeter.find(clientSocket) == tweeter.end())		// check if a tweeter is logged in
		return false;

	else
		return true;
}

void TwitterServer::newTweet(const SOCKET clientSocket, const string text)
{
	multimap<string, string>::iterator it = abonnement.begin();
	string messageForClient;
	ofstream logFile(LOGFILEPATH, ios::app);

	try
	{
		tweet.insert( pair<string, string>(tweeter[clientSocket], text) );

		messageForClient = tweeter[clientSocket] + ": " + text + "\n";

		logFile.write(messageForClient.c_str(), messageForClient.length());

		sendToClient(&clientSocket, messageForClient.c_str());

		sendToFollowers(clientSocket, messageForClient);
	}
	catch(string failure)
	{
		printf("%s", failure.c_str());
	}

	logFile.close();
}

void TwitterServer::logOutTweeter(const SOCKET clientSocket)
{
	tweeter.erase(clientSocket);

	try
	{
		sendToClient(&clientSocket, "twitter: oooh, the bird flew away!\n");
	}
	catch(string failure)
	{
		printf("%s", failure.c_str());
	}
}

void TwitterServer::sendNameOfTweeter(const SOCKET clientSocket)
{
	string messageForClient;

	messageForClient = "twitter: " + tweeter[clientSocket] + "\n";

	try
	{
		sendToClient(&clientSocket, messageForClient.c_str());		// whoami?
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
		sendToClient(&clientSocket, "twitter: hello bird!\n");
	}
	catch(string failure)
	{
		printf("%s", failure.c_str());
	}
}