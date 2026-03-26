cmake_minimum_required(VERSION 3.30.5)

# Use Clang
set(CMAKE_CXX_COMPILER /usr/local/bin/clang++)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")

project(test_files)

# Fetch nlohmann/json
include(FetchContent)
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)

# Add src directory to include path
include_directories(src)

# Build examples
add_executable(and_interface examples/and_interface.cpp)
add_executable(equal examples/equal.cpp)
add_executable(primitive_interface examples/primitive_interface.cpp examples/equal.hpp)
add_executable(test_repeatability examples/test_repeatability.cpp)
add_executable(not examples/not.cpp)
add_executable(bool_constructors examples/bool_constructors.cpp)
add_executable(uint8_constructors examples/uint8_constructors.cpp)
add_executable(test_or examples/test_or.cpp)
add_executable(test_and examples/test_and.cpp)
add_executable(test_implies examples/test_implies.cpp)

foreach(target and_interface equal primitive_interface test_repeatability not bool_constructors uint8_constructors test_or test_and test_implies)
    target_link_libraries(${target} PRIVATE nlohmann_json::nlohmann_json)
endforeach()
