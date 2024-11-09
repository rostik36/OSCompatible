# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(OSCompatibleUnitTests VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)



set(OS_COMPATIBLE_INC_DIR  ${CMAKE_CURRENT_LIST_DIR}/../include)


file(GLOB SOURCES *.cpp)


set(gtest_build_tests ON CACHE BOOL "Build all of gtest's own tests.")

add_subdirectory(../vendor/googletest ${CMAKE_CURRENT_BINARY_DIR}/vendor/googletest)
enable_testing()


# this flag adds and removes necessary parts in the Engine included headers
add_compile_definitions(-DUNIT_TESTS)

# # Add the executable
add_executable(OSCompatibleUnitTests ${SOURCES})


target_link_libraries(OSCompatibleUnitTests PUBLIC OSCompatible)
target_link_libraries(OSCompatibleUnitTests PUBLIC gtest gtest_main)