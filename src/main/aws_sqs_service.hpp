#pragma once

#include <string>
#include <vector>

#include "cpprest/http_client.h"

#include "aws_auth.hpp"
#include "aws_get_request.hpp"
#include "data/aws_sqs_message.hpp"

class AwsSqsService {
public:
	AwsSqsService(Auth auth, std::string region);
	~AwsSqsService(){};

	std::vector<std::string> listQueues(std::string prefix);
	AwsSqsMessage receiveMessage(std::string queueUrl, std::string attributeName = "all", int maxNumberOfMessages = 1, int visibilityTimeout = 0, int waitTimeSeconds = 0);
	std::string sendMessage(std::string queueUrl, std::string message, int delaySeconds = 0);
	std::string deleteMessage(std::string queueUrl, AwsSqsMessage aws_sqs_message);
	std::string deleteMessage(std::string queueUrl, std::string receiptHandle);

private:
	AwsGetRequest awsGetRequest;
	web::http::client::http_client http_client;
	const std::string service = "sqs";
	std::string host;
};
