#include <vector>
#include <string>
#include <fstream>

#include <boost/algorithm/string/replace.hpp>
#include "pplx/pplxtasks.h"

#include "aws_s3_service.hpp"

AwsS3Service::AwsS3Service(Auth auth)
	: auth(auth){

}

std::string AwsS3Service::getObject(std::string bucketName, std::string objectName) {
	boost::posix_time::ptime time = boost::posix_time::second_clock::universal_time();
	web::http::http_request http_request;
	web::http::uri_builder uri_builder;
	std::string message;

	const std::string region = "eu-west-1";
	const std::string service = "s3";

// 	uri_builder.set_scheme("http");
	// 	uri_builder.set_host("sqs.eu-west-1.amazonaws.com");
	uri_builder.set_path("/" + objectName);
// 	uri_builder.set_query("Action=ReceiveMessage&AttributeName=" + attributeName + "&MaxNumberOfMessages=" + boost::lexical_cast<std::string>(maxNumberOfMessages) + "&QueueUrl=" + web::http::uri::encode_data_string(queueUrl) + "&VisibilityTimeout=" + boost::lexical_cast<std::string>(visibilityTimeout) + "&WaitTimeSeconds=" + boost::lexical_cast<std::string>(waitTimeSeconds));

	std::cout << "######## " << uri_builder.to_uri().to_string() << std::endl;

	std::string host = bucketName + ".s3.amazonaws.com:80";

	std::string uri;

	uri.append("http://").append(host).append("/");



	web::http::client::http_client http_client(uri.c_str());

	http_request.set_method(web::http::methods::GET);
	http_request.set_request_uri(uri_builder.to_uri());
	http_request.headers().add<std::string>("host", host);

	http_request = this->auth.signRequest(time, http_request, region, service, false);

	http_request.headers().remove("host");

	web::http::http_response response = http_client.request(http_request).then([=](web::http::http_response response){
		return response.extract_vector();
	}).then([=](std::vector<unsigned char> bytesVector){
		std::string filename = boost::replace_all_copy(objectName, "/", "");

		std::cout << "Stream " << bytesVector.size() << " bytes to : " << filename << std::endl;
		std::ofstream ofs(filename, std::ios_base::out | std::ios_base::ate);

		for(unsigned char c : bytesVector) {
		  ofs << c;
		}

		ofs.close();
	}).wait();

	return message;
}
