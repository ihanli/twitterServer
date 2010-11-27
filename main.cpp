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
	catch(string failure)
	{
		printf("%s", failure.c_str());
	}

    return 0;
}
