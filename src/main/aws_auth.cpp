#include "openssl/hmac.h"
#include "openssl/sha.h"

#include <boost/format.hpp>

#include "aws_auth.hpp"

Auth::Auth(const std::string accessKeyId, const std::string secretAccessKey)
: accessKeyId(accessKeyId),
	secretAccessKey(secretAccessKey) {

}

std::string Auth::hash(std::string input, std::string method) {
	EVP_MD_CTX *mdctx;
	const EVP_MD *md;
	unsigned char md_value[EVP_MAX_MD_SIZE];
	unsigned int md_len, i;

	OpenSSL_add_all_digests();

	//TODO replace SHA256 by method
	md = EVP_get_digestbyname("SHA256");

	if(!md) {
		std::cout << "Unknown message digest " << "SHA256" << std::endl;
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

std::string Auth::createCanonicalRequest(web::http::http_request httpRequest, const bool signedPayload) {
	std::ostringstream canonicalRequest;
	std::string headersToSign;

	canonicalRequest << httpRequest.method() << std::endl;
	canonicalRequest << httpRequest.relative_uri().path() << std::endl;
	canonicalRequest << httpRequest.request_uri().query() << std::endl;


	for(web::http::http_headers::iterator iterator = httpRequest.headers().begin(); iterator != httpRequest.headers().end(); iterator++) {
		canonicalRequest << iterator->first << ":" << iterator->second << std::endl;
		headersToSign += iterator->first + ";";
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

		std::string _hash = hash(strToHash, "");

		std::cout << "Body Hash : " << _hash << std::endl;

		canonicalRequest << _hash;
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
	unsigned int kDate_len, kRegion_len, kService_len, i;

	md = EVP_get_digestbyname("SHA256");

	if(!md) {
		std::cout << "Unknown message digest " << "SHA256" << std::endl;
		exit(2);
	}

	std::ostringstream oss;

	HMAC(md, fakeSecretAccessKey.c_str(), fakeSecretAccessKey.length(), reinterpret_cast<const unsigned char *>(date.c_str()), date.length(), kDate, &kDate_len);

	for(i = 0; i < kDate_len; i++)
		oss << boost::format("%1$02x") % (unsigned int) kDate[i];

	std::cout << "SigningKey HMAC : " << oss.str() << std::endl;

	HMAC(md, kDate, kDate_len, reinterpret_cast<const unsigned char *>(awsRegion.c_str()), awsRegion.length(), kRegion, &kRegion_len);

	oss.clear();
	oss.seekp(0);
	for(i = 0; i < kRegion_len; i++)
		oss << boost::format("%1$02x") % (unsigned int) kRegion[i];

	std::cout << "SigningKey HMAC : " << oss.str() << std::endl;

	HMAC(md, kRegion, kRegion_len, reinterpret_cast<const unsigned char *>(awsService.c_str()), awsService.length(), kService, &kService_len);

	oss.clear();
	oss.seekp(0);
	for(i = 0; i < kService_len; i++)
		oss << boost::format("%1$02x") % (unsigned int) kService[i];

	std::cout << "SigningKey HMAC : " << oss.str() << std::endl;

	std::string term = "aws4_request";

	HMAC(md, kService, kService_len, reinterpret_cast<const unsigned char *>(term.c_str()), term.length(), signingKey, kSigning_len);

	oss.clear();
	oss.seekp(0);
	for(i = 0; i < *kSigning_len; i++)
		oss << boost::format("%1$02x") % (unsigned int) signingKey[i];

	std::cout << "SigningKey HMAC : " << oss.str() << std::endl;

	return oss.str();
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
	unsigned int signature_len, i;

	//TODO replace SHA512 by method
	md = EVP_get_digestbyname("SHA256");

	if(!md) {
		std::cout << "Unknown message digest " << "SHA256" << std::endl;
		exit(2);
	}

	HMAC(md, signingKey, kSigning_len, reinterpret_cast<const unsigned char *>(stringToSign.c_str()), stringToSign.length(), signature, &signature_len);

	std::ostringstream oss;

	for(i = 0; i < signature_len; i++)
		oss << boost::format("%1$02x") % (unsigned int) signature[i];

	std::cout << "Signature HMAC : " << oss.str() << std::endl;

	return oss.str();
}

web::http::http_request Auth::signRequest(const boost::posix_time::ptime time, web::http::http_request request, const std::string region, const std::string service, const bool signedPayload) {
	std::string iso_date = boost::posix_time::to_iso_string(time) + "Z";
	std::string date = iso_date.substr(0, 8);
	web::http::uri_builder uri_builder(request.request_uri());
	std::string escapedString = web::http::uri::encode_data_string(accessKeyId + "/" + boost::posix_time::to_iso_string(time).substr(0, 8) + "/" + region + "/" + service + "/aws4_request");

	if(uri_builder.query() != "") {
		uri_builder.append_query("&");
	}
	uri_builder.append_query("X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=" + escapedString +
	"&X-Amz-Date=" + boost::posix_time::to_iso_string(time) +
	"Z&X-Amz-Expires=86400&X-Amz-SignedHeaders=host");

	request.set_request_uri(uri_builder.to_uri());

	std::cout << "Request : " << request.to_string() << std::endl;

	std::string canonicalRequest = createCanonicalRequest(request, signedPayload);

	std::string canonicalHttpRequestHash = hash(canonicalRequest, "");

	std::cout << "Request Hash : " << canonicalHttpRequestHash << std::endl;

	std::string stringToSign = createStringToSign(date, iso_date, region, service, canonicalHttpRequestHash);

	unsigned char signingKeyRaw[EVP_MAX_MD_SIZE];
	unsigned int signingKeyLen;

	std::string signingKey = createSigningKey(date, region, service, signingKeyRaw, &signingKeyLen);

	uri_builder.append_query("&X-Amz-Signature=" + createSignature(stringToSign, signingKeyRaw, signingKeyLen));

	request.set_request_uri(uri_builder.to_uri());

	return request;
}
