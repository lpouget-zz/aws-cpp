#pragma once

#include "cpprest/http_client.h"

#include <string>
#include <vector>

#include "aws_auth.hpp"

class AwsS3Service {
public:
	AwsS3Service(Auth auth);
	~AwsS3Service(){};

	std::vector<unsigned char> getObject(std::string bucketName, std::string objectName);
	void putObject(std::string bucketName, std::string objectName, std::vector<unsigned char> data);
	void putObject(std::string bucketName, std::string objectName, std::string data);

private:
	Auth auth;
};
