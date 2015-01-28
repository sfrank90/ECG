# Name the project you can choose any name you want here
PROJECT(Exercise09) 

# Check requirements
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0)

# set the compiler flags
SET(CMAKE_BUILD_TYPE debug)
SET(CMAKE_CXX_FLAGS "-Wall -std=c++11 -L/usr/lib/nvidia-331/")

# Define where your executables should be put
set(EXECUTABLE_OUTPUT_PATH ${Exercise09_BINARY_DIR})

# Add required packages (e.g. libraries)
FIND_PACKAGE(OpenGL 3.3 REQUIRED)
FIND_PACKAGE(GLEW REQUIRED)
FIND_PACKAGE(GLUT REQUIRED)

# Set include directories containing used header files
INCLUDE_DIRECTORIES(
  ${Exercise09_SOURCE_DIR}/include/
  ${Exercise09_SOURCE_DIR}/AntTweakBar/include
)

# Set link directories for addiontional libraries
LINK_DIRECTORIES(
  ${Exercise09_SOURCE_DIR}/lib/
  ${Exercise09_SOURCE_DIR}/AntTweakBar/lib
)

SET(ANTTWEAKBAR_LIBRARIES AntTweakBar)

# Tell CMake to process the sub-directories
ADD_SUBDIRECTORY(src)
