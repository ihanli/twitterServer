#include <iostream>
#include "TwitterServer.h"

using namespace std;

int main()
{
	TwitterServer myTwitterServer;

	try
	{
		myTwitterServer.configServer();

		while(true)
		{
			myTwitterServer.clientListener();
		}
	}
	catch(const char* failure)
	{
		printf("%s", failure);
	}

    return 0;
}
