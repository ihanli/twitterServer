/*###############################
# FH Salzburg                   #
# WS2010 MMT-B2009              #
# Multimediale Netzwerke        #
# Uebungsprojekt                #
# Fritsch Andreas, Hanli Ismail #
# Sun, 28.11.2010 22:00         #
###############################*/

//We need the class SocketBase, because you can't use sockets (in windows) until wsa was started.
//Also a method for socket creation is implemented, so we don't have to think about wsa all the time.

#include "SocketBase.h"

//Constructor for SocketBase. It takes three parameters with following default values:
//clientSocketVersion -> Version 2
//type				-> SOCK_STREAM
//protocol			-> 0

SocketBase::SocketBase(const WORD clientSocketVersion, const int type, const int protocol) :
           version(clientSocketVersion), type(type), protocol(protocol){}

//This method is responsible for everything that's needed to create a socket successfully.
//createSocket takes a reference as parameter. So the method call looks like shown below.
//
//createSocket(&mySocket);

void SocketBase::createSocket(SOCKET* base, int af)
{
    WSADATA wsa;
    int errorCode;

    printf("\nStarting baseSocket...");

    //We have to clear the memory of wsa, for security reasons.

    memset(&wsa, 0, sizeof(WSADATA));

    //Starts WSA so we can create a socket. WSAStartup writes some data into wsa. But we don't need this data.

    errorCode = WSAStartup(version, &wsa);

    //Error handling
    //Throws an error (including the error code) or prints that everything is O.K

    if(errorCode != 0)
        throw exceptionTexter("\nFAIL: Couldn't start baseSocket (WSAStartup) (Error Code: ", WSAGetLastError());
    else
        printf("\nSUCCESS: baseSocket started!");

    printf("\nCreating socket...");

    //Finally the socket can be created.

    *base = socket(af, type, protocol);

    if(*base == INVALID_SOCKET)
    	throw exceptionTexter("\nFAIL: Couldn't create socket (socket) (Error Code: ", WSAGetLastError());
    else
        printf("\nSUCCESS: Socket created!");
}
