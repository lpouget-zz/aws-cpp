#pragma once

#include <string>

#include<data/aws_object.hpp>

template<class T> class AwsResponse {
public:
	AwsResponse() {};

	AwsResponse(short unsigned int http_code, std::string error_msg, T awsObject) :
	http_code(http_code),
	error_msg(error_msg),
	awsObject(awsObject) {};
	~AwsResponse(){};

	short unsigned int getHttpCode() {
		return http_code;
	}

	std::string getErrorMsg() {
		return error_msg;
	}

	T getAwsObject() {
		return awsObject;
	}

private:
	short unsigned int http_code;
	std::string error_msg;
	T awsObject;
};
