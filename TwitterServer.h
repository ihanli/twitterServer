#ifndef TWITTERSERVER_H_INCLUDED
#define TWITTERSERVER_H_INCLUDED

#include <windows.h>
#include <cstring>
#include "SocketBase.h"

#define MAXCLIENTS 10

using namespace std;

class TwitterServer
{
    public:
        TwitterServer(const unsigned short port = 3000, const unsigned int size = 140, const int af = AF_INET, const WORD version = MAKEWORD(2,0), const int type = SOCK_STREAM, const int protocol = 0);
        ~TwitterServer();
        void configServer(const unsigned short port = 3000, const int af = AF_INET);
        unsigned int getBufferSize(void) const;
        void setBufferSize(unsigned int size);
        void clientListener(void);
        bool shutdownFlag(void) const;

    private:
        SOCKET requestSocket;
        SOCKET clients[MAXCLIENTS];
        SOCKADDR_IN localhost;
        FD_SET mainActionFlag;
        SocketBase socketCreator;
        unsigned int bufferSize;
        bool shutdownServer;
        char *clientMessage;

        void closeRequestSocket(void) const;
        void closeSockets(void);
        int acceptClient(void);
        void sendToClient(const SOCKET* client, const char* message);
        void receive(SOCKET* clientSocket);
};

#endif // TWITTERSERVER_H_INCLUDED
