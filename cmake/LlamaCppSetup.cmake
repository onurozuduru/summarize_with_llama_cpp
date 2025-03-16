###############################################################################
#File: LlamaCppSetup.cmake
#
#License: MIT
#
#Copyright (C) 2025 Onur Ozuduru
#
#Follow Me!
#  github: github.com/onurozuduru
###############################################################################

# Fetch llama.cpp
include(FetchContent)
FetchContent_Declare(
    llamacpp
    GIT_REPOSITORY https://github.com/ggml-org/llama.cpp.git
    GIT_TAG master
)

# Ensure static library build
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libraries" FORCE)

# Project configuration - disable unnecessary components
set(LLAMA_BUILD_EXAMPLES OFF CACHE BOOL "Build llama.cpp EXAMPLES" FORCE)
set(LLAMA_BUILD_TESTS OFF CACHE BOOL "Build llama.cpp TESTS" FORCE)
set(LLAMA_BUILD_SERVER OFF CACHE BOOL "Build llama.cpp SERVER" FORCE)

# CPU optimizations
set(GGML_AVX ON CACHE BOOL "Enable AVX instruction set" FORCE)
set(GGML_AVX2 ON CACHE BOOL "Enable AVX2 instruction set" FORCE)
set(GGML_FMA ON CACHE BOOL "Enable FMA instruction set" FORCE)
set(GGML_F16C ON CACHE BOOL "Enable F16C instruction set" FORCE)

# GPU Optimizations
option(AUTO_DETECT "Automatically detect CUDA and Metal" OFF)

option(USE_CUDA "Enable cuBLAS for NVIDIA GPU acceleration" OFF)
option(USE_METAL "Enable Metal for Apple GPU acceleration" OFF)

if(AUTO_DETECT)
    find_package(CUDA QUIET)
    if(CUDA_FOUND)
        set(USE_CUDA ON)
        message(STATUS "CUDA found, enabling cuBLAS support")
    endif()

    if(APPLE)
        set(USE_METAL ON)
        message(STATUS "Building on macOS, enabling Metal support")
    endif()
endif()

set(GGML_CUDA
    ${USE_CUDA}
    CACHE BOOL
    "Enable cuBLAS for NVIDIA GPU acceleration"
    FORCE
)
set(GGML_METAL
    ${USE_METAL}
    CACHE BOOL
    "Enable Metal for Apple GPU acceleration"
    FORCE
)
set(GGML_CLBLAST OFF CACHE BOOL "Enable OpenCL acceleration via CLBlast" FORCE)

# Make llama available to the project
FetchContent_MakeAvailable(llamacpp)

# Add alias llamacpp for llama
add_library(llamacpp ALIAS llama)
