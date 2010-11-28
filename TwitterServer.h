/*###############################
# FH Salzburg                   #
# WS2010 MMT-B2009              #
# Multimediale Netzwerke        #
# Uebungsprojekt                #
# Fritsch Andreas, Hanli Ismail #
# Sun, 28.11.2010 22:00         #
###############################*/

#ifndef TWITTERSERVER_H_INCLUDED
#define TWITTERSERVER_H_INCLUDED

#include <map>
#include <windows.h>
#include "SocketBase.h"

#define MAXCLIENTS 10
#define BUFFERSIZE 140

using namespace std;

class TwitterServer
{
	public:
		TwitterServer(const unsigned short port = 3000);
		~TwitterServer();
		void configServer(const unsigned short port = 3000);
		void clientListener(void);

	private:
		SOCKET requestSocket;
		map<unsigned int, SOCKET> clients;
		map<SOCKET, string> tweeter;
		multimap<string, string> tweet;
		multimap<SOCKET, SOCKET> abonnement;
		SOCKADDR_IN localhost;
		FD_SET actionFlag;
		SocketBase socketCreator;
		char* clientMessage;

		void closeRequestSocket(void) const;
		void closeSockets(void);
		void acceptClient(int numberOfClients);
		void sendToClient(const SOCKET* client, const char* message);
		void receive(const SOCKET* clientSocket);

		//Helper methods

		SOCKET getSocketByTweeter(const string name);
		void followTweeter(const SOCKET follower, const string followedTweeter);
		void commandInterpreter(char* command[], const SOCKET clientSocket);
		bool loggedIn(const SOCKET clientSocket);
		void newTweet(const SOCKET clientSocket, const string text);
		void makeSubString(char* command[]);
		void logInTweeter(const SOCKET clientSocket, const string name);
		void logOutTweeter(const SOCKET clientSocket);
		void sendNameOfTweeter(const SOCKET clientSocket);
		void setClientToOnline(void);
		void setClientToOffline(SOCKET* clientSocket);
};

#endif // TWITTERSERVER_H_INCLUDED
