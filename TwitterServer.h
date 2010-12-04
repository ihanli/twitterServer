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
#include "SocketBase.h"
#include "MultiplexingServer.h"

#define MAXCLIENTS 10			// how many clients the server can handle
#define BUFFERSIZE 140			// maximum lenghth of the tweets

using namespace std;

class TwitterServer:MultiplexingServer
{
	public:
		TwitterServer(const unsigned short port = 3000);
		~TwitterServer();
		void setupServer(const unsigned short port = 3000);
		void run(void);

	private:
		map<SOCKET, string> tweeter;			// saves tweeter socket and name
		multimap<string, string> tweet;			// saves tweets by socket and content
		multimap<string, string> abonnement;	// saves who follows who
		SOCKADDR_IN localhost;

		//Helper methods
		SOCKET getSocketByTweeter(const string name);
		void getOtherTweets(const SOCKET clientSocket);
		void getAllTweets(const SOCKET clientSocket);
		void getOwnTweets(const SOCKET clientSocket);
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
