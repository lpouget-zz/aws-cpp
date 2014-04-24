#include "cpprest/http_client.h"

#include "pplx/pplxtasks.h"

#include "openssl/hmac.h"
#include "openssl/sha.h"

#include "aws_sqs_service.hpp"

web::http::http_response checkResponse(const std::string &url, const web::http::http_response &response) {
    std::cout << response.to_string() << std::endl;
    return response;
}

std::string createCanonicalRequest(std::string httpRequestMethod, std::string canonicalUri, std::string canonicalQueryString, std::string canonicalHeaders, std::string signedHeaders, std::string requestPayload) {
  return httpRequestMethod + ' ' + canonicalUri + '?' + canonicalQueryString + '\n' + canonicalHeaders + '\n' + signedHeaders + '\n' + requestPayload;
}



std::vector<std::string> AwsSqsService::listQueues(std::string prefix) {
  utility::string_t address = U("http://www.google.com/");

    web::http::uri uri = web::http::uri(address);
    

//     web::http::client::http_client client(web::http::uri_builder(uri).append_path(U("/blackjack/dealer")).to_uri());
    
    web::http::http_request http_request;
    
    http_request.set_request_uri(uri);
    
    std::cout << http_request.to_string() << std::endl;

//     utility::ostringstream_t buf;
//     buf << U("?request=refresh&name=");
//     checkResponse("blackjack/dealer", client.request(web::http::methods::PUT, buf.str()).get());
}