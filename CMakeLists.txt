cmake_minimum_required(VERSION 3.27)

project(FasterAPI VERSION 1.0.0)

# Libs
# Add ASIO include directory
include_directories(${CMAKE_SOURCE_DIR}/asio/asio/include)

# Add definitions for ASIO
add_definitions(-DASIO_STANDALONE)
add_definitions(-DASIO_NO_DEPRECATED)

# define sources and headers
set(HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/common.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/httphdr.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/httpreq_message.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/httprsp_listener.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/httprsp_message.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/httprsp_run.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/utils.hpp"
)
set(SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/common.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/httprsp_listener.cpp"
)

# Create the executable
add_executable(FasterAPI
    ${HEADERS}
    ${SOURCES}
)

# Set the runtime output directory to be inside the build directory
set_target_properties(FasterAPI PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Link necessary libraries, if any, for ASIO (e.g., pthread)
find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} Threads::Threads)

# Configure the file into the build directory
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/config.h.in"
    "${CMAKE_BINARY_DIR}/include/config.h"
)

target_include_directories(FasterAPI PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_BINARY_DIR}/include" # Ensures config.h can be found
)
