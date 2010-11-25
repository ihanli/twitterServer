#include "TwitterServer.h"

TwitterServer::TwitterServer(const unsigned short port, const unsigned int size, const int af, const WORD version, const int type, const int protocol) :
               socketCreator(version, type, protocol), bufferSize(size), timeout.tv_sec(300), timeout.tv_usec(0)
{
    activeClients = 0;

    for(int i = 0;i < MAXCLIENTS;i++)
    {
        comSockets[i] = INVALID_SOCKET;
    }

    FD_ZERO(&fdSet);
    FD_SET(requestSocket, &fdSet);

    try
    {
        socketCreator.createSocket(&requestSocket, af);
    }
    catch(unsigned char* e)
    {
        printf("%c", *e);
    }
}

TwitterServer::~TwitterServer(void)
{
    closeSockets();
}

void TwitterServer::configServer(const unsigned short port, const int af)
{
    int errorCode;

    for(int i = 0;i < MAXCLIENTS;i++)
    {
        if(clientSockets[i] != INVALID_SOCKET)
        {
            FD_SET(clientSockets[i], &fdSet);
        }
    }

    memset(&localhost, 0, sizeof(SOCKADDR_IN));
    localhost.sin_family = af;
    localhost.sin_port = htons(port);
    localhost.sin_addr.s_addr = ADDR_ANY;

//    errorCode = ioctlsocket(requestSocket, FIONBIO, 1);
//
//    if(errorCode < 0)
//    {
//        throw "\nFAIL: Couldn't set request socket to non-blocking";
//    }

    printf("\nBinding socket...");

    errorCode = bind(requestSocket, (SOCKADDR*)&localhost, sizeof(SOCKADDR_IN));

    if(errorCode == SOCKET_ERROR)
    {
        closeRequestSocket();

        throw "\nFAIL: Unable to bind socket!";
    }
    else
    {
        printf("\nSUCCESS: Bound socket to port %u!", port);
    }

    printf("\nStarting listen mode...");

    errorCode = listen(requestSocket, 10);

    if(errorCode == SOCKET_ERROR)
    {
        closeRequestSocket();

        throw "\nFAIL: Unable to initiate listen mode!";
    }
    else
    {
        printf("\nSUCCESS: Listen mode started!");
    }
}

void TwitterServer::clientListener(void)
{
    int errorCode;

    activeClients = select(0, &fdSet, NULL, NULL, timeout);

    if(activeClients == SOCKET_ERROR)
    {
        throw "\nFAIL: Something went wrong with SELECT!";
    }
    else if(activeClients == 0)
    {
        throw "\nFAIL: Connection timed out!";
    }
    else
    {
    }
}

void TwitterServer::acceptClient(unsigned int clientIndex)
{
    int errorCode;

	comSockets[clientIndex] = accept(requestSocket, NULL, NULL);

	if(clientSockets == INVALID_SOCKET)
	{
		throw "\nFAIL: Couldn't connect client!";
	}
	else
	{
		printf("\nSUCCESS: Connected with client!");
	}
}

void TwitterServer::sendToClient(const char* message)
{
    //TODO: check if whole message was sent

    int errorCode;

    errorCode = send(comSocket, message, bufferSize, 0);

    if(errorCode == SOCKET_ERROR)
    {
        throw "\nFAIL: Unable to send message!";
    }
}

void TwitterServer::receive(char *buffer)
{
    //TODO: check if we received the whole message

    int errorCode;

    while(true)
    {

    }

    errorCode = recv(comSocket, buffer, bufferSize, 0);

    if(errorCode == 0)
    {
        throw "\nFAIL: Lost connection to client!";
    }
    else if(errorCode == SOCKET_ERROR)
    {
        throw "\nFAIL: Unable to receive message!";
    }
}

bool TwitterServer::shutdownFlag(void) const { return shutdownServer; }

unsigned int TwitterServer::getBufferSize(void) const { return bufferSize; }

void TwitterServer::setBufferSize(unsigned int size){ bufferSize = size; }

void TwitterServer::closeRequestSocket(void) const
{
    closesocket(requestSocket);
    WSACleanup();
}

void TwitterServer::closeSockets(void) const
{
    closeRequestSocket();

    for (int i = 0; i < MAXCLIENTS; i++)
    {
      if (FD_ISSET(comSockets[i], &fdSet))
         close(i);
    }
}