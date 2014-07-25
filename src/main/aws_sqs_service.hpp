#pragma once

#include <string>
#include <vector>

#include "cpprest/http_client.h"

#include "aws_auth.hpp"
#include "aws_get_request.hpp"
#include "data/aws_sqs_message.hpp"

class AwsSqsService {
public:
	AwsSqsService(Auth auth, const std::string region);
	AwsSqsService(AwsSqsService && awsSqsService);
	~AwsSqsService(){};

	std::vector<std::string> listQueues(std::string prefix);
	AwsSqsMessage receiveMessage(std::string queueUrl, std::string attributeName = "all", int maxNumberOfMessages = 1, int visibilityTimeout = 0, int waitTimeSeconds = 0);
	std::string sendMessage(std::string queueUrl, std::string message, int delaySeconds = 0);
	std::string deleteMessage(std::string queueUrl, AwsSqsMessage aws_sqs_message);
	std::string deleteMessage(std::string queueUrl, std::string receiptHandle);

	AwsSqsService & operator=(AwsSqsService && awsSqsService);

private:
	const std::string service = "sqs";
	std::unique_ptr<std::string> host;
	std::unique_ptr<web::uri> uri;
	std::unique_ptr<web::http::client::http_client> http_client;
	std::unique_ptr<AwsGetRequest> awsGetRequest;
};
