#include <iostream>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

#include "aws_sqs_service.hpp"

namespace po = boost::program_options;

int main(int argc, char** argv) {
  AwsSqsService aws_sqs_service;
  
  aws_sqs_service.listQueues("");
  
// 	po::options_description desc("Allowed options");
// 	desc.add_options()
//     		("help", "produce help message")
//     		("compression", po::value<int>(), "set compression level");
// 
// 	po::variables_map vm;
// 	po::store(po::parse_command_line(argc, argv, desc), vm);
// 	po::notify(vm);
// 
// 	if (vm.count("help")) {
// 		std::cout << desc << std::endl;
// 		return 1;
// 	}
}