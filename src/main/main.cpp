#include <iostream>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

#include "aws_sqs_service.hpp"
#include "aws_s3_service.hpp"

namespace po = boost::program_options;

int main(int argc, char** argv) {
	Auth auth("YOUR_ACCESS_ID", "YOUR_SECRET_KEY");
  AwsSqsService aws_sqs_service(auth);

  std::vector<std::string> queues = aws_sqs_service.listQueues("dev");

  for(std::string value : queues) {
	std::cout << value << std::endl;
  }

  aws_sqs_service.receiveMessage(queues[0]);

  AwsS3Service aws_s3_service(auth);

  aws_s3_service.getObject("bucket_name", "object_name");
}
