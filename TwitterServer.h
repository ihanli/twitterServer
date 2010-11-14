#ifndef TWITTERSERVER_H_INCLUDED
#define TWITTERSERVER_H_INCLUDED

#include "SocketBase.h"

class TwitterServer
{
    public:
        TwitterServer(const unsigned short port = 3000, const unsigned int size = 140, const int af = AF_INET, const WORD version = MAKEWORD(2,0), const int type = SOCK_STREAM, const int protocol = 0);
        ~TwitterServer();
        void configServer(const unsigned short port = 3000, const int af = AF_INET);
        void acceptClient(void);
        void sendToClient(const char* message);
        void receive(char *buffer);
        unsigned int getBufferSize(void);
        void setBufferSize(unsigned int size);

    private:
        SOCKET requestSocket, comSocket;
        SOCKADDR_IN localhost;
        SocketBase socketCreator;
        unsigned int bufferSize;
};

#endif // TWITTERSERVER_H_INCLUDED
