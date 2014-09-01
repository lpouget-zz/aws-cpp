#include "pplx/pplxtasks.h"
#include <cpprest/basic_types.h>

#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "openssl/hmac.h"

#include "stdio.h"

#include <iomanip>
#include <sstream>

#include "aws_sqs_service.hpp"
#include "aws_sqs_exception.hpp"

AwsSqsService::AwsSqsService(Auth auth, const std::string region) {
	host = std::unique_ptr<std::string>(new std::string(this->service + "." + region + ".amazonaws.com:80"));
	uri = std::unique_ptr<web::uri>(new web::uri(U(std::string("http://") + *host + std::string("/"))));
	http_client = std::unique_ptr<web::http::client::http_client>(new web::http::client::http_client(*uri));
	awsGetRequest = std::unique_ptr<AwsGetRequest>(new AwsGetRequest(auth, region, this->service));
}

AwsSqsService::AwsSqsService(AwsSqsService && awsSqsService) :
	host(std::move(awsSqsService.host)),
	uri(std::move(awsSqsService.uri)),
	http_client(std::move(awsSqsService.http_client)),
	awsGetRequest(std::move(awsSqsService.awsGetRequest)) {
}

AwsSqsService & AwsSqsService::operator=(AwsSqsService && awsSqsService) {
        host = std::move(awsSqsService.host);
        uri = std::move(awsSqsService.uri);
        http_client = std::move(awsSqsService.http_client);
        awsGetRequest = std::move(awsSqsService.awsGetRequest);
}

std::vector<std::string> AwsSqsService::listQueues(std::string prefix) {
	try {
		web::http::http_request http_request = awsGetRequest->getSignedHttpRequest("Action=ListQueues&QueueNamePrefix=" + prefix);

		return http_client->request(http_request).then([=](web::http::http_response response){
			return response.extract_string();
		}).then([&](utility::string_t str){
			std::vector<std::string> queues;
			boost::property_tree::ptree pt;
			boost::property_tree::ptree empty_pt;

			std::istringstream iss;

			iss.str(str);

			boost::property_tree::read_xml(iss, pt);

			BOOST_FOREACH(boost::property_tree::ptree::value_type &v, pt.get_child_optional("ListQueuesResponse.ListQueuesResult").get_value_or(empty_pt))
				queues.push_back(v.second.data());

			return queues;
		}).get();
	} catch (const std::system_error & e) {
		throw AwsSqsException(e);
	} catch (const std::exception & e) {
		throw AwsSqsException(e);
	} catch (...) {
		//Unknown exception...
		throw AwsSqsException();
	}
}

AwsResponse<AwsSqsMessage> AwsSqsService::receiveMessage(std::string queueUrl, std::string attributeName, int maxNumberOfMessages, int visibilityTimeout, int waitTimeSeconds) {
	try {
		web::http::http_request http_request = awsGetRequest->getSignedHttpRequest("Action=ReceiveMessage&AttributeName=" + attributeName + "&MaxNumberOfMessages=" + boost::lexical_cast<std::string>(maxNumberOfMessages) + "&QueueUrl=" + web::http::uri::encode_data_string(queueUrl) + "&VisibilityTimeout=" + boost::lexical_cast<std::string>(visibilityTimeout) + "&WaitTimeSeconds=" + boost::lexical_cast<std::string>(waitTimeSeconds));

		AwsResponse<AwsSqsMessage> aws_response = http_client->request(http_request).then([=](web::http::http_response response){
			if(response.status_code() != web::http::status_codes::OK) {
				return AwsResponse<AwsSqsMessage>(response.status_code(), response.extract_string().get(), AwsSqsMessage());
			}

			boost::property_tree::ptree pt;
			std::istringstream iss;

			iss.str(response.extract_string().get());

			boost::property_tree::read_xml(iss, pt);

			std::string body = pt.get_optional<std::string>("ReceiveMessageResponse.ReceiveMessageResult.Message.Body").get_value_or("");
			std::string md5OfBody = pt.get_optional<std::string>("ReceiveMessageResponse.ReceiveMessageResult.Message.MD5OfBody").get_value_or("");
			std::string receiptHandle = pt.get_optional<std::string>("ReceiveMessageResponse.ReceiveMessageResult.Message.ReceiptHandle").get_value_or("");
			std::string messageId = pt.get_optional<std::string>("ReceiveMessageResponse.ReceiveMessageResult.Message.MessageId").get_value_or("");

			return AwsResponse<AwsSqsMessage>(response.status_code(), "", AwsSqsMessage(body, md5OfBody, receiptHandle, messageId));
		}).get();

		return aws_response;
	} catch (const std::system_error & e) {
		throw AwsSqsException(e);
	} catch (const std::exception & e) {
		throw AwsSqsException(e);
	} catch (...) {
		//Unknown exception...
		throw AwsSqsException();
	}
}

std::string AwsSqsService::sendMessage(std::string queueUrl, std::string message, int delaySeconds) {
	try {
		web::http::http_request http_request = awsGetRequest->getSignedHttpRequest("Action=SendMessage&DelaySeconds=" + boost::lexical_cast<std::string>(delaySeconds)
			+ "&MessageBody=" + web::http::uri::encode_data_string(message)
			+ "&QueueUrl=" + web::http::uri::encode_data_string(queueUrl));

		return http_client->request(http_request).then([=](web::http::http_response response){
			return response.extract_string();
		}).then([=](utility::string_t str){
			return str;
		}).get();
	} catch (const std::system_error & e) {
		throw new AwsSqsException(e);
	} catch (const std::exception & e) {
		throw new AwsSqsException(e);
	} catch (...) {
		//Unknown exception...
	}
}

std::string AwsSqsService::deleteMessage(std::string queueUrl, AwsSqsMessage aws_sqs_message) {
	try {
		return deleteMessage(queueUrl, aws_sqs_message.getReceiptHandle());
	} catch (...) {
		std::rethrow_exception(std::current_exception());
	}
}

std::string AwsSqsService::deleteMessage(std::string queueUrl, std::string receiptHandle) {
	try {
		web::http::http_request http_request = awsGetRequest->getSignedHttpRequest("Action=DeleteMessage&QueueUrl=" + web::http::uri::encode_data_string(queueUrl) + "&ReceiptHandle=" + web::http::uri::encode_data_string(receiptHandle));

		return http_client->request(http_request).then([=](web::http::http_response response){
			return response.extract_string();
		}).then([=](utility::string_t str){
			return str;
		}).get();
	} catch (const std::system_error & e) {
		throw new AwsSqsException(e);
	} catch (const std::exception & e) {
		throw new AwsSqsException(e);
	} catch (...) {
		//Unknown exception...
	}
}
