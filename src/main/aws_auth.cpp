#include "openssl/hmac.h"
#include "openssl/sha.h"

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/ostream_iterator.hpp>

#include "aws_auth.hpp"

Auth::Auth(const std::string accessKeyId, const std::string secretAccessKey)
: accessKeyId(accessKeyId),
	secretAccessKey(secretAccessKey) {

}

std::string Auth::base64(const unsigned char * data, const unsigned int dataLength) {
	std::stringstream os;
	typedef boost::archive::iterators::base64_from_binary<    // convert binary values ot base64 characters
		boost::archive::iterators::transform_width<   // retrieve 6 bit integers from a sequence of 8 bit bytes
			const unsigned char *, 6, 8
		>
	>
	base64_text; // compose all the above operations in to a new iterator

	std::copy(
		base64_text(data), base64_text(data + dataLength), boost::archive::iterators::ostream_iterator<char>(os)
	);

	std::cout << os.str();

	return os.str();
}

std::string Auth::hex(const unsigned char * data, const unsigned int dataLength) {
	std::ostringstream oss;

	for(int i = 0; i < dataLength; i++)
		oss << boost::format("%1$02x") % (unsigned int) data[i];

	std::cout << "SigningKey HMAC : " << oss.str() << std::endl;

	return oss.str();
}

std::string Auth::hash(std::string input, std::string method) {
	EVP_MD_CTX *mdctx;
	const EVP_MD *md;
	unsigned char md_value[EVP_MAX_MD_SIZE];
	unsigned int md_len;

	OpenSSL_add_all_digests();

	//TODO replace SHA256 by method
	md = EVP_get_digestbyname(method.c_str());

	if(!md) {
		std::cout << "Unknown message digest " << method << std::endl;
		exit(2);
	}

	mdctx = EVP_MD_CTX_create();
	EVP_DigestInit_ex(mdctx, md, NULL);
	EVP_DigestUpdate(mdctx, input.c_str(), input.length());
	EVP_DigestFinal_ex(mdctx, md_value, &md_len);
	EVP_MD_CTX_destroy(mdctx);

	return hex(md_value, md_len);
}

std::string Auth::createCanonicalRequest(web::http::http_request httpRequest, const bool signedPayload) {
	std::ostringstream canonicalRequest;
	std::string headersToSign;

	canonicalRequest << httpRequest.method() << std::endl;
	canonicalRequest << httpRequest.relative_uri().path() << std::endl;
	canonicalRequest << httpRequest.request_uri().query() << std::endl;


	for(web::http::http_headers::iterator iterator = httpRequest.headers().begin(); iterator != httpRequest.headers().end(); iterator++) {
		canonicalRequest << boost::algorithm::to_lower_copy<std::string>(iterator->first) << ":" << iterator->second << std::endl;
		headersToSign += boost::algorithm::to_lower_copy<std::string>(iterator->first) + ";";
	}

	headersToSign = headersToSign.substr(0, headersToSign.size() - 1);

	canonicalRequest << std::endl << headersToSign << std::endl;

	if(signedPayload) {
		Concurrency::streams::istream is = httpRequest.body();

		std::string strToHash = "";

		if(is.is_valid()) {
			std::ostringstream oss;
			std::string line;

			while(!is.is_eof()) {
				unsigned char uc = (unsigned char) is.read().get();

				if(!is.is_eof()) {
					oss << boost::format("%1$c") % uc;
				}
			}

			strToHash = oss.str();
		}

		canonicalRequest << hash(strToHash);
	} else {
		canonicalRequest << "UNSIGNED-PAYLOAD";
	}

	std::cout << "Canonical Request : " << canonicalRequest.str() << std::endl << "@@@@@@@@@@@@@@@@@@@@@@@@@" << std::endl;

	return canonicalRequest.str();
}

std::string Auth::createSigningKey(std::string date, std::string awsRegion, std::string awsService, unsigned char * signingKey, unsigned int * kSigning_len) {
	const EVP_MD *md;
	std::string fakeSecretAccessKey = "AWS4" + secretAccessKey;

	unsigned char kDate[EVP_MAX_MD_SIZE];
	unsigned char kRegion[EVP_MAX_MD_SIZE];
	unsigned char kService[EVP_MAX_MD_SIZE];
	unsigned int kDate_len, kRegion_len, kService_len;

	md = EVP_get_digestbyname("SHA256");

	if(!md) {
		std::cout << "Unknown message digest " << "SHA256" << std::endl;
		exit(2);
	}

	HMAC(md, fakeSecretAccessKey.c_str(), fakeSecretAccessKey.length(), reinterpret_cast<const unsigned char *>(date.c_str()), date.length(), kDate, &kDate_len);

	hex(kDate, kDate_len);

	HMAC(md, kDate, kDate_len, reinterpret_cast<const unsigned char *>(awsRegion.c_str()), awsRegion.length(), kRegion, &kRegion_len);

	hex(kRegion, kRegion_len);

	HMAC(md, kRegion, kRegion_len, reinterpret_cast<const unsigned char *>(awsService.c_str()), awsService.length(), kService, &kService_len);

	hex(kService, kService_len);

	std::string term = "aws4_request";

	HMAC(md, kService, kService_len, reinterpret_cast<const unsigned char *>(term.c_str()), term.length(), signingKey, kSigning_len);

	return hex(signingKey, *kSigning_len);
}

std::string Auth::createStringToSign(std::string date, std::string iso_date, std::string awsRegion, std::string awsService, std::string canonicalHttpRequestHash) {
	std::string algorithm = "AWS4-HMAC-SHA256";

	std::ostringstream oss;

	oss << algorithm << std::endl << iso_date << std::endl << date << '/' << awsRegion << '/' << awsService << '/' << "aws4_request" << std::endl << canonicalHttpRequestHash;

	std::cout << oss.str() << std::endl;

	return oss.str();
}

