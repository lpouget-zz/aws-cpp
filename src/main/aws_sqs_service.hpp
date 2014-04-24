#pragma once

#include <string>
#include <vector>

class AwsSqsService {
public:
  AwsSqsService(){};
  ~AwsSqsService(){};
  
  std::vector<std::string> listQueues(std::string prefix);
};