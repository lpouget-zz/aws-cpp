#include <boost/test/unit_test.hpp>

#include "cpprest/http_client.h"

#include "aws_auth.hpp"

//____________________________________________________________________________//

BOOST_AUTO_TEST_SUITE( test_aws_auth )

BOOST_AUTO_TEST_CASE( test_aws_auth_hash_payload )
{
	std::string payload = "Action=ListUsers&Version=2010-05-08";
	Auth auth("", "");

	BOOST_CHECK_EQUAL( auth.hash(payload, ""), "b6359072c78d70ebee1e81adcbab4f01bf2c23245fa365ef83fe8f1f955085e2" );
}

BOOST_AUTO_TEST_CASE( test_aws_auth_hash_empty_payload )
{
	std::string payload = "";
	Auth auth("", "");

	BOOST_CHECK_EQUAL( auth.hash(payload, ""), "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855" );
}

BOOST_AUTO_TEST_CASE( test_aws_auth_canonicalRequest )
{
	utility::string_t address = U("/?foo=Zoo&foo=aha");

	// 	eu-west-1

	web::http::uri uri = web::http::uri(address);

	web::http::http_request http_request;

	http_request.set_method(web::http::methods::GET);
	http_request.set_request_uri(uri);
	http_request.headers().add<std::string>("date", "Mon, 09 Sep 2011 23:36:00 GMT");
	http_request.headers().add<std::string>("host", "host.foo.com");

	std::ostringstream expected;

	expected << "GET" << std::endl << "/" << std::endl << "foo=Zoo&foo=aha" << std::endl << "date:Mon, 09 Sep 2011 23:36:00 GMT" << std::endl << "host:host.foo.com" << std::endl << std::endl << "date;host" << std::endl << "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";

	Auth auth("", "");

	BOOST_CHECK_EQUAL( auth.createCanonicalRequest(http_request, true), expected.str() );
}

BOOST_AUTO_TEST_CASE( test_aws_auth_create_signing_key )
{
	std::string signingKey;
	std::string date = "20120215";
	std::string awsRegion = "us-east-1";
	std::string awsService = "iam";
	unsigned char signingKeyRaw[64];
	unsigned int signingKeyLen;

	Auth auth("", "wJalrXUtnFEMI/K7MDENG+bPxRfiCYEXAMPLEKEY");

	signingKey = auth.createSigningKey(date, awsRegion, awsService, signingKeyRaw, &signingKeyLen);

	BOOST_CHECK_EQUAL( signingKey, "f4780e2d9f65fa895f9c67b32ce1baf0b0d8a43505a000a1a9e090d414db404d" );
}

BOOST_AUTO_TEST_CASE( test_aws_auth_create_signature )
{
	std::string signingKey;
	std::string date = "20110909";
	std::string awsRegion = "us-east-1";
	std::string awsService = "host";
	unsigned char signingKeyRaw[64];
	unsigned int signingKeyLen;
	Auth auth("", "wJalrXUtnFEMI/K7MDENG+bPxRfiCYEXAMPLEKEY");

	signingKey = auth.createSigningKey(date, awsRegion, awsService, signingKeyRaw, &signingKeyLen);

	std::string stringToSign = "AWS4-HMAC-SHA256\n20110909T233600Z\n20110909/us-east-1/host/aws4_request\ndf63ee3247c0356c696a3b21f8d8490b01fa9cd5bc6550ef5ef5f4636b7b8901";
	std::string signature = auth.createSignature(stringToSign, signingKeyRaw, signingKeyLen);

	BOOST_CHECK_EQUAL( signature, "830cc36d03f0f84e6ee4953fbe701c1c8b71a0372c63af9255aa364dd183281e" );
}

BOOST_AUTO_TEST_SUITE_END()
