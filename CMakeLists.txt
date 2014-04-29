cmake_minimum_required (VERSION 2.6)

set(EXECUTABLE_NAME aws-cpp)

project(${EXECUTABLE_NAME})

find_package(Boost REQUIRED COMPONENTS program_options date_time)
find_package(OpenSSL REQUIRED)

set(CMAKE_CXX_FLAGS "-g --std=c++11")

include_directories(
  src/main
  src/test
  /usr/local/include/casablanca
  ${Boost_INCLUDE_DIRS}
  ${OPENSSL_INCLUDE_DIR}
)

add_executable( ${EXECUTABLE_NAME}
  src/main/main.cpp
  src/main/aws_sqs_service.cpp
	src/main/aws_auth.cpp
)

enable_testing()

add_test( ${EXECUTABLE_NAME}_test
	src/test/main_test.cpp
	src/main/aws_sqs_service.cpp
  src/test/aws_sqs_service_test.cpp
)

add_executable( ${EXECUTABLE_NAME}_test
	src/test/main_test.cpp
	src/main/aws_sqs_service.cpp
	src/main/aws_auth.cpp
	src/test/aws_auth_test.cpp
  )

target_link_libraries(${EXECUTABLE_NAME} "-l:/usr/local/lib/libcasablanca.so" ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES})
target_link_libraries(${EXECUTABLE_NAME}_test "-l:/usr/local/lib/libcasablanca.so" ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES})
