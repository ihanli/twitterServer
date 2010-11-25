#ifndef TWITTERSERVER_H_INCLUDED
#define TWITTERSERVER_H_INCLUDED

#include <windows.h>
#include "SocketBase.h"

class TwitterServer
{
    public:
        TwitterServer(const unsigned short port = 3000, const unsigned int size = 140, const int af = AF_INET, const WORD version = MAKEWORD(2,0), const int type = SOCK_STREAM, const int protocol = 0);
        ~TwitterServer();
        void configServer(const unsigned short port = 3000, const int af = AF_INET);
        unsigned int getBufferSize(void);
        void setBufferSize(unsigned int size);
        void clientListener(void);
        bool shutdownFlag(void) const;

    private:
        SOCKET requestSocket, MAXCLIENTS;
        SOCKADDR_IN localhost;
        FD_SET mainActionFlag, workingActionFlag;
        SocketBase socketCreator;
        const struct timeval timeout;
        int comSocket;
        int activeClients;
        unsigned int bufferSize const;
        bool shutdownServer = false;
        bool closeConnection = false;
        char clientMessage[bufferSize];

        void closeRequestSocket(void) const;
        void closeSockets(void) const;
        void acceptClient(void);
        void sendToClient(const char* message);
        void receive(int clientSocket);
};

#endif // TWITTERSERVER_H_INCLUDED
