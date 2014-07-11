#pragma once

#include <string>
#include <exception>
#include <system_error>
#include <string.h>

class AwsSqsException  : std::exception {
private:
	const std::exception exception;
public:
	AwsSqsException(const std::exception & exception) : exception(exception) {

	}

	AwsSqsException(const std::system_error & system_error) : exception(system_error) {

	}

	virtual ~AwsSqsException() throw() {};

	virtual const char* what() const throw()
	{
		return strcat((char *) "Aws Sqs Exception with cause : " , exception.what());
	}
};
