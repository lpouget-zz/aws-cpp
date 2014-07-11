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
#include "aws_get_request.hpp"

#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

AwsGetRequest::AwsGetRequest(Auth auth, std::string region, std::string service)
	: auth(auth),
		region(region),
		
		uri()
{
}

web::http::http_request AwsGetRequest::getSignedHttpRequest(std::string safe_uri) {
	boost::posix_time::ptime time = boost::posix_time::second_clock::universal_time();
	web::http::http_request http_request;
	web::http::uri_builder uri_builder;

	uri_builder.set_path("/");
	uri_builder.set_query(safe_uri);

	web::http::client::http_client http_client(uri);

	http_request.set_method(web::http::methods::GET);
	http_request.set_request_uri(uri_builder.to_uri());
	http_request.headers().add<std::string>("host", host);

	http_request = this->auth.signRequestInQuery(time, http_request, region, service);

	http_request.headers().remove("host");

	return http_request;
}
