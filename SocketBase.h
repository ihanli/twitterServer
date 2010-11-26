#ifndef SOCKETBASE_H_INCLUDED
#define SOCKETBASE_H_INCLUDED

#include <winsock2.h>
#include "SocketBase.h"
#include "ExceptionTexter.h"

class SocketBase
{
    public:
        SocketBase(const WORD clientSocketVersion = MAKEWORD(2,0), const int type = SOCK_STREAM, const int protocol = 0);
        void createSocket(SOCKET* base, int af);

    private:
        WORD version;
        int type;
        int protocol;
};

#endif // SOCKETBASE_H_INCLUDED
