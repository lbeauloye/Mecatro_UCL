cmake_minimum_required(VERSION 3.10)  # CMake version check
project(pi_ctrl)               # Create project "simple_example"
set(CMAKE_CXX_STANDARD 14)            # Enable c++14 standard

include_directories(${PROJECT_SOURCE_DIR})
file(GLOB SRCS
    "${PROJECT_SOURCE_DIR}/*.h"
	"${PROJECT_SOURCE_DIR}/*.c"
	"${PROJECT_SOURCE_DIR}/*.hpp"
	"${PROJECT_SOURCE_DIR}/*.cpp"
)

# Add executable target with source files listed in SOURCE_FILES variable
add_executable(pi_ctrl main.cpp ${SRCS})