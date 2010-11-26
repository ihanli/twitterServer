#ifndef TWITTERSERVER_H_INCLUDED
#define TWITTERSERVER_H_INCLUDED

#include <windows.h>
#include <string>
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
        SOCKET clients[MAXCLIENTS];
        SOCKADDR_IN localhost;
        FD_SET actionFlag;
        SocketBase socketCreator;
        char *clientMessage;

        void closeRequestSocket(void) const;
        void closeSockets(void);
        void acceptClient(void);
        void sendToClient(const SOCKET* client, const char* message);
        void receive(SOCKET* clientSocket);

        void checkClientActivity(void);
        void setClientToOffline(SOCKET* clientSocket);
};

#endif // TWITTERSERVER_H_INCLUDED
