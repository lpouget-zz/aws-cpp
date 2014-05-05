#pragma once

#include "cpprest/http_client.h"

#include <string>
#include <vector>

#include "aws_auth.hpp"
#include "data/aws_sqs_message.hpp"

class AwsSqsService {
public:
	AwsSqsService(Auth auth, std::string region);
	~AwsSqsService(){};

	std::vector<std::string> listQueues(std::string prefix);
	AwsSqsMessage receiveMessage(std::string queueUrl, std::string attributeName = "all", int maxNumberOfMessages = 1, int visibilityTimeout = 0, int waitTimeSeconds = 0);
	std::string deleteMessage(std::string queueUrl, AwsSqsMessage aws_sqs_message);
	std::string deleteMessage(std::string queueUrl, std::string receiptHandle);

private:
	Auth auth;
	const std::string region;
	const std::string service = "sqs";
	const std::string host;
	const std::string  uri;
};
