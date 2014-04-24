cmake_minimum_required (VERSION 2.6)

set(EXECUTABLE_NAME aws-cpp)

project(${EXECUTABLE_NAME})

find_package(Boost REQUIRED COMPONENTS program_options)
find_package(OpenSSL REQUIRED)

set(CMAKE_CXX_FLAGS "-g --std=c++11")

include_directories(
  src/main
  /usr/local/include/casablanca
  ${Boost_INCLUDE_DIRS}
  ${OPENSSL_INCLUDE_DIR}
)

add_executable( ${EXECUTABLE_NAME}
  src/main/main.cpp
  src/main/aws_sqs_service.cpp
)

target_link_libraries(${EXECUTABLE_NAME} "-l:/usr/local/lib/libcasablanca.so" ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES})