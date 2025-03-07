cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)
project(CPSUI)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release
        CACHE STRING "Set build type to Debug or Release (default Release)" FORCE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

ADD_DEFINITIONS(-std=c++14)


include_directories("./include")
include_directories("./share")

link_directories("./lib")

add_subdirectory("./msvc/CPSUI")


