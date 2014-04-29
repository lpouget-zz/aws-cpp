#pragma once

#include "cpprest/http_client.h"

#include <string>
#include <vector>

#include "aws_auth.hpp"

class AwsSqsService {
public:
	AwsSqsService(Auth auth);
	~AwsSqsService(){};

	std::vector<std::string> listQueues(std::string prefix);
	std::vector<std::string> receiveMessage(std::string queueUrl, std::string attributeName = "all", int maxNumberOfMessages = 1, int visibilityTimeout = 0, int waitTimeSeconds = 0);

private:
	Auth auth;
};
