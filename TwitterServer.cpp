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

void TwitterServer::makeSubString(char* command[])			// ??????????
{
	unsigned int space = strcspn(clientMessage, " ");

	command[0] = new char[space];
	strncpy(command[0], clientMessage, space);
	command[0][space] = '\0';

	if(space <= strlen(clientMessage))
	{
		command[1] = new char[strlen(clientMessage) - space];

		for(unsigned int pos = space + 1;pos < strlen(clientMessage);pos++)
			command[1][pos - space - 1] = clientMessage[pos];

		command[1][strlen(clientMessage) - space - 1] = '\0';
	}
	else
		command[1] = NULL;
}

void TwitterServer::commandInterpreter(char* command[], const SOCKET clientSocket)
{
	string messageForClient;

	makeSubString(command);						// divide the received string

	if(!strcmp(command[0], ":i"))				// interpret commands and do related action
	{
		logInTweeter(clientSocket, command[1]);
	}
	else if(loggedIn(clientSocket))				// those commands only work when logged in
	{
		if(!strcmp(command[0], ":o"))
			logOutTweeter(clientSocket);

		else if(!strcmp(command[0], ":me"))
			sendNameOfTweeter(clientSocket);

		else if(!strcmp(command[0], ":f"))
			followTweeter(clientSocket, command[1]);

		else if(!strcmp(command[0], ":p"))
			getAllTweets(clientSocket);

		else if(!strcmp(command[0], ":t"))
			newTweet(clientSocket, command[1]);

		else
			sendToClient(&clientSocket, "twitter: command not found!\n");
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

	sendToClient(&clientSocket, "ETX");		// send final message "end of text"
}

void TwitterServer::getAllTweets(const SOCKET clientSocket)
{
	getOwnTweets(tweeter[clientSocket], clientSocket);
	getOtherTweets(clientSocket);
}

void TwitterServer::getOtherTweets(const SOCKET clientSocket)
{
	string followedTweeter;
	multimap<string, string>::iterator it;

	if(!abonnement.empty())			// are there any followers?
	{
		for(it = abonnement.begin();it != abonnement.end();it++)
		{
			if(it->second == tweeter[clientSocket])			// ????????
				getOwnTweets(it->first, clientSocket);
		}
	}
}

void TwitterServer::getOwnTweets(const string nameOfTweeter, const SOCKET clientSocket)
{
	multimap<string, string>::iterator it;

	string formattedTweet;

	try
	{
		for(it = tweet.begin();it != tweet.end();it++)		// iterate through all tweets
		{
			if(it->first == nameOfTweeter)					// if the tweeter name was found
			{
				formattedTweet = nameOfTweeter + ": " + it->second + "\n";		// construct the output
				sendToClient(&clientSocket, formattedTweet.c_str());			// send it to the client
			}
		}
	}
	catch(string failure)
	{
		printf("%s", failure.c_str());
	}
}

void TwitterServer::sendToFollowers(const SOCKET clientSocket, const string message)
{
	SOCKET followerSocket;
	multimap<string, string>::iterator it;
	pair<multimap<string, string>::iterator, multimap<string,string>::iterator> followers;

	if(!abonnement.empty())			// are there any followers?
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

SOCKET TwitterServer::getSocketByTweeter(const string name)
{
	map<SOCKET, string>::iterator it = tweeter.begin();

	while(it != tweeter.end())
	{
		if(it->second == name)
			return it->first;		// return the socket if tweeter was found

		it++;
	}
	return INVALID_SOCKET;			// else return invalid
}

void TwitterServer::followTweeter(const SOCKET follower, const string followedTweeter)
{
	string messageForClient;
	SOCKET tweeterSocket = getSocketByTweeter(followedTweeter);		// find out socket of followed tweeter

	if(tweeterSocket != INVALID_SOCKET && loggedIn(tweeterSocket))	// if that tweeter is logged in and valid
	{
		abonnement.insert( pair<string, string>(followedTweeter, tweeter[follower]) );		// add an abonement

		messageForClient = "twitter: " + tweeter[follower] + " is now following " + followedTweeter + "!\n";		// on success construct output
	}
	else
		messageForClient = "twitter: the tweeter you want to follow, doesn't exist or is not logged in!\n";		// on fail

	try
	{
		sendToClient(&follower, messageForClient.c_str());		// send the result to the client
	}
	catch(string failure)
	{
		printf("%s", failure.c_str());
	}
}

bool TwitterServer::loggedIn(const SOCKET clientSocket)			// check if a tweeter is logged in
{
	if(tweeter.find(clientSocket) == tweeter.end() || tweeter[clientSocket] == "OFFLINE")	// not logged in if tweeter not found or offline
		return false;

	else
		return true;
}

void TwitterServer::newTweet(const SOCKET clientSocket, const string text)
{
	multimap<string, string>::iterator it = abonnement.begin();
	string messageForClient;
	ofstream logFile(LOGFILEPATH, ios::app);	// output for message file

	try
	{
		tweet.insert( pair<string, string>(tweeter[clientSocket], text) );		// insert tweet as pair in multimap

		messageForClient = tweeter[clientSocket] + ": " + text + "\n";			// construct message for client

		logFile.write(messageForClient.c_str(), messageForClient.length());		// write to logfile

		sendToClient(&clientSocket, messageForClient.c_str());					// send feedback to client

		sendToFollowers(clientSocket, messageForClient);						// send new tweet to all followers
	}
	catch(string failure)
	{
		printf("%s", failure.c_str());
	}

	logFile.close();
}

void TwitterServer::logOutTweeter(const SOCKET clientSocket)
{
	string messageForClient = "twitter: " + tweeter[clientSocket] + " just logged out!\n";
	tweeter[clientSocket] = "OFFLINE";		// set status to offline

	try
	{
		sendToClient(&clientSocket, messageForClient.c_str());		// send feedback to client
	}
	catch(string failure)
	{
		printf("%s", failure.c_str());
	}
}

void TwitterServer::sendNameOfTweeter(const SOCKET clientSocket)
{
	string messageForClient;

	messageForClient = "twitter: you are " + tweeter[clientSocket] + "\n";	// construct message for client

	try
	{
		sendToClient(&clientSocket, messageForClient.c_str());				// and send it
	}
	catch(string failure)
	{
		printf("%s", failure.c_str());
	}
}

void TwitterServer::logInTweeter(const SOCKET clientSocket, const string name)
{
	string messageForClient = "twitter: " + name + " logged in!\n";			// construct message for client

	tweeter[clientSocket] = name;											// set to online

	try
	{
		sendToClient(&clientSocket, messageForClient.c_str());				// send feedback
	}
	catch(string failure)
	{
		printf("%s", failure.c_str());
	}
}