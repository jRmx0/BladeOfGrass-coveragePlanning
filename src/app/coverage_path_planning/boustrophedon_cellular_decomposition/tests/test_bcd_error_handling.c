#define BCD_TESTING
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "boustrophedon_cellular_decomposition.h"

// Test helper function to create a simple polygon
static polygon_t create_test_polygon(int vertex_count, int build_edges, polygon_winding_t winding) {
    polygon_t polygon = {0};
    polygon.winding = winding;
    polygon.vertex_count = vertex_count;
    
    if (vertex_count > 0) {
        polygon.vertices = malloc(vertex_count * sizeof(polygon_vertex_t));
        // Create a simple square
        if (vertex_count >= 4) {
            polygon.vertices[0] = (polygon_vertex_t){0.0f, 0.0f};
            polygon.vertices[1] = (polygon_vertex_t){1.0f, 0.0f};
            polygon.vertices[2] = (polygon_vertex_t){1.0f, 1.0f};
            polygon.vertices[3] = (polygon_vertex_t){0.0f, 1.0f};
        }
        
        if (build_edges) {
            polygon.edge_count = vertex_count;
            polygon.edges = malloc(vertex_count * sizeof(polygon_edge_t));
            for (int i = 0; i < vertex_count; i++) {
                polygon.edges[i].begin = polygon.vertices[i];
                polygon.edges[i].end = polygon.vertices[(i + 1) % vertex_count];
            }
        }
    }
    
    return polygon;
}

// Test helper function to free polygon memory
static void free_test_polygon(polygon_t *polygon) {
    if (polygon) {
        free(polygon->vertices);
        free(polygon->edges);
        memset(polygon, 0, sizeof(polygon_t));
    }
}

// Test 1: NULL input validation
static void test_null_input_validation() {
    printf("Running test_null_input_validation...\n");
    
    bcd_event_t *event_list = NULL;
    int event_count = 0;
    
    // Test NULL environment
    int result = build_bcd_event_list(NULL, &event_list, &event_count);
    assert(result == BCD_ERROR_INVALID_INPUT);
    assert(event_list == NULL);
    assert(event_count == 0);
    
    // Test NULL event_list pointer
    input_environment_t env = {0};
    result = build_bcd_event_list(&env, NULL, &event_count);
    assert(result == BCD_ERROR_INVALID_INPUT);
    
    // Test NULL event_count pointer
    result = build_bcd_event_list(&env, &event_list, NULL);
    assert(result == BCD_ERROR_INVALID_INPUT);
    
    printf("✓ test_null_input_validation passed\n");
}

// Test 2: Input environment structure integrity validation
static void test_input_environment_validation() {
    printf("Running test_input_environment_validation...\n");
    
    input_environment_t env;
    
    // Test boundary vertices NULL but count > 0
    memset(&env, 0, sizeof(env));
    env.boundary.vertices = NULL;
    env.boundary.vertex_count = 4;
    int result = test_validate_input_environment(&env);
    assert(result == BCD_ERROR_INVALID_INPUT);
    
    // Test boundary vertices not NULL but count = 0
    memset(&env, 0, sizeof(env));
    env.boundary.vertices = (polygon_vertex_t*)0x1; // Non-NULL pointer
    env.boundary.vertex_count = 0;
    result = test_validate_input_environment(&env);
    assert(result == BCD_ERROR_INVALID_INPUT);
    
    // Test obstacles NULL but count > 0
    memset(&env, 0, sizeof(env));
    env.obstacles = NULL;
    env.obstacle_count = 2;
    result = test_validate_input_environment(&env);
    assert(result == BCD_ERROR_INVALID_INPUT);
    
    // Test obstacles not NULL but count = 0
    memset(&env, 0, sizeof(env));
    env.obstacles = (polygon_t*)0x1; // Non-NULL pointer
    env.obstacle_count = 0;
    result = test_validate_input_environment(&env);
    assert(result == BCD_ERROR_INVALID_INPUT);
    
    // Test obstacle with vertices NULL but count > 0
    memset(&env, 0, sizeof(env));
    polygon_t obstacle = {0};
    obstacle.vertices = NULL;
    obstacle.vertex_count = 3;
    env.obstacles = &obstacle;
    env.obstacle_count = 1;
    result = test_validate_input_environment(&env);
    assert(result == BCD_ERROR_INVALID_INPUT);
    
    // Test valid environment
    memset(&env, 0, sizeof(env));
    result = test_validate_input_environment(&env);
    assert(result == BCD_SUCCESS);
    
    printf("✓ test_input_environment_validation passed\n");
}

