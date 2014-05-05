#include <vector>
#include <string>
#include <fstream>

#include <boost/algorithm/string/replace.hpp>
#include "pplx/pplxtasks.h"

#include "aws_s3_service.hpp"

AwsS3Service::AwsS3Service(Auth auth)
	: auth(auth){

}

std::vector<unsigned char> AwsS3Service::getObject(std::string bucketName, std::string objectName) {
	boost::posix_time::ptime time = boost::posix_time::second_clock::universal_time();
	web::http::http_request http_request;
	web::http::uri_builder uri_builder;
	std::string message;

	const std::string region = "eu-west-1";
	const std::string service = "s3";

	uri_builder.set_path("/" + objectName);

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

	web::http::http_response response = http_client.request(http_request).get();

	return response.extract_vector().get();
}
