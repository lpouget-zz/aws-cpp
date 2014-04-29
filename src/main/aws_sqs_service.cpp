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

AwsSqsService::AwsSqsService(Auth auth)
	: auth(auth){

}

std::vector<std::string> AwsSqsService::listQueues(std::string prefix) {
	boost::posix_time::ptime time = boost::posix_time::second_clock::universal_time();
    web::http::http_request http_request;
	web::http::uri_builder uri_builder;
	std::vector<std::string> queues;

	const std::string region = "eu-west-1";
	const std::string service = "sqs";

// 	uri_builder.set_scheme("http");
// 	uri_builder.set_host("sqs.eu-west-1.amazonaws.com");
	uri_builder.set_path("/");
	uri_builder.set_query("Action=ListQueues&QueueNamePrefix=" + prefix);

	std::cout << "######## " << uri_builder.to_uri().to_string() << std::endl;

	const char* uri = "http://sqs.eu-west-1.amazonaws.com/";

	web::http::client::http_client http_client(uri);

	http_request.set_method(web::http::methods::GET);
	http_request.set_request_uri(uri_builder.to_uri());
	http_request.headers().add<std::string>("host", "sqs.eu-west-1.amazonaws.com:80");

	http_request = this->auth.signRequest(time, http_request, region, service);

	http_request.headers().remove("host");

	std::cout << http_request.to_string() << std::endl;

	web::http::http_response response = http_client.request(http_request).then([=](web::http::http_response response){
		return response.extract_string();
	}).then([&](utility::string_t str){
		std::cout << str << std::endl;
		boost::property_tree::ptree pt;

		std::istringstream iss;

		iss.str(str);

		boost::property_tree::read_xml(iss, pt);

		BOOST_FOREACH(boost::property_tree::ptree::value_type &v, pt.get_child("ListQueuesResponse.ListQueuesResult"))
			queues.push_back(v.second.data());
	}).wait();

	return queues;
}

std::vector<std::string> AwsSqsService::receiveMessage(std::string queueUrl, std::string attributeName, int maxNumberOfMessages, int visibilityTimeout, int waitTimeSeconds) {
	boost::posix_time::ptime time = boost::posix_time::second_clock::universal_time();
	web::http::http_request http_request;
	web::http::uri_builder uri_builder;
	std::vector<std::string> queues;

	const std::string region = "eu-west-1";
	const std::string service = "sqs";

	// 	uri_builder.set_scheme("http");
	// 	uri_builder.set_host("sqs.eu-west-1.amazonaws.com");
	uri_builder.set_path("/");
	uri_builder.set_query("Action=ReceiveMessage&AttributeName=" + attributeName + "&MaxNumberOfMessages=" + boost::lexical_cast<std::string>(maxNumberOfMessages) + "&QueueUrl=" + web::http::uri::encode_data_string(queueUrl) + "&VisibilityTimeout=" + boost::lexical_cast<std::string>(visibilityTimeout) + "&WaitTimeSeconds=" + boost::lexical_cast<std::string>(waitTimeSeconds));

	std::cout << "######## " << uri_builder.to_uri().to_string() << std::endl;

	const char* uri = "http://sqs.eu-west-1.amazonaws.com/";

	web::http::client::http_client http_client(uri);

	http_request.set_method(web::http::methods::GET);
	http_request.set_request_uri(uri_builder.to_uri());
	http_request.headers().add<std::string>("host", "sqs.eu-west-1.amazonaws.com:80");

	http_request = this->auth.signRequest(time, http_request, region, service);

	http_request.headers().remove("host");

	std::cout << http_request.to_string() << std::endl;

	web::http::http_response response = http_client.request(http_request).then([=](web::http::http_response response){
		return response.extract_string();
	}).then([&](utility::string_t str){
		std::cout << str << std::endl;
	}).wait();

	return std::vector<std::string>();
}
