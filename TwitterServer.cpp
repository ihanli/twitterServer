#include "TwitterServer.h"

TwitterServer::TwitterServer(const unsigned short port, const unsigned int size, const int af, const WORD version, const int type, const int protocol) :
               socketCreator(version, type, protocol), bufferSize(size), timeout.tv_sec(300), timeout.tv_usec(0)
{
    activeClients = 0;
    comSocket = 0;

	FD_ZERO(&workingActionFlag);
    FD_ZERO(&mainActionFlag);
    FD_SET(requestSocket, &mainActionFlag);

    try
    {
        socketCreator.createSocket(&requestSocket, af);
        MAXCLIENTS = requestSocket;
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
    int new_sd = 0;

	memcpy(&workingActionFlag, &mainActionFlag, sizeof(mainActionFlag));

    activeClients = select(0, &workingActionFlag, NULL, NULL, timeout);

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
    	for(int currentClient = 0;currentClient <= MAXCLIENTS && activeClients > 0;currentClient++)
    	{
			if(FD_ISSET(currentClient, workingActionFlag))
			{
				activeClients--;

				if(currentClient == requestSocket)
				{
					printf("\nListening socket is readable...");

					while(comSocket != -1)
					{
						try
						{
							acceptClient();
							FD_SET(comSocket, &workingActionFlag);

							if(comSocket > MAXCLIENTS)
							{
								MAXCLIENTS = comSocket;
							}
						}
						catch(const char* e)
						{
							if(*e == "\nFAIL: No clients left")
							{
								shutdownServer = true;
							}
							else
							{
								break;
							}
						}
					}
				}
				else
				{
					printf("\nReading from client...");

					while(true)
					{
						try
						{
							receive(currentClient);
						}
						catch(const char* e)
						{
							printf("%c", *e);
							closeConnection = true;
							break;
						}
					}

					if(closeConnection)
					{
						close(currentClient);
						FD_CLR(currentClient, &mainActionFlag);

						if(currentClient == MAXCLIENTS)
						{
							while(FD_ISSET(MAXCLIENTS, &mainActionFlag) == false)
							{
								MAXCLIENTS--;
							}
						}
					}
				}
			}
    	}
    }
}

void TwitterServer::acceptClient(void)
{
    int errorCode;

	comSocket = accept(requestSocket, NULL, NULL);

	if(comSocket == INVALID_SOCKET)
	{
		if(errno != EWOULDBLOCK)
		{
			throw "\nFAIL: No clients left"
		}
		else
		{
			throw "\nFAIL: Couldn't connect client!";
		}
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

void TwitterServer::receive(int clientSocket)
{
    //TODO: check if we received the whole message

    int errorCode;

    errorCode = recv(clientSocket, clientMessage, bufferSize, 0);

    if(errorCode == 0)
    {
        throw "\nFAIL: Lost connection to client!";
    }
    else if(errorCode == SOCKET_ERROR && errno != EWOULDBLOCK)
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
      if (FD_ISSET(comSockets[i], &actionFlag))
         close(i);
    }
}