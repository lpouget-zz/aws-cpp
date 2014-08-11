#pragma once

#include <string>

#include<data/aws_object.hpp>

class AwsSqsMessage : AwsObject {
public:
	AwsSqsMessage() {};

	AwsSqsMessage(std::string body, std::string md5OfBody, std::string receiptHandle, std::string messageId) :
		body(body),
		md5OfBody(md5OfBody),
		receiptHandle(receiptHandle),
		messageId(messageId) {};
	~AwsSqsMessage(){};

	std::string getBody() {
		return body;
	}

	std::string getMd5OfBody() {
		return md5OfBody;
	}

	std::string getReceiptHandle() {
		return receiptHandle;
	}

	std::string getMessageId() {
		return messageId;
	}

private:
	std::string body;
	std::string md5OfBody;
	std::string receiptHandle;
	std::string messageId;
};
