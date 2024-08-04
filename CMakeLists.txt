# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(OSCompatibleThread VERSION 1.0)


# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


file(GLOB HEADERS include/*.h)
file(GLOB SOURCES src/*.cpp)
message(${SOURCES})

include_directories(include)



# Add the executable
add_executable(OSCompatibleThread ${HEADERS} ${SOURCES})