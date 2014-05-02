#pragma once

#include "cpprest/http_client.h"

#include <string>
#include <vector>

#include "aws_auth.hpp"

class AwsS3Service {
public:
	AwsS3Service(Auth auth);
	~AwsS3Service(){};

	std::string getObject(std::string bucketName, std::string objectName);

private:
	Auth auth;
};