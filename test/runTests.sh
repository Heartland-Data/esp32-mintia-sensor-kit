#!/bin/bash

set -u

# Move to the same execution directory as this file
cd $(dirname $0)

SCRIPT_DIR=$(pwd)
BUILD_DIR=$(pwd)/test/build
COVERAGE_DIR=$(pwd)/coverage

echo "$0 pass -> $SCRIPT_DIR"
echo "BUILD_DIR pass -> $BUILD_DIR"
echo "COVERAGE_DIR pass -> $COVERAGE_DIR"

# Check the number of arguments and set RUN_MODE
if [ $# -ge 1 ]; then
    RUN_MODE="$1"
else
    RUN_MODE="--test"  # Default value
fi

# clean build directory if --clean flag is passed
if [ "$RUN_MODE" == "--clean" ]; then
    echo "Cleaning build directory..."
    if [ -d $BUILD_DIR ]; then
        rm -rf $BUILD_DIR
        echo "Build directory cleaned."
    else
        echo "Build directory does not exist."
    fi

    echo "Cleaning coverage directory..."
    if [ -d $COVERAGE_DIR ]; then
        rm -rf $COVERAGE_DIR
        echo "Coverage directory cleaned."
    else
        echo "Coverage directory does not exist."
    fi
    exit 0
fi

# check coverage flag
if [ "$RUN_MODE" == "--coverage" ]; then
    echo "Coverage mode enabled."
    COVERAGE_FLAGS="-DCMAKE_CXX_FLAGS=--coverage -DCMAKE_C_FLAGS=--coverage"
else
    echo "Running tests without coverage."
    COVERAGE_FLAGS=""
fi

# build
if [ ! -d $BUILD_DIR ]; then
    echo "create build folder -> $BUILD_DIR"
    mkdir $BUILD_DIR
else
    echo "The build folder already exists."
    # rm -rf $BUILD_DIR
    # mkdir $BUILD_DIR
fi

cd $BUILD_DIR
cmake $COVERAGE_FLAGS ..
make || exit 1

# run test suites
./runTests --gtest_output="xml:report.xml" || exit 1

# if coverage enabled, generate coverage report
if [ "$RUN_MODE" == "--coverage" ]; then
    echo "Generating coverage report..."
    # create coverage directory
    mkdir -p $COVERAGE_DIR
    # collect coverage info
    lcov --capture --directory . --output-file coverage.info --ignore-errors mismatch

    # remove test code, google test code, and other library code coverage from report
    lcov --remove coverage.info '/usr/*' '*/gtest/*' '*/gmock/*' '*/test/*' --output-file coverage_filtered.info
    # generate coverage report
    genhtml coverage_filtered.info --output-directory $COVERAGE_DIR
    echo "Coverage report generated in root/coverage directory."
fi