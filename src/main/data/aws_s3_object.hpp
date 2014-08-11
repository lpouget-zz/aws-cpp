#pragma once

#include <vector>

#include<data/aws_object.hpp>

class AwsS3Object : AwsObject {
public:
	AwsS3Object() {};

	AwsS3Object(std::vector<unsigned char> buffer) :
		buffer(buffer) {};
	~AwsS3Object(){};

	std::vector<unsigned char> getBuffer() {
		return buffer;
	}

private:
	std::vector<unsigned char> buffer;
};
