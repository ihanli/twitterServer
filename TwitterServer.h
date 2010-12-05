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

#include <fstream>
#include "MultiplexingServer.h"

#define LOGFILEPATH "logfile.txt"		// our logfile

using namespace std;

class TwitterServer:MultiplexingServer		// server is derived from our multiplexing server
{
	public:
		TwitterServer(const unsigned short port = 3000);
		~TwitterServer();
		void setupServer(const unsigned short port = 3000);
		void run(void);

	private:
		map<SOCKET, string> tweeter;			// this map saves the tweeter socket and name
		multimap<string, string> tweet;			// this multimap saves the tweets by socket and content
		multimap<string, string> abonnement;	// this multimap saves who is followed by whom

		//Helper methods
		SOCKET getSocketByTweeter(const string name);
		void getOtherTweets(const SOCKET clientSocket);
		void getAllTweets(const SOCKET clientSocket);
		void getOwnTweets(const string nameOfTweeter, const SOCKET clientSocket);
		void sendToFollowers(const SOCKET clientSocket, const string message);
		void followTweeter(const SOCKET follower, const string followedTweeter);
		void commandInterpreter(char* command[], const SOCKET clientSocket);
		bool loggedIn(const SOCKET clientSocket);
		void newTweet(const SOCKET clientSocket, const string text);
		void makeSubString(char* command[]);
		void logInTweeter(const SOCKET clientSocket, const string name);
		void logOutTweeter(const SOCKET clientSocket);
		void sendNameOfTweeter(const SOCKET clientSocket);
};

#endif // TWITTERSERVER_H_INCLUDED
