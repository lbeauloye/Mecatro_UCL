# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/jyl/Documents/GitHub/MotorBoard/uavcan_test

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/jyl/Documents/GitHub/MotorBoard/uavcan_test/build

# Utility rule file for mtr_dsdlc.

# Include the progress variables for this target.
include CMakeFiles/mtr_dsdlc.dir/progress.make

CMakeFiles/mtr_dsdlc:
	cd /home/jyl/Documents/GitHub/MotorBoard/uavcan_test && libuavcan_dsdlc ./motor -I/usr/local/share/uavcan/dsdl/uavcan

mtr_dsdlc: CMakeFiles/mtr_dsdlc
mtr_dsdlc: CMakeFiles/mtr_dsdlc.dir/build.make

.PHONY : mtr_dsdlc

# Rule to build all files generated by this target.
CMakeFiles/mtr_dsdlc.dir/build: mtr_dsdlc

.PHONY : CMakeFiles/mtr_dsdlc.dir/build

CMakeFiles/mtr_dsdlc.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/mtr_dsdlc.dir/cmake_clean.cmake
.PHONY : CMakeFiles/mtr_dsdlc.dir/clean

CMakeFiles/mtr_dsdlc.dir/depend:
	cd /home/jyl/Documents/GitHub/MotorBoard/uavcan_test/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/jyl/Documents/GitHub/MotorBoard/uavcan_test /home/jyl/Documents/GitHub/MotorBoard/uavcan_test /home/jyl/Documents/GitHub/MotorBoard/uavcan_test/build /home/jyl/Documents/GitHub/MotorBoard/uavcan_test/build /home/jyl/Documents/GitHub/MotorBoard/uavcan_test/build/CMakeFiles/mtr_dsdlc.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/mtr_dsdlc.dir/depend
