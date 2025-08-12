#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "coverage_path_planning.h"

// Test JSON input for a simple rectangle environment
static const char* test_simple_rectangle_json = 
"{"
"  \"id\": 1,"
"  \"pathWidth\": 1.0,"
"  \"pathOverlap\": 0.1,"
"  \"boundary\": ["
"    {\"x\": 0.0, \"y\": 0.0},"
"    {\"x\": 10.0, \"y\": 0.0},"
"    {\"x\": 10.0, \"y\": 10.0},"
"    {\"x\": 0.0, \"y\": 10.0}"
"  ],"
"  \"obstacles\": []"
"}";

// Test JSON input for rectangle with single obstacle
static const char* test_rectangle_with_obstacle_json = 
"{"
"  \"id\": 2,"
"  \"pathWidth\": 1.0,"
"  \"pathOverlap\": 0.1,"
"  \"boundary\": ["
"    {\"x\": 0.0, \"y\": 0.0},"
"    {\"x\": 20.0, \"y\": 0.0},"
"    {\"x\": 20.0, \"y\": 15.0},"
"    {\"x\": 0.0, \"y\": 15.0}"
"  ],"
"  \"obstacles\": ["
"    ["
"      {\"x\": 5.0, \"y\": 5.0},"
"      {\"x\": 10.0, \"y\": 5.0},"
"      {\"x\": 10.0, \"y\": 10.0},"
"      {\"x\": 5.0, \"y\": 10.0}"
"    ]"
"  ]"
"}";

// Test function to verify end-to-end integration
static void test_integration_simple_rectangle() {
    printf("\n=== Testing Integration: Simple Rectangle ===\n");
    
    // This should parse JSON, generate events, and complete successfully
    coverage_path_planning_process(test_simple_rectangle_json);
    
    printf("Simple rectangle integration test completed\n");
}

static void test_integration_rectangle_with_obstacle() {
    printf("\n=== Testing Integration: Rectangle with Obstacle ===\n");
    
    // This should parse JSON, generate events for boundary and obstacle, and complete successfully
    coverage_path_planning_process(test_rectangle_with_obstacle_json);
    
    printf("Rectangle with obstacle integration test completed\n");
}

static void test_integration_invalid_json() {
    printf("\n=== Testing Integration: Invalid JSON ===\n");
    
    const char* invalid_json = "{\"invalid\": \"json\", \"missing\": \"required_fields\"}";
    
    // This should fail gracefully during JSON parsing
    coverage_path_planning_process(invalid_json);
    
    printf("Invalid JSON integration test completed\n");
}

static void test_integration_null_input() {
    printf("\n=== Testing Integration: NULL Input ===\n");
    
    // This should fail gracefully with NULL input
    coverage_path_planning_process(NULL);
    
    printf("NULL input integration test completed\n");
}

static void test_integration_empty_environment() {
    printf("\n=== Testing Integration: Empty Environment ===\n");
    
    const char* empty_env_json = 
    "{"
    "  \"id\": 3,"
    "  \"pathWidth\": 1.0,"
    "  \"pathOverlap\": 0.1,"
    "  \"boundary\": [],"
    "  \"obstacles\": []"
    "}";
    
    // This should handle empty environment gracefully
    coverage_path_planning_process(empty_env_json);
    
    printf("Empty environment integration test completed\n");
}

// Main test runner
int main() {
    printf("Starting Coverage Path Planning Integration Tests\n");
    printf("================================================\n");
    
    // Run integration tests
    test_integration_simple_rectangle();
    test_integration_rectangle_with_obstacle();
    test_integration_invalid_json();
    test_integration_null_input();
    test_integration_empty_environment();
    
    printf("\n================================================\n");
    printf("All integration tests completed successfully!\n");
    printf("Note: Check output above for any error messages or warnings.\n");
    
    return 0;
}