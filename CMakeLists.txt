cmake_minimum_required(VERSION 3.10)

#Setup project basics
set(CPASS_VERSION 1.0)
project(CPass VERSION ${CPASS_VERSION})

if (NOT DEFINED VERSION)
    set(VERSION 1.0)
endif ()

if(NOT DEFINED CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX /usr/bin)
endif()

#Setup standards
set(CMAKE_C_STANDARD 20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -std=c++20" )

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    message("---> You are building in DEBUG mode.")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pipe -g -pthread -pthread -Wall -Wextra -D_REENTRANT -fPIC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe -g -pthread -pthread -Wall -Wextra -D_REENTRANT -fPIC")
else()
    message("---> You are not building in debug mode.")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong -D_FORTITY_SOURCE=1 -z noexecstack -pie -fPIC -z lazy")
endif()

set(CMAKE_VERBOSE_MAKEFILE ON)

#Add the precious files :)
add_subdirectory(src)

