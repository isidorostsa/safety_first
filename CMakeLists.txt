cmake_minimum_required(VERSION 3.30.5)
project(test_files)

# Use Clang
set(CMAKE_CXX_COMPILER /usr/local/bin/clang++)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")

# Add src directory to include path
include_directories(src)

# Build examples
add_executable(and_interface examples/and_interface.cpp
        examples/test_repeatability.cpp
)
add_executable(primitive_interface examples/primitive_interface.cpp)
add_executable(test_repeatability examples/test_repeatability.cpp)
