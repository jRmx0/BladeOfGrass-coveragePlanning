# Coverage Path Planning Integration Tests

This directory contains integration tests that verify the complete coverage path planning pipeline.

## Test Files

- `test_integration.c` - Tests the JSON parsing and basic integration functionality
- `test_end_to_end.c` - End-to-end tests of the complete pipeline from JSON input to coverage output

## Running Tests

```bash
# Run all integration tests
make test

# Build all test executables
make all

# Clean test artifacts
make clean
```

## Test Coverage

These integration tests verify:
- JSON input parsing and validation
- Integration between coverage_path_planning.c and BCD algorithms
- Memory management across the complete pipeline
- Error handling for invalid inputs
- End-to-end functionality from input to output