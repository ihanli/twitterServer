#include <iostream>
#include "TwitterServer.h"

using namespace std;

int main()
{
	TwitterServer myServer;

	myServer.configServer();

	while(myServer.shutdownFlag() == false)
	{
		try
		{
			myServer.clientListener();
		}
		catch(const char* e)
		{
			printf("%s", *e);
		}
	}

    return 0;
}
