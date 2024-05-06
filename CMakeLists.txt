cmake_minimum_required(VERSION 3.27)

project(FasterAPI VERSION 1.0.0)

# Define ASIO_STANDALONE to use the standalone ASIO instead of Boost.ASIO
add_definitions(-DASIO_STANDALONE)

# Include the ASIO header files
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/lib")

# define sources and headers
set(SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
)
set(HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/common.hpp"
)

# Create the executable
add_executable(FasterAPI
    ${SOURCES}
    ${HEADERS}
)

# Set the runtime output directory to be inside the build directory
set_target_properties(FasterAPI PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Link the pthread library
target_link_libraries(FasterAPI PRIVATE pthread)

# Configure the file into the build directory
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/config.h.in"
    "${CMAKE_BINARY_DIR}/include/config.h"
)

target_include_directories(FasterAPI PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_BINARY_DIR}/include" # Ensures config.h can be found
)
