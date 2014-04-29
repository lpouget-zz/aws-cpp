#include <iostream>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

#include "aws_sqs_service.hpp"

namespace po = boost::program_options;

int main(int argc, char** argv) {
	Auth auth("YOUR_ACCESS_KEY_ID", "YOUR_SECRET_KEY");
  AwsSqsService aws_sqs_service(auth);

  std::vector<std::string> queues = aws_sqs_service.listQueues("dev");

  for(std::string value : queues) {
	std::cout << value << std::endl;
  }

  aws_sqs_service.receiveMessage(queues[0]);
}