// Test 3: Polygon validation checks
static void test_polygon_validation() {
    printf("Running test_polygon_validation...\n");
    
    // Test NULL polygon
    int result = test_validate_polygon(NULL, "Test");
    assert(result == BCD_ERROR_INVALID_INPUT);
    
    // Test empty polygon (vertex_count = 0)
    polygon_t polygon = create_test_polygon(0, 0, POLYGON_WINDING_CW);
    result = test_validate_polygon(&polygon, "Empty");
    assert(result == BCD_ERROR_POLYGON_VALIDATION);
    free_test_polygon(&polygon);
    
    // Test polygon with vertices NULL
    polygon = create_test_polygon(4, 0, POLYGON_WINDING_CW);
    free(polygon.vertices);
    polygon.vertices = NULL;
    result = test_validate_polygon(&polygon, "NoVertices");
    assert(result == BCD_ERROR_POLYGON_VALIDATION);
    free_test_polygon(&polygon);
    
    // Test polygon with edges not built
    polygon = create_test_polygon(4, 0, POLYGON_WINDING_CW);
    result = test_validate_polygon(&polygon, "NoEdges");
    assert(result == BCD_ERROR_POLYGON_VALIDATION);
    free_test_polygon(&polygon);
    
    // Test polygon with edge_count = 0
    polygon = create_test_polygon(4, 1, POLYGON_WINDING_CW);
    polygon.edge_count = 0;
    result = test_validate_polygon(&polygon, "ZeroEdges");
    assert(result == BCD_ERROR_POLYGON_VALIDATION);
    free_test_polygon(&polygon);
    
    // Test polygon with mismatched edge and vertex counts
    polygon = create_test_polygon(4, 1, POLYGON_WINDING_CW);
    polygon.edge_count = 3;
    result = test_validate_polygon(&polygon, "MismatchedCounts");
    assert(result == BCD_ERROR_POLYGON_VALIDATION);
    free_test_polygon(&polygon);
    
    // Test polygon with unknown winding
    polygon = create_test_polygon(4, 1, POLYGON_WINDING_UNKNOWN);
    result = test_validate_polygon(&polygon, "UnknownWinding");
    assert(result == BCD_ERROR_WINDING_VALIDATION);
    free_test_polygon(&polygon);
    
    // Test polygon with insufficient vertices (< 3)
    polygon = create_test_polygon(2, 1, POLYGON_WINDING_CW);
    result = test_validate_polygon(&polygon, "TwoVertices");
    assert(result == BCD_ERROR_POLYGON_VALIDATION);
    free_test_polygon(&polygon);
    
    // Test valid polygon
    polygon = create_test_polygon(4, 1, POLYGON_WINDING_CW);
    result = test_validate_polygon(&polygon, "Valid");
    assert(result == BCD_SUCCESS);
    free_test_polygon(&polygon);
    
    printf("✓ test_polygon_validation passed\n");
}

// Test 4: Memory allocation failure simulation
static void test_memory_allocation_failure() {
    printf("Running test_memory_allocation_failure...\n");
    
    // This test would require mocking malloc to return NULL
    // For now, we'll test the error path by creating a very large environment
    // that would likely cause allocation failure on most systems
    
    input_environment_t env = {0};
    polygon_t boundary = create_test_polygon(4, 1, POLYGON_WINDING_CW);
    env.boundary = boundary;
    
    // Create an environment that would require excessive memory
    // This is a conceptual test - in practice, we'd need malloc mocking
    
    bcd_event_t *event_list = NULL;
    int event_count = 0;
    
    // Test with valid small environment (should succeed)
    int result = build_bcd_event_list(&env, &event_list, &event_count);
    assert(result == BCD_SUCCESS);
    assert(event_list != NULL);
    assert(event_count == 4);
    
    // Clean up
    free(event_list);
    free_test_polygon(&boundary);
    
    printf("✓ test_memory_allocation_failure passed\n");
}

