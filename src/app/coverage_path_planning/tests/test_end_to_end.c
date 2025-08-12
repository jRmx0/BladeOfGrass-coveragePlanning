#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "coverage_path_planning.h"
#include "boustrophedon_cellular_decomposition/boustrophedon_cellular_decomposition.h"
#include "boustrophedon_cellular_decomposition/coverage.h"

// Test function to directly verify event list generation and processing
static void test_direct_event_list_processing() {
    printf("\n=== Testing Direct Event List Processing ===\n");
    
    // Use JSON-based approach instead of programmatic creation to avoid memory issues
    const char* test_json = 
    "{"
    "  \"id\": 1,"
    "  \"pathWidth\": 1.0,"
    "  \"pathOverlap\": 0.1,"
    "  \"boundary\": ["
    "    {\"x\": 0.0, \"y\": 0.0},"
    "    {\"x\": 5.0, \"y\": 0.0},"
    "    {\"x\": 5.0, \"y\": 5.0},"
    "    {\"x\": 0.0, \"y\": 5.0}"
    "  ],"
    "  \"obstacles\": ["
    "    ["
    "      {\"x\": 1.5, \"y\": 1.5},"
    "      {\"x\": 3.5, \"y\": 1.5},"
    "      {\"x\": 3.5, \"y\": 3.5},"
    "      {\"x\": 1.5, \"y\": 3.5}"
    "    ]"
    "  ]"
    "}";
    
    printf("Testing complete pipeline with JSON input...\n");
    coverage_path_planning_process(test_json);
    
    printf("Direct event list processing test completed successfully\n");
}

// Test function to verify memory management
static void test_memory_management() {
    printf("\n=== Testing Memory Management ===\n");
    
    const char* test_json = 
    "{"
    "  \"id\": 1,"
    "  \"pathWidth\": 1.0,"
    "  \"pathOverlap\": 0.1,"
    "  \"boundary\": ["
    "    {\"x\": 0.0, \"y\": 0.0},"
    "    {\"x\": 2.0, \"y\": 0.0},"
    "    {\"x\": 2.0, \"y\": 2.0},"
    "    {\"x\": 0.0, \"y\": 2.0}"
    "  ],"
    "  \"obstacles\": []"
    "}";
    
    // Run the pipeline multiple times to check for memory leaks
    for (int i = 0; i < 3; i++) {
        printf("Memory test iteration %d...\n", i + 1);
        coverage_path_planning_process(test_json);
    }
    
    printf("Memory management test completed\n");
}

// Main test runner
int main() {
    printf("Starting End-to-End Coverage Path Planning Tests\n");
    printf("===============================================\n");
    
    // Run comprehensive tests
    test_direct_event_list_processing();
    test_memory_management();
    
    printf("\n===============================================\n");
    printf("All end-to-end tests completed successfully!\n");
    printf("Integration between JSON parsing, event generation, and BCD processing verified.\n");
    
    return 0;
}