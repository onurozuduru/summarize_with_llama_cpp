#!/bin/bash
###############################################################################
#File: build.sh
#
#License: MIT
#
#Copyright (C) 2025 Onur Ozuduru
#
#Follow Me!
#  github: github.com/onurozuduru
###############################################################################

# Simple build script for the project using CMake and Ninja

### Fields
AUTO_DETECT=""
MODEL_URL=""
MODEL_PATH=""
JOBS=8
GENERATE_DOCS=""

### Functions

print_help() {
	echo "Usage: $0 [OPTIONS]"
	echo "Options:"
	echo "  --auto-detect       Enable auto detection"
	echo "  --model-url=URL     Set model URL"
	echo "  --model-path=PATH   Set model path"
	echo "  -j, --jobs NUMBER   Number of parallel jobs (default: $JOBS)"
	echo "  --docs              Generate documentation"
	echo "  -h, --help          Show this help message"
}

# Function to check if a command exists
check_command() {
	if ! command -v "$1" &>/dev/null; then
		echo "Error: $1 is not installed or not in PATH!"
		exit 1
	fi
}

### Get params
# -l long options (--help)
# -o short options (-h)
# : options takes argument (--option1 arg1)
# $@ pass all command line parameters.
set -e
params=$(getopt -l "help,auto-detect,model-url:,model-path:,jobs:,docs" -o "hj:" -- "$@")

eval set -- "$params"

### Run
while true; do
	case $1 in
	-h | --help)
		print_help
		exit 0
		;;
	--auto-detect)
		AUTO_DETECT="YES"
		;;
	--model-url)
		shift
		MODEL_URL="$1"
		;;
	--model-path)
		shift
		MODEL_PATH="$1"
		;;
	-j | --jobs)
		shift
		JOBS="$1"
		;;
	--docs)
		GENERATE_DOCS="YES"
		;;
	--)
		shift
		break
		;;
	*)
		print_help
		exit 0
		;;
	esac
	shift
done

echo "Checking for required tools..."
check_command cmake
check_command ninja

# Print build configuration
echo "Build Configuration:"
if [ -n "$AUTO_DETECT" ]; then
	echo "  Auto Detection: $AUTO_DETECT"
fi
if [ -n "$MODEL_URL" ]; then
	echo "  Model URL: $MODEL_URL"
fi
if [ -n "$MODEL_PATH" ]; then
	echo "  Model Path: $MODEL_PATH"
fi
echo "  Jobs: $JOBS"
echo "  Generate Documentation: $GENERATE_DOCS"

# Configure CMake arguments
CMAKE_ARGS="-G Ninja"

if [ -n "$AUTO_DETECT" ]; then
	CMAKE_ARGS="$CMAKE_ARGS -DAUTO_DETECT=ON"
fi

if [ -n "$MODEL_URL" ]; then
	CMAKE_ARGS="$CMAKE_ARGS -DMODEL_URL=$MODEL_URL"
fi

if [ -n "$MODEL_PATH" ]; then
	CMAKE_ARGS="$CMAKE_ARGS -DMODEL_PATH=$MODEL_PATH"
fi

# Build project
echo "Configuring project..."
if ! cmake $CMAKE_ARGS -S . -B build; then
	echo "CMake configuration failed!"
	exit 1
fi

echo "Building project..."
if ! cmake --build build/ -j $JOBS; then
	echo "Build failed"
	exit 1
fi

# Generate documentation if requested
if [ -n "$GENERATE_DOCS" ]; then
	check_command doxygen
	echo "Generating documentation..."
	doxygen Doxyfile || echo "Documentation generation failed"
fi

echo "Build completed successfully!"
