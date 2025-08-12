# Boustrophedon Cellular Decomposition Tests

This directory contains comprehensive unit tests for the BCD algorithm implementation.

## Test Files

### Core Algorithm Tests
- `test_geometric_utils.c` - Tests geometric utility functions (angles, edge calculations)
- `test_bcd_event_extraction.c` - Tests event extraction from environment geometry
- `test_bcd_event_extraction_comprehensive.c` - Comprehensive event extraction tests
- `test_bcd_event_classification.c` - Tests event type classification logic
- `test_bcd_event_list_building.c` - Tests complete event list generation

### Validation and Error Handling
- `test_bcd_error_handling.c` - Tests error conditions and edge cases
- `test_bcd_comprehensive_validation.c` - Comprehensive validation of the complete pipeline

### Integration Tests
- `test_bcd_json_pipeline.c` - Tests JSON input processing and BCD integration

## Running Tests

```bash
# Run all BCD tests
make test

# Build all test executables
make all

# Run individual test
make test_geometric_utils.exe
./test_geometric_utils.exe

# Clean test artifacts
make clean
```

## Test Coverage

These tests verify:
- Geometric calculations (angles, edge detection)
- Event extraction from polygonal environments
- Event classification (boundary, obstacle types)
- Event list building and sorting
- Input validation and error handling
- Memory management
- JSON parsing integration
- Complete algorithm pipeline