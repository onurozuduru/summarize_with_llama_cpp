###############################################################################
#File: CMakeLists.txt
#
#License: MIT
#
#Copyright (C) 2025 Onur Ozuduru
#
#Follow Me!
#  github: github.com/onurozuduru
###############################################################################

cmake_minimum_required(VERSION 3.25)
project(summarize_with_llama_cpp_project CXX)
set(APP_NAME summarize_with_llama_cpp)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)

# Print the generator to verify
message(STATUS "CMake generator is ${CMAKE_GENERATOR}")

# Ensure build type is set
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Place all build outputs in the build directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Include model configuration and download logic
include(ModelConfig)

# Fetch and configure llama.cpp
include(LlamaCppSetup)

add_executable(${APP_NAME} src/argument_parser.cpp src/model.cpp src/main.cpp)

# Link with llama.cpp
target_link_libraries(${APP_NAME} PRIVATE llamacpp)
target_include_directories(
    ${APP_NAME}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Install configuration - place everything in the build directory
install(TARGETS ${APP_NAME} RUNTIME DESTINATION ${CMAKE_BINARY_DIR}/bin)

# Copy model files to installation directory
install(
    FILES "${MODEL_PATH}/${MODEL_NAME}"
    DESTINATION ${CMAKE_BINARY_DIR}/models
)

# Include status message module
include(StatusMessage)

message(STATUS "!! Configuration Summary:")

print_status("Build Directory" "${CMAKE_BINARY_DIR}")
print_status("Binary Output" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
print_status("Library Output" "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
print_status("Model Path" "${MODEL_PATH}/${MODEL_NAME}")
print_status("C++ Standard" "${CMAKE_CXX_STANDARD}")
print_status("CUDA Support" "${GGML_CUDA}")
print_status("Metal Support" "${GGML_METAL}")
