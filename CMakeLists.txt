cmake_minimum_required (VERSION 2.6)

set(EXECUTABLE_NAME aws-cpp)

project(aws-cpp)

set(Casablanca_INCLUDE_DIRS /usr/local/include/casablanca CACHE STRING "Casablanca include path")
set(Casablanca_LIBRARIES "-lcasablanca" CACHE STRING "Casablanca libraries link value")

find_package(Boost REQUIRED COMPONENTS program_options date_time)
find_package(OpenSSL REQUIRED)

set(CMAKE_CXX_FLAGS "-g --std=c++11")

include_directories(
  src/main
  src/test
  ${Casablanca_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${OPENSSL_INCLUDE_DIR}
)

add_library( "${EXECUTABLE_NAME}" SHARED
	src/main/aws_sqs_service.cpp
	src/main/aws_s3_service.cpp
	src/main/aws_auth.cpp
	src/main/aws_get_request.cpp
)

link_directories(${aws-cpp_BINARY_DIR}/libaws-cpp)

# add_library("lib${EXECUTABLE_NAME}" $<TARGET_OBJECTS:"${EXECUTABLE_NAME}_obj">)

enable_testing()

add_test( "${EXECUTABLE_NAME}-test"
	src/test/main_test.cpp
	src/main/aws_sqs_service.cpp
	src/test/aws_sqs_service_test.cpp
)

add_executable( "${EXECUTABLE_NAME}-test"
	src/test/main_test.cpp
	src/test/integration_test.cpp
	src/test/aws_auth_test.cpp
)

target_link_libraries(${EXECUTABLE_NAME} ${Casablanca_LIBRARIES} ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES})
target_link_libraries("${EXECUTABLE_NAME}-test" aws-cpp ${Casablanca_LIBRARIES} ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES})

install(TARGETS ${EXECUTABLE_NAME} 
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

install(DIRECTORY src/main/ DESTINATION include/${EXECUTABLE_NAME} FILES_MATCHING PATTERN "*.hpp")

