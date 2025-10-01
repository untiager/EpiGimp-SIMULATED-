cmake_minimum_required(VERSION 3.10)

project(EpiGimp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Separate main source from library sources for testing
file(GLOB_RECURSE LIB_SOURCES 
    ${CMAKE_SOURCE_DIR}/src/Core/*.cpp
    ${CMAKE_SOURCE_DIR}/src/UI/*.cpp
    ${CMAKE_SOURCE_DIR}/src/Utils/*.cpp
    ${CMAKE_SOURCE_DIR}/src/Commands/*.cpp)

# Main executable
add_executable(EpiGimp ${CMAKE_SOURCE_DIR}/src/main.cpp ${LIB_SOURCES})

# Enable warnings
target_compile_options(EpiGimp PRIVATE 
    -Wall -Wextra -Wpedantic
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

# Link with raylib and required libraries
target_link_libraries(EpiGimp PRIVATE raylib m)

# Google Test setup
option(BUILD_TESTS "Build unit tests" ON)

if(BUILD_TESTS)
    # Enable testing
    enable_testing()
    
    # Find or fetch Google Test
    find_package(GTest QUIET)
    if(NOT GTest_FOUND)
        include(FetchContent)
        FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/v1.14.0.zip
            DOWNLOAD_EXTRACT_TIMESTAMP true
        )
        # For Windows: Prevent overriding the parent project's compiler/linker settings
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
    endif()

    # Create a library for testing (without main.cpp)
    add_library(EpiGimpLib STATIC ${LIB_SOURCES})
    target_compile_options(EpiGimpLib PRIVATE 
        -Wall -Wextra -Wpedantic
        $<$<CONFIG:Debug>:-g -O0>
        $<$<CONFIG:Release>:-O3 -DNDEBUG>
    )
    target_link_libraries(EpiGimpLib PRIVATE raylib m)

    # Test executable
    file(GLOB_RECURSE TEST_SOURCES ${CMAKE_SOURCE_DIR}/tests/*.cpp)
    add_executable(EpiGimpTests ${TEST_SOURCES})
    
    target_link_libraries(EpiGimpTests PRIVATE 
        EpiGimpLib
        gtest
        raylib
        m
    )
    
    # Register tests with CTest
    include(GoogleTest)
    gtest_discover_tests(EpiGimpTests)
endif()

# Print sources
message(STATUS "Library Sources: ${LIB_SOURCES}")
if(BUILD_TESTS)
    message(STATUS "Test Sources: ${TEST_SOURCES}")
endif()