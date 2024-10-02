# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)


set(PROJECT_NAME "OSCompatible")

# Project name and version
project(${PROJECT_NAME} VERSION 1.3.1)

option(${PROJECT_NAME}_IncludeExamples "Include OSCompatiable Examples" OFF)
option(${PROJECT_NAME}_IncludeTests "Include OSCompatiable Examples" OFF)


# Create an interface library target
add_library(${PROJECT_NAME} INTERFACE)


if(IncludeExamples)
    add_subdirectory(examples)
endif()

if(IncludeTests)
    add_subdirectory(tests)
endif()


# Specify the include directories for the header files
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_SOURCE_DIR}/include)

# Optionally, you can set properties like compile options if needed
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_17)