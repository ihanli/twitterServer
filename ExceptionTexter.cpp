#include "ExceptionTexter.h"

const char* exceptionTexter(const char* exceptionText, const int errorCode)
{
	ostringstream finalException;

	finalException << exceptionText;
	finalException << errorCode;
	finalException << ")";

	return finalException.str().c_str();
}
