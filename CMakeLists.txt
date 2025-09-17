cmake_minimum_required(VERSION 3.10)

project(EpiGimp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)

# Add executable
add_executable(EpiGimp ${SOURCES})

# Link with raylib and math library
target_link_libraries(EpiGimp PRIVATE raylib m)