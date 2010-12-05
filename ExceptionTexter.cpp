/*###############################
# FH Salzburg                   #
# WS2010 MMT-B2009              #
# Multimediale Netzwerke        #
# Uebungsprojekt                #
# Fritsch Andreas, Hanli Ismail #
# Sun, 28.11.2010 22:00         #
###############################*/

#include "ExceptionTexter.h"

//This function is "needed", because we wanted to concatenate strings with integer in c++ style.

const char* exceptionTexter(const char* exceptionText, const int errorCode)
{
	ostringstream finalException;

//	ostringstream converst the shifted types automatically into string objects.

	finalException << exceptionText;
	finalException << errorCode;
	finalException << ")";

//	We had to return a c-string in case of compatibility problems

	return finalException.str().c_str();
}
