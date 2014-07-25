#pragma once

#include <string>
#include <exception>
#include <system_error>

class AwsSqsException : std::exception {
private:
	const std::string m_what;
public:
	AwsSqsException(const std::exception & exception) : m_what(exception.what()) {

	}

	AwsSqsException() : m_what("") {

	}

	virtual ~AwsSqsException() throw() {};

	virtual const char* what() const throw()
	{
		return std::string("Aws Sqs Exception with cause : "  + m_what).c_str();
	}
};
