cmake_minimum_required(VERSION 3.10)

project(EpiGimp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
file(GLOB SOURCES 
                ${CMAKE_SOURCE_DIR}/src/*.cpp
                ${CMAKE_SOURCE_DIR}/src/*/*.cpp)

# Add executable
add_executable(EpiGimp ${SOURCES})

# Enable warnings
target_compile_options(EpiGimp PRIVATE 
    -Wall -Wextra -Wpedantic
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

# Link with raylib and required libraries
target_link_libraries(EpiGimp PRIVATE raylib m)

# Print sources
message(STATUS "Sources: ${SOURCES}")