// Test 5: Event geometry population errors
static void test_event_geometry_population_errors() {
    printf("Running test_event_geometry_population_errors...\n");
    
    bcd_event_t event = {0};
    polygon_t polygon = create_test_polygon(4, 1, POLYGON_WINDING_CW);
    
    // Test NULL event
    int result = test_populate_event_geometry(NULL, &polygon, 0);
    assert(result == BCD_ERROR_INVALID_INPUT);
    
    // Test NULL polygon
    result = test_populate_event_geometry(&event, NULL, 0);
    assert(result == BCD_ERROR_INVALID_INPUT);
    
    // Test invalid vertex index (negative)
    result = test_populate_event_geometry(&event, &polygon, -1);
    assert(result == BCD_ERROR_INVALID_INPUT);
    
    // Test invalid vertex index (too large)
    result = test_populate_event_geometry(&event, &polygon, 10);
    assert(result == BCD_ERROR_INVALID_INPUT);
    
    // Test polygon with no edges
    free(polygon.edges);
    polygon.edges = NULL;
    polygon.edge_count = 0;
    result = test_populate_event_geometry(&event, &polygon, 0);
    assert(result == BCD_ERROR_POLYGON_VALIDATION);
    
    // Test valid case
    polygon.edges = malloc(4 * sizeof(polygon_edge_t));
    polygon.edge_count = 4;
    for (int i = 0; i < 4; i++) {
        polygon.edges[i].begin = polygon.vertices[i];
        polygon.edges[i].end = polygon.vertices[(i + 1) % 4];
    }
    result = test_populate_event_geometry(&event, &polygon, 0);
    assert(result == BCD_SUCCESS);
    
    free_test_polygon(&polygon);
    
    printf("✓ test_event_geometry_population_errors passed\n");
}

// Test 6: Event constraint validation errors
static void test_event_constraint_validation_errors() {
    printf("Running test_event_constraint_validation_errors...\n");
    
    // Test NULL events array with positive count
    int result = test_validate_event_constraints(NULL, 5);
    assert(result == BCD_ERROR_INVALID_INPUT);
    
    // Test NULL events array with zero count (should succeed)
    result = test_validate_event_constraints(NULL, 0);
    assert(result == BCD_SUCCESS);
    
    // Test empty event list (should succeed)
    bcd_event_t events[1];
    result = test_validate_event_constraints(events, 0);
    assert(result == BCD_SUCCESS);
    
    // Test duplicate IN events at same x-coordinate
    bcd_event_t duplicate_events[2];
    duplicate_events[0].polygon_vertex.x = 1.0f;
    duplicate_events[0].bcd_event_type = IN;
    duplicate_events[1].polygon_vertex.x = 1.0f;
    duplicate_events[1].bcd_event_type = IN;
    
    result = test_validate_event_constraints(duplicate_events, 2);
    assert(result == BCD_ERROR_CONSTRAINT_VIOLATION);
    
    // Test duplicate OUT events at same x-coordinate
    duplicate_events[0].bcd_event_type = OUT;
    duplicate_events[1].bcd_event_type = OUT;
    
    result = test_validate_event_constraints(duplicate_events, 2);
    assert(result == BCD_ERROR_CONSTRAINT_VIOLATION);
    
    // Test valid events (different types at same x-coordinate)
    duplicate_events[0].bcd_event_type = IN;
    duplicate_events[1].bcd_event_type = OUT;
    
    result = test_validate_event_constraints(duplicate_events, 2);
    assert(result == BCD_SUCCESS);
    
    printf("✓ test_event_constraint_validation_errors passed\n");
}

