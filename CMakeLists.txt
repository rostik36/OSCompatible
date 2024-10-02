# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(OSCompatibleLibrary VERSION 1.1)

option(IncludeExamples, "Include OSCompatiable Examples" OFF)
option(IncludeTests, "Include OSCompatiable Examples" OFF)


if(IncludeExamples)
    add_subdirectory(examples)
endif()

if(IncludeTests)
    add_subdirectory(tests)
endif()

# Include the dir to be visible for the target project
target_include_directories(OSCompatiable PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)