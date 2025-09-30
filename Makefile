# Makefile for In-Memory Database Project

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -I.
SRCDIR = src
BUILDDIR = build

# Source files
SOURCES = $(SRCDIR)/in_memory_db_imp.cpp
HEADERS = $(SRCDIR)/in_memory_db.hpp $(SRCDIR)/in_memory_db_imp.hpp

# Targets
TEST_TARGET = $(BUILDDIR)/test_db
DEMO_TARGET = $(BUILDDIR)/demo

.PHONY: all clean test demo run-test run-demo compile-only

# Default target
all: $(TEST_TARGET) $(DEMO_TARGET)

# Create build directory
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Compile test program
$(TEST_TARGET): test_db.cpp $(SOURCES) $(HEADERS) | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) test_db.cpp $(SOURCES) -o $(TEST_TARGET)

# Compile demo program
$(DEMO_TARGET): demo.cpp $(SOURCES) $(HEADERS) | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) demo.cpp $(SOURCES) -o $(DEMO_TARGET)

# Run tests
test: $(TEST_TARGET)
	@echo "Running database tests..."
	@./$(TEST_TARGET)

# Run demo
demo: $(DEMO_TARGET)
	@echo "Running database demo..."
	@./$(DEMO_TARGET)

# Just compile without running
compile-only: all
	@echo "Compilation complete. Binaries are in $(BUILDDIR)/"

# Clean build artifacts
clean:
	rm -rf $(BUILDDIR)

# Help target
help:
	@echo "Available targets:"
	@echo "  all         - Compile both test and demo programs (default)"
	@echo "  test        - Compile and run tests"
	@echo "  demo        - Compile and run demo"
	@echo "  compile-only- Just compile without running"
	@echo "  clean       - Remove build artifacts"
	@echo "  help        - Show this help message"
	@echo ""
	@echo "Usage examples:"
	@echo "  make test      # Run all tests"
	@echo "  make demo      # Run interactive demo"
	@echo "  make clean     # Clean build directory"
