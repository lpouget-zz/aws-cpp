#include <iostream>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

#include "aws_sqs_service.hpp"
#include "data/aws_sqs_message.hpp"
#include "aws_s3_service.hpp"

namespace po = boost::program_options;

int main(int argc, char** argv) {
	Auth auth("YOUR_ACCESS_ID", "YOUR_SECRET_KEY");
	AwsSqsService aws_sqs_service(auth, "YOUR_REGION");

  std::vector<std::string> queues = aws_sqs_service.listQueues("dev");

  for(std::string value : queues) {
	std::cout << value << std::endl;
  }

  AwsSqsMessage message = aws_sqs_service.receiveMessage(queues[0]);

  std::cout << message.getBody() << std::endl;

  aws_sqs_service.deleteMessage(queues[0], message.getReceiptHandle());

  AwsS3Service aws_s3_service(auth);

  std::string objectName = "path/object_name";

  std::vector<unsigned char> bytesVector = aws_s3_service.getObject("bucket_name", objectName);

  std::string filename = boost::replace_all_copy(objectName, "/", "");

  std::cout << "Stream " << bytesVector.size() << " bytes to : " << filename << std::endl;
  std::ofstream ofs(filename, std::ios_base::out | std::ios_base::ate);

  for(unsigned char c : bytesVector) {
	  ofs << c;
  }

  ofs.close();
}
