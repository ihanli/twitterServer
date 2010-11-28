/*###############################
# FH Salzburg                   #
# WS2010 MMT-B2009              #
# Multimediale Netzwerke        #
# Uebungsprojekt                #
# Fritsch Andreas, Hanli Ismail #
# Sun, 28.11.2010 22:00         #
###############################*/

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
