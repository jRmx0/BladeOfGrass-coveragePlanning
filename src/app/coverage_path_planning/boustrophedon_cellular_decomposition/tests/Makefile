# Comprehensive test Makefile for Boustrophedon Cellular Decomposition
CC = gcc
CFLAGS = -I.. -I../.. -I../../.. -std=c99 -Wall -Wextra
LIBS = -lm -lws2_32

# Source files
BCD_SRC = ../boustrophedon_cellular_decomposition.c
COVERAGE_SRC = ../coverage.c
CPP_SRC = ../../coverage_path_planning.c
CJSON_SRC = ../../../../dependencies/cJSON/cJSON.c

# Test executables
TESTS = test_geometric_utils.exe \
        test_bcd_event_extraction.exe \
        test_bcd_event_extraction_comprehensive.exe \
        test_bcd_event_classification.exe \
        test_bcd_event_list_building.exe \
        test_bcd_error_handling.exe \
        test_bcd_comprehensive_validation.exe \
        test_bcd_json_pipeline.exe

.PHONY: all test clean help

all: $(TESTS)

test: $(TESTS)
	@echo "Running all BCD tests..."
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
	@echo "All tests completed successfully!"

# Individual test targets
test_geometric_utils.exe: test_geometric_utils.c $(BCD_SRC)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

test_bcd_event_extraction.exe: test_bcd_event_extraction.c $(BCD_SRC) $(CPP_SRC)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

test_bcd_event_extraction_comprehensive.exe: test_bcd_event_extraction_comprehensive.c $(BCD_SRC) $(CPP_SRC)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

test_bcd_event_classification.exe: test_bcd_event_classification.c $(BCD_SRC) $(CPP_SRC)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

test_bcd_event_list_building.exe: test_bcd_event_list_building.c $(BCD_SRC) $(CPP_SRC)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

test_bcd_error_handling.exe: test_bcd_error_handling.c $(BCD_SRC) $(CPP_SRC)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

test_bcd_comprehensive_validation.exe: test_bcd_comprehensive_validation.c $(BCD_SRC) $(CPP_SRC)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

test_bcd_json_pipeline.exe: test_bcd_json_pipeline.c $(BCD_SRC) $(COVERAGE_SRC) $(CPP_SRC) $(CJSON_SRC)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

clean:
	@if exist *.exe del *.exe
	@echo "Cleaned test executables"

help:
	@echo "Available targets:"
	@echo "  all     - Build all test executables"
	@echo "  test    - Build and run all tests"
	@echo "  clean   - Remove test executables"
	@echo "  help    - Show this help message"
	@echo ""
	@echo "Individual test targets:"
	@for %%t in ($(TESTS)) do echo "  %%t"