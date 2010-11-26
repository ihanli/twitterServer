#include <iostream>
#include "TwitterServer.h"

using namespace std;

int main()
{
	TwitterServer myServer;

	try
	{
		myServer.configServer();

		while(true)
		{
			myServer.clientListener();
		}
	}
	catch(const char* failure)
	{
		printf("%s", failure);
	}

    return 0;
}
