
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

set(CMAKE_CXX_STANDARD 11)

SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")  
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall") 

PROJECT (flvpusher)

set(BIN_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

include_directories("/usr/include/openssl")
include_directories("/usr/include")

IF( NOT CMAKE_BUILD_TYPE )
   SET( CMAKE_BUILD_TYPE Release)
ENDIF()
message(STATUS "Build Type: " "${CMAKE_BUILD_TYPE}")

find_package(ZLIB REQUIRED)
if(ZLIB_FOUND)
    message(STATUS "Found zlib")
endif()

find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    message(STATUS "Found openssl")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_INCLUDE_CURRENT_DIR ON)

aux_source_directory(flv flvpusher)
aux_source_directory(mux flvpusher)

FILE(GLOB flv_head "inc/flv/*.h")
FILE(GLOB mux_head "inc/mux/*.h")
FILE(GLOB main_src flvpusher.cpp)
FILE(GLOB flv_src "flv/*.cpp")
FILE(GLOB mux_src "mux/*.cpp")

set(flvpusher_src ${main_src} ${flv_src} ${mux_src})
set(flvpusher_head ${flv_head} ${mux_head})

add_library(flvpush STATIC
${flv_head} ${flv_src}
${mux_head} ${mux_src}
)

add_executable(${PROJECT_NAME} ${flvpusher_src} ${flvpusher_head})
target_link_libraries(${PROJECT_NAME} libz.so libssl.so libcrypto.so librtmp.so)






