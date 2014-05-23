#include "pplx/pplxtasks.h"

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "openssl/hmac.h"

#include "stdio.h"

#include <iomanip>
#include <sstream>

#include "aws_sqs_service.hpp"

web::http::http_response checkResponse(const std::string &url, const web::http::http_response &response) {
	std::cout << response.to_string() << std::endl;
	return response;
}

AwsSqsService::AwsSqsService(Auth auth, std::string region)
	: auth(auth),
	region(region),
	host(service + "." + region + ".amazonaws.com:80"),
	uri(std::string().append("http://").append(host).append("/")) {
}

std::vector<std::string> AwsSqsService::listQueues(std::string prefix) {
	boost::posix_time::ptime time = boost::posix_time::second_clock::universal_time();
    web::http::http_request http_request;
	web::http::uri_builder uri_builder;
	std::vector<std::string> queues;

	uri_builder.set_path("/");
	uri_builder.set_query("Action=ListQueues&QueueNamePrefix=" + prefix);

	web::http::client::http_client http_client(uri);

	http_request.set_method(web::http::methods::GET);
	http_request.set_request_uri(uri_builder.to_uri());
	http_request.headers().add<std::string>("host", host);

	http_request = this->auth.signRequestInQuery(time, http_request, region, service);

	http_request.headers().remove("host");

	web::http::http_response response = http_client.request(http_request).then([=](web::http::http_response response){
		return response.extract_string();
	}).then([&](utility::string_t str){
		boost::property_tree::ptree pt;
		boost::property_tree::ptree empty_pt;

		std::istringstream iss;

		iss.str(str);

		boost::property_tree::read_xml(iss, pt);

		BOOST_FOREACH(boost::property_tree::ptree::value_type &v, pt.get_child_optional("ListQueuesResponse.ListQueuesResult").get_value_or(empty_pt))
			queues.push_back(v.second.data());
	}).wait();

	return queues;
}

AwsSqsMessage AwsSqsService::receiveMessage(std::string queueUrl, std::string attributeName, int maxNumberOfMessages, int visibilityTimeout, int waitTimeSeconds) {
	boost::posix_time::ptime time = boost::posix_time::second_clock::universal_time();
	web::http::http_request http_request;
	web::http::uri_builder uri_builder;
	AwsSqsMessage aws_sqs_message("", "", "", "");


	// 	uri_builder.set_scheme("http");
	// 	uri_builder.set_host("sqs.eu-west-1.amazonaws.com");
	uri_builder.set_path("/");
	uri_builder.set_query("Action=ReceiveMessage&AttributeName=" + attributeName + "&MaxNumberOfMessages=" + boost::lexical_cast<std::string>(maxNumberOfMessages) + "&QueueUrl=" + web::http::uri::encode_data_string(queueUrl) + "&VisibilityTimeout=" + boost::lexical_cast<std::string>(visibilityTimeout) + "&WaitTimeSeconds=" + boost::lexical_cast<std::string>(waitTimeSeconds));

	web::http::client::http_client http_client(uri);

	http_request.set_method(web::http::methods::GET);
	http_request.set_request_uri(uri_builder.to_uri());
	http_request.headers().add<std::string>("host", host);

	http_request = this->auth.signRequestInQuery(time, http_request, region, service);

	http_request.headers().remove("host");

	web::http::http_response response = http_client.request(http_request).then([=](web::http::http_response response){
		return response.extract_string();
	}).then([&](utility::string_t str){
		boost::property_tree::ptree pt;

		std::istringstream iss;

		iss.str(str);

		boost::property_tree::read_xml(iss, pt);

		std::string body = pt.get_optional<std::string>("ReceiveMessageResponse.ReceiveMessageResult.Message.Body").get_value_or("");
		std::string md5OfBody = pt.get_optional<std::string>("ReceiveMessageResponse.ReceiveMessageResult.Message.MD5OfBody").get_value_or("");
		std::string receiptHandle = pt.get_optional<std::string>("ReceiveMessageResponse.ReceiveMessageResult.Message.ReceiptHandle").get_value_or("");
		std::string messageId = pt.get_optional<std::string>("ReceiveMessageResponse.ReceiveMessageResult.Message.MessageId").get_value_or("");

		aws_sqs_message = AwsSqsMessage(body, md5OfBody, receiptHandle, messageId);
	}).wait();

	return aws_sqs_message;
}

std::string AwsSqsService::deleteMessage(std::string queueUrl, AwsSqsMessage aws_sqs_message) {
	return deleteMessage(queueUrl, aws_sqs_message.getReceiptHandle());
}

std::string AwsSqsService::deleteMessage(std::string queueUrl, std::string receiptHandle) {
	boost::posix_time::ptime time = boost::posix_time::second_clock::universal_time();
	web::http::http_request http_request;
	web::http::uri_builder uri_builder;
	std::string message;

	// 	uri_builder.set_scheme("http");
	// 	uri_builder.set_host("sqs.eu-west-1.amazonaws.com");
	uri_builder.set_path("/");
	uri_builder.set_query("Action=DeleteMessage&QueueUrl=" + web::http::uri::encode_data_string(queueUrl) + "&ReceiptHandle=" + web::http::uri::encode_data_string(receiptHandle));

	web::http::client::http_client http_client(uri);

	http_request.set_method(web::http::methods::GET);
	http_request.set_request_uri(uri_builder.to_uri());
	http_request.headers().add<std::string>("host", host);

	http_request = this->auth.signRequestInQuery(time, http_request, region, service);

	http_request.headers().remove("host");

	web::http::http_response response = http_client.request(http_request).then([=](web::http::http_response response){
		return response.extract_string();
	}).then([&](utility::string_t str){
		message = str;
	}).wait();

	return message;
}
