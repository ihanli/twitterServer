#ifndef TWITTERSERVER_H_INCLUDED
#define TWITTERSERVER_H_INCLUDED

#include <windows.h>
#include "SocketBase.h"

// #define MAXCLIENTS 10

class TwitterServer
{
    public:
        TwitterServer(const unsigned short port = 3000, const unsigned int size = 140, const int af = AF_INET, const WORD version = MAKEWORD(2,0), const int type = SOCK_STREAM, const int protocol = 0);
        ~TwitterServer();
        void configServer(const unsigned short port = 3000, const int af = AF_INET);
        void acceptClient(unsigned int clientIndex);
        void sendToClient(const char* message);
        void receive(char *buffer);
        unsigned int getBufferSize(void);
        void setBufferSize(unsigned int size);
        void clientListener(void);
        bool shutdownFlag(void) const;

    private:
        SOCKET requestSocket;
        SOCKADDR_IN localhost;
        SocketBase socketCreator;
        int comSockets[MAXCLIENTS];
        int activeClients;
        const unsigned int MAXCLIENTS = 10;
        const struct timeval timeout;
        FD_SET fdSet;
        bool shutdownServer = false;

        unsigned int bufferSize const;
        void closeRequestSocket(void) const;
        void closeSockets(void) const;
};

#endif // TWITTERSERVER_H_INCLUDED
