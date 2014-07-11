#include "pplx/pplxtasks.h"

#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "openssl/hmac.h"
#include <syslog.h>

#include "stdio.h"

#include <iomanip>
#include <sstream>

#include "aws_sqs_service.hpp"
#include "aws_sqs_exception.hpp"

AwsSqsService::AwsSqsService(Auth auth, std::string region)
	: awsGetRequest(auth, region, service),
	host(service + "." + region + ".amazonaws.com:80"),
	http_client(std::string().append("http://").append(host).append("/")){

}

std::vector<std::string> AwsSqsService::listQueues(std::string prefix) {
	try {
		web::http::http_request http_request = awsGetRequest.getSignedHttpRequest("Action=ListQueues&QueueNamePrefix=" + prefix);

		return http_client.request(http_request).then([=](web::http::http_response response){
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
		throw new AwsSqsException(e);
	} catch (const std::exception & e) {
		throw new AwsSqsException(e);
	} catch (...) {
		//Unknown exception...
	}
}

AwsSqsMessage AwsSqsService::receiveMessage(std::string queueUrl, std::string attributeName, int maxNumberOfMessages, int visibilityTimeout, int waitTimeSeconds) {
	try {
		web::http::http_request http_request = awsGetRequest.getSignedHttpRequest("Action=ReceiveMessage&AttributeName=" + attributeName + "&MaxNumberOfMessages=" + boost::lexical_cast<std::string>(maxNumberOfMessages) + "&QueueUrl=" + web::http::uri::encode_data_string(queueUrl) + "&VisibilityTimeout=" + boost::lexical_cast<std::string>(visibilityTimeout) + "&WaitTimeSeconds=" + boost::lexical_cast<std::string>(waitTimeSeconds));

		AwsSqsMessage aws_sqs_message = http_client.request(http_request).then([=](web::http::http_response response){
			return response.extract_string();
		}).then([=](utility::string_t str){
			boost::property_tree::ptree pt;
			std::istringstream iss;

			iss.str(str);

			boost::property_tree::read_xml(iss, pt);

			std::string body = pt.get_optional<std::string>("ReceiveMessageResponse.ReceiveMessageResult.Message.Body").get_value_or("");
			std::string md5OfBody = pt.get_optional<std::string>("ReceiveMessageResponse.ReceiveMessageResult.Message.MD5OfBody").get_value_or("");
			std::string receiptHandle = pt.get_optional<std::string>("ReceiveMessageResponse.ReceiveMessageResult.Message.ReceiptHandle").get_value_or("");
			std::string messageId = pt.get_optional<std::string>("ReceiveMessageResponse.ReceiveMessageResult.Message.MessageId").get_value_or("");

			AwsSqsMessage aws_sqs_message = AwsSqsMessage(body, md5OfBody, receiptHandle, messageId);

			return aws_sqs_message;
		}).get();

		syslog(LOG_INFO | LOG_USER, "ok");

		return aws_sqs_message;
	} catch (const std::system_error & e) {
		throw new AwsSqsException(e);
	} catch (const std::exception & e) {
		throw new AwsSqsException(e);
	} catch (...) {
		//Unknown exception...
	}
}

std::string AwsSqsService::sendMessage(std::string queueUrl, std::string message, int delaySeconds) {
	try {
		web::http::http_request http_request = awsGetRequest.getSignedHttpRequest("Action=SendMessage&QueueUrl=" + web::http::uri::encode_data_string(queueUrl) + "&MessageBody=" + web::http::uri::encode_data_string(message) + "&DelaySeconds=" + boost::lexical_cast<std::string>(delaySeconds));

		return http_client.request(http_request).then([=](web::http::http_response response){
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
		web::http::http_request http_request = awsGetRequest.getSignedHttpRequest("Action=DeleteMessage&QueueUrl=" + web::http::uri::encode_data_string(queueUrl) + "&ReceiptHandle=" + web::http::uri::encode_data_string(receiptHandle));

		return http_client.request(http_request).then([=](web::http::http_response response){
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
