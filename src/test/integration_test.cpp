#include <boost/test/unit_test.hpp>

#include "cpprest/http_client.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "aws_sqs_service.hpp"
#include "data/aws_sqs_message.hpp"
#include "aws_s3_service.hpp"

BOOST_AUTO_TEST_SUITE( test_aws_integration )

BOOST_AUTO_TEST_CASE( test_aws_integration_full ) {
	boost::property_tree::ptree pt;

	boost::property_tree::read_json("../src/resources/access.properties", pt);

	Auth auth(pt.get<std::string>("aws.access.keyId"), pt.get<std::string>("aws.access.secretKey"));
	AwsSqsService aws_sqs_service(auth, "eu-west-1");

	std::vector<std::string> queues = aws_sqs_service.listQueues(pt.get<std::string>("aws.test.queuePrefix"));

	for(std::string value : queues) {
		std::cout << value << std::endl;
	}

	AwsSqsMessage message = aws_sqs_service.receiveMessage(queues[0]);

	std::cout << message.getBody() << std::endl;

	aws_sqs_service.deleteMessage(queues[0], message);

	AwsS3Service aws_s3_service(auth);

	std::string objectName = pt.get<std::string>("aws.test.objectName");

	std::vector<unsigned char> bytesVector = aws_s3_service.getObject(pt.get<std::string>("aws.test.bucketName"), objectName);

	std::string filename = boost::replace_all_copy(objectName, "/", "");
	std::ofstream ofs(filename, std::ios_base::out | std::ios_base::ate);

	for(unsigned char c : bytesVector) {
		ofs << c;
	}

	ofs.close();

	aws_s3_service.putObject("cf-photo-blur", objectName, bytesVector);
}

BOOST_AUTO_TEST_SUITE_END()
