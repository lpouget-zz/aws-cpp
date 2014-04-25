#include "cpprest/http_client.h"

#include "pplx/pplxtasks.h"

#include "openssl/hmac.h"
#include "openssl/sha.h"

#include "boost/format.hpp"
#include "boost/format/group.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>

#include "stdio.h"

#include <iomanip>
#include <sstream>

#include "aws_sqs_service.hpp"

web::http::http_response checkResponse(const std::string &url, const web::http::http_response &response) {
	std::cout << response.to_string() << std::endl;
	return response;
}

std::string hash(std::string input, std::string method) {
	EVP_MD_CTX *mdctx;
	const EVP_MD *md;
	unsigned char md_value[EVP_MAX_MD_SIZE];
	unsigned int md_len, i;

	OpenSSL_add_all_digests();

	//TODO replace SHA512 by method
	md = EVP_get_digestbyname("SHA512");

	if(!md) {
		std::cout << "Unknown message digest " << "SHA512" << std::endl;
		exit(2);
	}

	mdctx = EVP_MD_CTX_create();
	EVP_DigestInit_ex(mdctx, md, NULL);
	EVP_DigestUpdate(mdctx, input.c_str(), input.length());
	EVP_DigestFinal_ex(mdctx, md_value, &md_len);
	EVP_MD_CTX_destroy(mdctx);

	std::ostringstream oss;

	for(i = 0; i < md_len; i++)
		oss << boost::format("%1$02x") % (unsigned int) md_value[i];

	return oss.str();
}

std::string createCanonicalRequest(web::http::http_request httpRequest) {
	web::http::http_request canonicalHttpRequest = httpRequest;

	std::ostringstream oss;

	Concurrency::streams::istream is = httpRequest.body();
	std::string line;

	while(!is.is_eof()) {
		unsigned char uc = (unsigned char) is.read().get();

		if(!is.is_eof()) {
			oss << boost::format("%1$c") % uc;
		}
	}

	std::string _hash = hash(oss.str(), "");

	std::cout << "Body Hash : " << _hash << std::endl;

	canonicalHttpRequest.set_body(_hash);

	return canonicalHttpRequest.to_string();
}

void createSigningKey(std::string date, std::string secretAccessKey, std::string awsRegion, std::string awsService, unsigned char* kSigning) {
	HMAC_CTX *hctx;
	const EVP_MD *md;
	std::string fakeSecretAccessKey = "AWS" + secretAccessKey;

	unsigned char kDate[EVP_MAX_MD_SIZE];
	unsigned char kRegion[EVP_MAX_MD_SIZE];
	unsigned char kService[EVP_MAX_MD_SIZE];
	unsigned int kDate_len, kRegion_len, kService_len, kSigning_len, i;

	//TODO replace SHA512 by method
	md = EVP_get_digestbyname("SHA512");

	if(!md) {
		std::cout << "Unknown message digest " << "SHA512" << std::endl;
		exit(2);
	}

	HMAC(md, fakeSecretAccessKey.c_str(), fakeSecretAccessKey.length(), reinterpret_cast<const unsigned char *>(date.c_str()), date.length(), kDate, &kDate_len);
	HMAC(md, kDate, kDate_len, reinterpret_cast<const unsigned char *>(awsRegion.c_str()), awsRegion.length(), kRegion, &kRegion_len);
	HMAC(md, kRegion, kRegion_len, reinterpret_cast<const unsigned char *>(awsService.c_str()), awsService.length(), kService, &kService_len);

	std::cout << sizeof("aws4_request") << std::endl;

	HMAC(md, kService, kService_len, reinterpret_cast<const unsigned char *>("aws4_request"), sizeof("aws4_request"), kSigning, &kSigning_len);

	std::ostringstream oss;

	for(i = 0; i < kSigning_len; i++)
		oss << boost::format("%1$d") % (unsigned int) kSigning[i] << " ";

	std::cout << "SigningKey HMAC : " << oss.str() << std::endl;
}

std::string createStringToSign(std::string date, std::string awsRegion, std::string awsService, std::string canonicalHttpRequestHash) {
	std::string algorithm = "AWS4-HMAC-SHA512";

	boost::posix_time::ptime t = boost::posix_time::second_clock::local_time();
	std::ostringstream oss;

	oss << algorithm << std::endl << boost::posix_time::to_iso_string(t) << 'Z' << std::endl << date << '/' << awsRegion << '/' << awsService << '/' << "aws4_request" << std::endl << canonicalHttpRequestHash;

	std::cout << oss.str() << std::endl;

	return oss.str();
}

std::string createSignature(std::string stringToSign, const unsigned char* signingKey) {
	HMAC_CTX *hctx;
	const EVP_MD *md;
	unsigned char signature[EVP_MAX_MD_SIZE];
	unsigned int signature_len, i;

	//TODO replace SHA512 by method
	md = EVP_get_digestbyname("SHA512");

	if(!md) {
		std::cout << "Unknown message digest " << "SHA512" << std::endl;
		exit(2);
	}

	std::cout << sizeof(signingKey) << std::endl;

	HMAC(md, stringToSign.c_str(), stringToSign.length(), signingKey, sizeof(signingKey), signature, &signature_len);

	std::ostringstream oss;

	for(i = 0; i < signature_len; i++)
		oss << boost::format("%1$d") % (unsigned int) signature[i] << " ";

	std::cout << "Signature HMAC : " << oss.str() << std::endl;

	return oss.str();
}

std::vector<std::string> AwsSqsService::listQueues(std::string prefix) {
  utility::string_t address = U("http://www.google.com/");

    web::http::uri uri = web::http::uri(address);
    web::http::http_request http_request;

	http_request.set_method(web::http::methods::GET);
	http_request.set_request_uri(uri);
	http_request.set_body("Iamabody!");

	boost::gregorian::date date(boost::gregorian::day_clock::local_day());
	std::ostringstream oss;

	boost::gregorian::date_facet* facet(new boost::gregorian::date_facet("%Y%m%d"));
	oss.imbue(std::locale(oss.getloc(), facet));
	oss << date;

    std::cout << http_request.to_string() << std::endl;

	http_request = createCanonicalRequest(http_request);

	std::cout << http_request.to_string() << std::endl;

	std::string canonicalHttpRequestHash = hash(http_request.to_string(), "");

	std::cout << "Request Hash : " << canonicalHttpRequestHash << std::endl;

	std::string stringToSign = createStringToSign(oss.str(), "MyRegion", "MyService", canonicalHttpRequestHash);

	unsigned char signingKey[EVP_MAX_MD_SIZE];

	createSigningKey(oss.str(), "MySecretKey", "MyRegion", "MyService", signingKey);
	std::string signature = createSignature(stringToSign, signingKey);

	return std::vector<std::string>();
//     utility::ostringstream_t buf;
//     buf << U("?request=refresh&name=");
//     checkResponse("blackjack/dealer", client.request(web::http::methods::PUT, buf.str()).get());
}
