# Coverage Path Planning Test Suite

This document describes the comprehensive test suite for the coverage path planning system.

## Test Organization

The test suite is organized into two main categories:

### 1. Algorithm Unit Tests (`boustrophedon_cellular_decomposition/tests/`)
Focused tests for the BCD algorithm implementation:
- **Geometric utilities** - Angle calculations, edge detection
- **Event extraction** - Converting environment geometry to events
- **Event classification** - Determining event types (boundary, obstacle)
- **Event list building** - Complete event processing pipeline
- **Error handling** - Edge cases and error conditions
- **JSON integration** - JSON input processing

### 2. Integration Tests (`tests/`)
End-to-end tests for the complete system:
- **JSON parsing** - Input validation and parsing
- **Pipeline integration** - Complete processing flow
- **Memory management** - Resource cleanup verification
- **End-to-end functionality** - Full system operation

## Running Tests

### Run All Tests
```bash
# From coverage_path_planning directory
make -f test_Makefile test
```

### Run Specific Test Suites
```bash
# BCD algorithm tests only
make -f test_Makefile test-bcd

# Integration tests only
make -f test_Makefile test-integration
```

### Run Individual Tests
```bash
# Navigate to specific test directory
cd boustrophedon_cellular_decomposition/tests
make test_geometric_utils.exe
./test_geometric_utils.exe
```

## Test Structure

```
coverage_path_planning/
├── test_Makefile                           # Master test coordinator
├── TESTING.md                              # This documentation
├── tests/                                  # Integration tests
│   ├── Makefile                           # Integration test build
│   ├── README.md                          # Integration test docs
│   ├── test_integration.c                 # JSON parsing tests
│   └── test_end_to_end.c                  # Complete pipeline tests
└── boustrophedon_cellular_decomposition/
    └── tests/                             # Algorithm unit tests
        ├── Makefile                       # BCD test build
        ├── README.md                      # BCD test docs
        ├── TEST_SUMMARY.md                # Original test summary
        ├── test_geometric_utils.c         # Geometric calculations
        ├── test_bcd_event_extraction.c    # Event extraction
        ├── test_bcd_event_classification.c # Event classification
        ├── test_bcd_event_list_building.c # Event list processing
        ├── test_bcd_error_handling.c      # Error conditions
        ├── test_bcd_comprehensive_validation.c # Complete validation
        └── test_bcd_json_pipeline.c       # JSON integration
```

## Build Requirements

- GCC compiler with C99 support
- Windows libraries (`-lws2_32`)
- Math library (`-lm`)
- cJSON dependency (automatically included)

## Test Coverage

The test suite provides comprehensive coverage of:
- ✅ Geometric utility functions
- ✅ Environment parsing and validation
- ✅ Event extraction from polygonal boundaries
- ✅ Event classification algorithms
- ✅ Event list building and sorting
- ✅ Error handling and edge cases
- ✅ Memory management
- ✅ JSON input processing
- ✅ End-to-end pipeline functionality

## Maintenance

When adding new functionality:
1. Add unit tests to the appropriate `boustrophedon_cellular_decomposition/tests/` file
2. Add integration tests to `tests/` if the change affects the complete pipeline
3. Update the relevant Makefile to include new test executables
4. Update documentation as needed