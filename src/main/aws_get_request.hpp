/*
 * Copyright 2014 <copyright holder> <email>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef AWSGETREQUEST_H
#define AWSGETREQUEST_H

#include "cpprest/http_client.h"

#include "aws_auth.hpp"

class AwsGetRequest
{
public:
	AwsGetRequest(Auth auth, std::string region, std::string service);
	~AwsGetRequest() {};

	web::http::http_request getSignedHttpRequest(std::string safe_uri);
private:
	web::http::http_request http_request;
	Auth auth;
	const std::string region;
	const std::string service = "sqs";
	const std::string host;
	const std::string  uri;
};

#endif // AWSGETREQUEST_H
