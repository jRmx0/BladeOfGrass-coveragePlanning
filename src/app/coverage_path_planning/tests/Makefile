# Test Makefile for Coverage Path Planning Integration Tests
CC = gcc
CFLAGS = -I.. -I../.. -std=c99 -Wall -Wextra
LIBS = -lm -lws2_32

# Source files
CPP_SRC = ../coverage_path_planning.c
BCD_SRC = ../boustrophedon_cellular_decomposition/boustrophedon_cellular_decomposition.c
COVERAGE_SRC = ../boustrophedon_cellular_decomposition/coverage.c
CJSON_SRC = ../../../dependencies/cJSON/cJSON.c

# Test executables
TESTS = test_integration.exe test_end_to_end.exe

.PHONY: all test clean help

all: $(TESTS)

test: $(TESTS)
	@echo "Running integration tests..."
	@for %%t in ($(TESTS)) do ( \
		echo Running %%t... & \
		%%t & \
		if errorlevel 1 ( \
			echo FAILED: %%t & \
			exit /b 1 \
		) else ( \
			echo PASSED: %%t \
		) \
	)
	@echo "All integration tests completed successfully!"

test_integration.exe: test_integration.c $(CPP_SRC) $(BCD_SRC) $(CJSON_SRC)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

test_end_to_end.exe: test_end_to_end.c $(CPP_SRC) $(BCD_SRC) $(COVERAGE_SRC) $(CJSON_SRC)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

clean:
	@if exist *.exe del *.exe
	@echo "Cleaned integration test executables"

help:
	@echo "Available targets:"
	@echo "  all     - Build all integration test executables"
	@echo "  test    - Build and run all integration tests"
	@echo "  clean   - Remove test executables"
	@echo "  help    - Show this help message"