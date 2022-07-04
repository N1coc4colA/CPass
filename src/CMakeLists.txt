cmake_minimum_required(VERSION 3.10)

#Setup project basics
project(cpass VERSION ${CPASS_VERSION})

file(GLOB_RECURSE SOURCE_FILES RELATIVE ${PROJECT_SOURCE_DIR} *.cpp *.c qt/*.c qt/*.cpp)
file(GLOB_RECURSE HEADER_FILES RELATIVE ${PROJECT_SOURCE_DIR} *.hpp *.h qt/*.h qt/*.hpp)

find_package(PkgConfig REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)

pkg_check_modules(GCrypt REQUIRED IMPORTED_TARGET libgcrypt)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(AUTOMOC_COMPILER_PREDEFINES ON)

add_executable(cpass ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(cpass ${CMAKE_DL_LIBS} Qt5::Core Qt5::Gui Qt5::Widgets PkgConfig::GCrypt)
