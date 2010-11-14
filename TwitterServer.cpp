#include "TwitterServer.h"

TwitterServer::TwitterServer(const unsigned short port, const unsigned int size, const int af, const WORD version, const int type, const int protocol) :
               socketCreator(version, type, protocol), bufferSize(size)
{
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
    closesocket(requestSocket);
    closesocket(comSocket);
    WSACleanup();
}

void TwitterServer::configServer(const unsigned short port, const int af)
{
    int errorCode;

    memset(&localhost, 0, sizeof(SOCKADDR_IN));
    localhost.sin_family = af;
    localhost.sin_port = htons(port);
    localhost.sin_addr.s_addr = ADDR_ANY;

    printf("\nBinding socket...");

    errorCode = bind(requestSocket, (SOCKADDR*)&localhost, sizeof(SOCKADDR_IN));

    if(errorCode == SOCKET_ERROR)
    {
        closesocket(requestSocket);
        WSACleanup();

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
        closesocket(requestSocket);
        WSACleanup();

        throw "\nFAIL: Unable to initiate listen mode!";
    }
    else
    {
        printf("\nSUCCESS: Listen mode started!");
    }
}

void TwitterServer::acceptClient(void)
{
    comSocket = accept(requestSocket, NULL, NULL);

    printf("\nConnecting client...");

    if(comSocket == INVALID_SOCKET)
    {
        throw "\nFAIL: Unable to connect client!";
    }
    else
    {
        printf("\nSUCCESS: Connected with client!");
    }
}

void TwitterServer::sendToClient(const char* message)
{
    int errorCode;

    errorCode = send(comSocket, message, bufferSize, 0);

    if(errorCode == SOCKET_ERROR)
    {
        throw "\nFAIL: Unable to send message!";
    }
}

void TwitterServer::receive(char *buffer)
{
    int errorCode;

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

unsigned int TwitterServer::getBufferSize(void){ return bufferSize; }

void TwitterServer::setBufferSize(unsigned int size){ bufferSize = size; }