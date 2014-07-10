#pragma once

#include <exception>

class AwsSqsException  : std::exception {
public:
	virtual ~AwsSqsException() throw() {};

	virtual const char* what() const throw()
	{
		return "Aws Sqs Exception";
	}
};
