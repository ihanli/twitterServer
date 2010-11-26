#include "ExceptionTexter.h"

char* exceptionTexter(const char* exceptionText, int errorCode)
{
	char* finalException;
	string foo;

	//TODO: append integer to string of chars

	sscanf(finalException, "%s (Error Code: %d)", exceptionText, &errorCode);

	return finalException;
}
