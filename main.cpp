#include <iostream>
#include "TwitterServer.h"

using namespace std;

int main()
{
	TwitterServer myServer;

	try
	{
		myServer.configServer();

		while(myServer.shutdownFlag() == false)
		{
			myServer.clientListener();
		}
	}
	catch(const char* e)
	{
		printf("%c", *e);
	}

    return 0;
}
