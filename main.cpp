#include <iostream>
#include "TwitterServer.h"

using namespace std;

int main()
{
	TwitterServer myServer;
	const unsigned int MESSAGESIZE = 140;
    char kbInput[MESSAGESIZE];
    char message[MESSAGESIZE];

    try
    {
        myServer.configServer();
    }
    catch(const char* e)
    {
        printf("%c", *e);
    }

    while(true)
    {
        try
        {
            myServer.acceptClient();
        }
        catch(const char* e)
        {
            printf("%c", *e);
        }

        printf("\n\niterative server - windows only\n\n");
        myServer.sendToClient("Joshua: Welcome Dr. Falken");

        while(true)
        	{
            try
            {
                myServer.receive(message);

                if(message[0] == 'q' && message[1] == '\0')
                	{
                    printf("\nClient disconnected...");
                    break;
                }

                printf("\nClient said: %s", message);
                myServer.sendToClient(message);
            }
            catch(const char* e)
            {
                printf("%c", *e);
            }
        }
    }

    return 0;
}
