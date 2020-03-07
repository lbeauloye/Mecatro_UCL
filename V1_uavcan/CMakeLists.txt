cmake_minimum_required(VERSION 2.8)
project(motor_board)

# UAVCAN Library
find_library(UAVCAN_LIB uavcan REQUIRED)

# ...
set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic -std=c++11")

# Add generated custom DSDL
include_directories("dsdlc_generated")

# SRC files
include_directories(${PROJECT_SOURCE_DIR}/src)
file(GLOB SRCS
    "${PROJECT_SOURCE_DIR}/src/*.h"
	"${PROJECT_SOURCE_DIR}/src/*.c"
	"${PROJECT_SOURCE_DIR}/src/*.hpp"
	"${PROJECT_SOURCE_DIR}/src/*.cpp"
)

# Link & exec
add_executable(main main.cpp platform_linux.cpp ${SRCS})
target_link_libraries(main ${UAVCAN_LIB} rt)