std::string Auth::createSignature(const std::string stringToSign, const unsigned char* signingKey, const unsigned int kSigning_len) {
	const EVP_MD *md;
	unsigned char signature[EVP_MAX_MD_SIZE];
	unsigned int signature_len;

	//TODO replace SHA512 by method
	md = EVP_get_digestbyname("SHA256");

	if(!md) {
		std::cout << "Unknown message digest " << "SHA256" << std::endl;
		exit(2);
	}

	HMAC(md, signingKey, kSigning_len, reinterpret_cast<const unsigned char *>(stringToSign.c_str()), stringToSign.length(), signature, &signature_len);

	return hex(signature, signature_len);
}

web::http::http_request Auth::signRequestInQuery(const boost::posix_time::ptime time, web::http::http_request request, const std::string region, const std::string service, const bool signedPayload) {
	std::string iso_date = boost::posix_time::to_iso_string(time) + "Z";
	std::string date = iso_date.substr(0, 8);
	web::http::uri_builder uri_builder(request.request_uri());
	std::string escapedString = web::http::uri::encode_data_string(accessKeyId + "/" + boost::posix_time::to_iso_string(time).substr(0, 8) + "/" + region + "/" + service + "/aws4_request");

	if(uri_builder.query() != "") {
		uri_builder.append_query("&");
	}

	std::string headersToSign;

	for(web::http::http_headers::iterator iterator = request.headers().begin(); iterator != request.headers().end(); iterator++) {
		headersToSign += boost::algorithm::to_lower_copy<std::string>(iterator->first) + ";";
	}

	headersToSign = headersToSign.substr(0, headersToSign.size() - 1);

	std::cout << "Signed headers : " << headersToSign << std::endl;

	uri_builder.append_query("X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=" + escapedString +
	"&X-Amz-Date=" + boost::posix_time::to_iso_string(time) +
	"Z&X-Amz-Expires=86400&X-Amz-SignedHeaders=" + web::http::uri::encode_data_string(headersToSign));

	request.set_request_uri(uri_builder.to_uri());

// 	std::cout << "Request : " << request.to_string() << std::endl;

	std::string canonicalRequest = createCanonicalRequest(request, signedPayload);

	std::string canonicalHttpRequestHash = hash(canonicalRequest);

	std::cout << "Request Hash : " << canonicalHttpRequestHash << std::endl;

	std::string stringToSign = createStringToSign(date, iso_date, region, service, canonicalHttpRequestHash);

	unsigned char signingKeyRaw[EVP_MAX_MD_SIZE];
	unsigned int signingKeyLen;

	std::string signingKey = createSigningKey(date, region, service, signingKeyRaw, &signingKeyLen);

	uri_builder.append_query("&X-Amz-Signature=" + createSignature(stringToSign, signingKeyRaw, signingKeyLen));

	request.set_request_uri(uri_builder.to_uri());

	return request;
}

web::http::http_request Auth::signRequestInHeaders(const boost::posix_time::ptime time, web::http::http_request request, const std::string region, const std::string service, const bool signedPayload) {
	std::string iso_date = boost::posix_time::to_iso_string(time) + "Z";
	std::string date = iso_date.substr(0, 8);
	web::http::uri_builder uri_builder(request.request_uri());
	std::string escapedString = web::http::uri::encode_data_string(accessKeyId + "/" + boost::posix_time::to_iso_string(time).substr(0, 8) + "/" + region + "/" + service + "/aws4_request");

	request.headers().add<std::string>("X-Amz-Algorithm", "AWS4-HMAC-SHA256");
	request.headers().add<std::string>("X-Amz-Credential", escapedString);
	request.headers().add<std::string>("X-Amz-Date", boost::posix_time::to_iso_string(time));
	request.headers().add<std::string>("X-Amz-Expires", "86400");

	std::string headersToSign;

	for(web::http::http_headers::iterator iterator = request.headers().begin(); iterator != request.headers().end(); iterator++) {
		headersToSign += boost::algorithm::to_lower_copy<std::string>(iterator->first) + ";";
	}

	headersToSign = headersToSign.substr(0, headersToSign.size() - 1);

	std::cout << "Signed headers : " << headersToSign << std::endl;

	uri_builder.append_query("X-Amz-SignedHeaders=" + web::http::uri::encode_data_string(headersToSign));
	request.headers().add<std::string>("X-Amz-SignedHeaders", escapedString);

	request.set_request_uri(uri_builder.to_uri());

	std::cout << "Request : " << request.to_string() << std::endl;

	std::string canonicalRequest = createCanonicalRequest(request, signedPayload);

	std::string canonicalHttpRequestHash = hash(canonicalRequest);

	std::cout << "Request Hash : " << canonicalHttpRequestHash << std::endl;

	std::string stringToSign = createStringToSign(date, iso_date, region, service, canonicalHttpRequestHash);

	unsigned char signingKeyRaw[EVP_MAX_MD_SIZE];
	unsigned int signingKeyLen;

	std::string signingKey = createSigningKey(date, region, service, signingKeyRaw, &signingKeyLen);

	std::cout << "Before" << std::endl;

	uri_builder.append_query("&X-Amz-Signature=" + createSignature(stringToSign, signingKeyRaw, signingKeyLen));

	std::cout << "After" << std::endl;
	request.set_request_uri(uri_builder.to_uri());

	std::cout << "After" << std::endl;

	return request;
}