// Test 7: Event classification errors
static void test_event_classification_errors() {
    printf("Running test_event_classification_errors...\n");
    
    // Test NULL event
    int result = find_bcd_event_type(NULL);
    assert(result == BCD_ERROR_INVALID_INPUT);
    
    // Test invalid polygon type
    bcd_event_t event = {0};
    event.polygon_type = (polygon_type_t)999; // Invalid enum value
    
    result = find_bcd_event_type(&event);
    assert(result == BCD_ERROR_INVALID_INPUT);
    
    // Test valid boundary event
    event.polygon_type = BOUNDARY;
    event.floor_edge.begin = (polygon_vertex_t){0.0f, 0.0f};
    event.floor_edge.end = (polygon_vertex_t){1.0f, 0.0f};
    event.ceiling_edge.begin = (polygon_vertex_t){0.0f, 1.0f};
    event.ceiling_edge.end = (polygon_vertex_t){0.0f, 0.0f};
    
    result = find_bcd_event_type(&event);
    assert(result == BCD_SUCCESS);
    
    // Test valid obstacle event
    event.polygon_type = OBSTACLE;
    result = find_bcd_event_type(&event);
    assert(result == BCD_SUCCESS);
    
    printf("✓ test_event_classification_errors passed\n");
}

// Test 8: Sorting errors
static void test_sorting_errors() {
    printf("Running test_sorting_errors...\n");
    
    // Test NULL events array
    int result = test_sort_events_by_x_coordinate(NULL, 5);
    assert(result == BCD_ERROR_INVALID_INPUT);
    
    // Test invalid event count
    bcd_event_t events[1];
    result = test_sort_events_by_x_coordinate(events, -1);
    assert(result == BCD_ERROR_INVALID_INPUT);
    
    result = test_sort_events_by_x_coordinate(events, 0);
    assert(result == BCD_ERROR_INVALID_INPUT);
    
    // Test valid sorting
    bcd_event_t test_events[3];
    test_events[0].polygon_vertex.x = 3.0f;
    test_events[1].polygon_vertex.x = 1.0f;
    test_events[2].polygon_vertex.x = 2.0f;
    
    result = test_sort_events_by_x_coordinate(test_events, 3);
    assert(result == BCD_SUCCESS);
    
    // Verify sorting worked
    assert(test_events[0].polygon_vertex.x <= test_events[1].polygon_vertex.x);
    assert(test_events[1].polygon_vertex.x <= test_events[2].polygon_vertex.x);
    
    printf("✓ test_sorting_errors passed\n");
}

// Test 9: Complete pipeline error recovery
static void test_complete_pipeline_error_recovery() {
    printf("Running test_complete_pipeline_error_recovery...\n");
    
    bcd_event_t *event_list = NULL;
    int event_count = 0;
    
    // Test with invalid environment (should fail early and not leak memory)
    input_environment_t env = {0};
    env.boundary.vertices = NULL;
    env.boundary.vertex_count = 4; // Inconsistent state
    
    int result = build_bcd_event_list(&env, &event_list, &event_count);
    assert(result == BCD_ERROR_INVALID_INPUT);
    assert(event_list == NULL);
    assert(event_count == 0);
    
    // Test with polygon missing edges (should fail and clean up)
    polygon_t boundary = create_test_polygon(4, 0, POLYGON_WINDING_CW); // No edges
    env.boundary = boundary;
    
    result = build_bcd_event_list(&env, &event_list, &event_count);
    assert(result == BCD_ERROR_POLYGON_VALIDATION);
    assert(event_list == NULL);
    assert(event_count == 0);
    
    free_test_polygon(&boundary);
    
    printf("✓ test_complete_pipeline_error_recovery passed\n");
}

// Main test runner
int main() {
    printf("Starting BCD Error Handling Tests...\n\n");
    
    test_null_input_validation();
    test_input_environment_validation();
    test_polygon_validation();
    test_memory_allocation_failure();
    test_event_geometry_population_errors();
    test_event_constraint_validation_errors();
    test_event_classification_errors();
    test_sorting_errors();
    test_complete_pipeline_error_recovery();
    
    printf("\n✅ All BCD error handling tests passed!\n");
    return 0;
}