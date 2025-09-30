#!/bin/bash

# Script to run individual test cases for the in-memory database

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}In-Memory Database Test Runner${NC}"
echo "================================="

# Check if test case name is provided
if [ $# -eq 0 ]; then
    echo -e "${RED}Usage: $0 <test_case_name>${NC}"
    echo ""
    echo "Available test cases:"
    echo "  - all           : Run all tests"
    echo "  - level1        : Run Level 1 (Basic Operations) tests"
    echo "  - level2        : Run Level 2 (Filtering) tests"
    echo "  - level3        : Run Level 3 (TTL) tests"
    echo "  - level4        : Run Level 4 (Backup/Restore) tests"
    echo "  - compile       : Just compile the code without running tests"
    exit 1
fi

TEST_CASE=$1

# Create build directory if it doesn't exist
mkdir -p build

# Compile the test program
echo -e "${BLUE}Compiling...${NC}"
g++ -std=c++17 -Wall -Wextra -O2 -I. test_db.cpp src/in_memory_db_imp.cpp -o build/test_db

if [ $? -ne 0 ]; then
    echo -e "${RED}Compilation failed!${NC}"
    exit 1
fi

if [ "$TEST_CASE" == "compile" ]; then
    echo -e "${GREEN}Compilation successful!${NC}"
    exit 0
fi

echo -e "${GREEN}Compilation successful!${NC}"
echo ""

# Run the appropriate test
case $TEST_CASE in
    "all")
        echo -e "${BLUE}Running all tests...${NC}"
        ./build/test_db
        ;;
    "level1"|"level2"|"level3"|"level4")
        echo -e "${BLUE}Running $TEST_CASE tests...${NC}"
        echo "Note: Currently running full test suite. Individual level testing can be implemented as needed."
        ./build/test_db
        ;;
    *)
        echo -e "${RED}Unknown test case: $TEST_CASE${NC}"
        echo "Run '$0' without arguments to see available test cases."
        exit 1
        ;;
esac
