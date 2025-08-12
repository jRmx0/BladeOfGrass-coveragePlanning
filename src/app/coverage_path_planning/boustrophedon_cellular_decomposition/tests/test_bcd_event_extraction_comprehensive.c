#define BCD_TESTING
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "boustrophedon_cellular_decomposition.h"

// Minimal implementation of polygon_build_edges for testing
int polygon_build_edges(polygon_t *polygon) {
    if (!polygon) return -1;
    if (polygon->edges) { 
        free(polygon->edges); 
        polygon->edges = NULL; 
        polygon->edge_count = 0; 
    }

    if (!polygon->vertices || polygon->vertex_count < 2) {
        polygon->edges = NULL;
        polygon->edge_count = 0;
        return 0;
    }

    uint32_t n = polygon->vertex_count;
    polygon_edge_t *edges = (polygon_edge_t *)malloc((size_t)n * sizeof(polygon_edge_t));
    if (!edges) return -2;

    for (uint32_t i = 0; i < n; ++i) {
        uint32_t j = (i + 1u) % n;
        edges[i].begin = polygon->vertices[i];
        edges[i].end   = polygon->vertices[j];
    }

    polygon->edges = edges;
    polygon->edge_count = n;
    return 0;
}

// Test helper function to create a simple rectangular polygon
polygon_t create_rectangle_polygon(float x1, float y1, float x2, float y2, polygon_winding_t winding) {
    polygon_t poly;
    poly.winding = winding;
    poly.vertex_count = 4;
    poly.vertices = malloc(4 * sizeof(polygon_vertex_t));
    
    // Create vertices for rectangle (clockwise or counter-clockwise based on winding)
    if (winding == POLYGON_WINDING_CW) {
        // Clockwise: bottom-left, bottom-right, top-right, top-left
        poly.vertices[0] = (polygon_vertex_t){x1, y1}; // bottom-left
        poly.vertices[1] = (polygon_vertex_t){x2, y1}; // bottom-right
        poly.vertices[2] = (polygon_vertex_t){x2, y2}; // top-right
        poly.vertices[3] = (polygon_vertex_t){x1, y2}; // top-left
    } else {
        // Counter-clockwise: bottom-left, top-left, top-right, bottom-right
        poly.vertices[0] = (polygon_vertex_t){x1, y1}; // bottom-left
        poly.vertices[1] = (polygon_vertex_t){x1, y2}; // top-left
        poly.vertices[2] = (polygon_vertex_t){x2, y2}; // top-right
        poly.vertices[3] = (polygon_vertex_t){x2, y1}; // bottom-right
    }
    
    // Build edges
    poly.edge_count = 4;
    poly.edges = malloc(4 * sizeof(polygon_edge_t));
    
    int result = polygon_build_edges(&poly);
    if (result != 0) {
        printf("Error: Failed to build polygon edges\n");
        free(poly.vertices);
        free(poly.edges);
        poly.vertices = NULL;
        poly.edges = NULL;
        poly.vertex_count = 0;
        poly.edge_count = 0;
    }
    
    return poly;
}

// Test helper function to free polygon memory
void free_polygon(polygon_t *poly) {
    if (poly->vertices) {
        free(poly->vertices);
        poly->vertices = NULL;
    }
    if (poly->edges) {
        free(poly->edges);
        poly->edges = NULL;
    }
    poly->vertex_count = 0;
    poly->edge_count = 0;
}

// Test helper function to create a simple input environment
input_environment_t create_simple_environment() {
    input_environment_t env;
    env.id = 1;
    env.path_width = 1.0f;
    env.path_overlap = 0.1f;
    
    // Create boundary polygon (10x10 rectangle)
    env.boundary = create_rectangle_polygon(0.0f, 0.0f, 10.0f, 10.0f, POLYGON_WINDING_CW);
    
    // Create one obstacle (2x2 rectangle at center)
    env.obstacle_count = 1;
    env.obstacles = malloc(1 * sizeof(polygon_t));
    env.obstacles[0] = create_rectangle_polygon(4.0f, 4.0f, 6.0f, 6.0f, POLYGON_WINDING_CCW);
    
    return env;
}

// Test helper function to free input environment
void free_test_environment(input_environment_t *env) {
    free_polygon(&env->boundary);
    
    if (env->obstacles) {
        for (uint32_t i = 0; i < env->obstacle_count; i++) {
            free_polygon(&env->obstacles[i]);
        }
        free(env->obstacles);
        env->obstacles = NULL;
    }
    env->obstacle_count = 0;
}

// Test populate_event_geometry function directly
void test_populate_event_geometry_direct() {
    printf("Testing populate_event_geometry function directly...\n");
    
    // Create a simple rectangle polygon
    polygon_t rect = create_rectangle_polygon(0.0f, 0.0f, 2.0f, 2.0f, POLYGON_WINDING_CW);
    
    if (rect.vertices == NULL || rect.edges == NULL) {
        printf("Error: Failed to create test rectangle\n");
        return;
    }
    
    // Test event at vertex 0 (bottom-left corner)
    bcd_event_t event;
    event.polygon_type = BOUNDARY;
    event.polygon_vertex = rect.vertices[0];
    
    // Call the function directly through test wrapper
    int result = test_populate_event_geometry(&event, &rect, 0);
    assert(result == 0);
    
    // Verify floor edge (from vertex[0] to vertex[1])
    assert(fabs(event.floor_edge.begin.x - rect.vertices[0].x) < 0.001f);
    assert(fabs(event.floor_edge.begin.y - rect.vertices[0].y) < 0.001f);
    assert(fabs(event.floor_edge.end.x - rect.vertices[1].x) < 0.001f);
    assert(fabs(event.floor_edge.end.y - rect.vertices[1].y) < 0.001f);
    
    // Verify ceiling edge (from vertex[3] to vertex[0])
    assert(fabs(event.ceiling_edge.begin.x - rect.vertices[3].x) < 0.001f);
    assert(fabs(event.ceiling_edge.begin.y - rect.vertices[3].y) < 0.001f);
    assert(fabs(event.ceiling_edge.end.x - rect.vertices[0].x) < 0.001f);
    assert(fabs(event.ceiling_edge.end.y - rect.vertices[0].y) < 0.001f);
    
    // Test event at vertex 1 (bottom-right corner)
    bcd_event_t event1;
    event1.polygon_type = BOUNDARY;
    event1.polygon_vertex = rect.vertices[1];
    
    result = test_populate_event_geometry(&event1, &rect, 1);
    assert(result == 0);
    
    // Verify floor edge (from vertex[1] to vertex[2])
    assert(fabs(event1.floor_edge.begin.x - rect.vertices[1].x) < 0.001f);
    assert(fabs(event1.floor_edge.begin.y - rect.vertices[1].y) < 0.001f);
    assert(fabs(event1.floor_edge.end.x - rect.vertices[2].x) < 0.001f);
    assert(fabs(event1.floor_edge.end.y - rect.vertices[2].y) < 0.001f);
    
    // Verify ceiling edge (from vertex[0] to vertex[1])
    assert(fabs(event1.ceiling_edge.begin.x - rect.vertices[0].x) < 0.001f);
    assert(fabs(event1.ceiling_edge.begin.y - rect.vertices[0].y) < 0.001f);
    assert(fabs(event1.ceiling_edge.end.x - rect.vertices[1].x) < 0.001f);
    assert(fabs(event1.ceiling_edge.end.y - rect.vertices[1].y) < 0.001f);
    
    free_polygon(&rect);
    printf("populate_event_geometry direct tests passed!\n");
}

// Test extract_events_from_environment function directly
void test_extract_events_from_environment_direct() {
    printf("Testing extract_events_from_environment function directly...\n");
    
    // Create a simple test environment
    input_environment_t env = create_simple_environment();
    
    if (env.boundary.vertices == NULL || env.obstacles == NULL) {
        printf("Error: Failed to create test environment\n");
        free_test_environment(&env);
        return;
    }
    
    // Calculate expected number of events
    int expected_event_count = env.boundary.vertex_count + env.obstacles[0].vertex_count;
    assert(expected_event_count == 8); // 4 boundary + 4 obstacle vertices
    
    // Allocate memory for events
    bcd_event_t *events = malloc(expected_event_count * sizeof(bcd_event_t));
    int actual_event_count = 0;
    
    // Call the function directly through test wrapper
    int result = test_extract_events_from_environment(&env, events, &actual_event_count);
    assert(result == 0);
    assert(actual_event_count == expected_event_count);
    
    // Verify boundary events (first 4 events)
    for (int i = 0; i < 4; i++) {
        assert(events[i].polygon_type == BOUNDARY);
        assert(fabs(events[i].polygon_vertex.x - env.boundary.vertices[i].x) < 0.001f);
        assert(fabs(events[i].polygon_vertex.y - env.boundary.vertices[i].y) < 0.001f);
        
        // Verify floor and ceiling edges are properly set
        // Floor edge should be from current vertex to next vertex
        int next_vertex = (i + 1) % 4;
        assert(fabs(events[i].floor_edge.begin.x - env.boundary.vertices[i].x) < 0.001f);
        assert(fabs(events[i].floor_edge.begin.y - env.boundary.vertices[i].y) < 0.001f);
        assert(fabs(events[i].floor_edge.end.x - env.boundary.vertices[next_vertex].x) < 0.001f);
        assert(fabs(events[i].floor_edge.end.y - env.boundary.vertices[next_vertex].y) < 0.001f);
        
        // Ceiling edge should be from previous vertex to current vertex
        int prev_vertex = (i - 1 + 4) % 4;
        assert(fabs(events[i].ceiling_edge.begin.x - env.boundary.vertices[prev_vertex].x) < 0.001f);
        assert(fabs(events[i].ceiling_edge.begin.y - env.boundary.vertices[prev_vertex].y) < 0.001f);
        assert(fabs(events[i].ceiling_edge.end.x - env.boundary.vertices[i].x) < 0.001f);
        assert(fabs(events[i].ceiling_edge.end.y - env.boundary.vertices[i].y) < 0.001f);
    }
    
    // Verify obstacle events (next 4 events)
    for (int i = 4; i < 8; i++) {
        int obs_vertex_idx = i - 4;
        assert(events[i].polygon_type == OBSTACLE);
        assert(fabs(events[i].polygon_vertex.x - env.obstacles[0].vertices[obs_vertex_idx].x) < 0.001f);
        assert(fabs(events[i].polygon_vertex.y - env.obstacles[0].vertices[obs_vertex_idx].y) < 0.001f);
        
        // Verify floor and ceiling edges are properly set for obstacle
        int next_vertex = (obs_vertex_idx + 1) % 4;
        assert(fabs(events[i].floor_edge.begin.x - env.obstacles[0].vertices[obs_vertex_idx].x) < 0.001f);
        assert(fabs(events[i].floor_edge.begin.y - env.obstacles[0].vertices[obs_vertex_idx].y) < 0.001f);
        assert(fabs(events[i].floor_edge.end.x - env.obstacles[0].vertices[next_vertex].x) < 0.001f);
        assert(fabs(events[i].floor_edge.end.y - env.obstacles[0].vertices[next_vertex].y) < 0.001f);
        
        int prev_vertex = (obs_vertex_idx - 1 + 4) % 4;
        assert(fabs(events[i].ceiling_edge.begin.x - env.obstacles[0].vertices[prev_vertex].x) < 0.001f);
        assert(fabs(events[i].ceiling_edge.begin.y - env.obstacles[0].vertices[prev_vertex].y) < 0.001f);
        assert(fabs(events[i].ceiling_edge.end.x - env.obstacles[0].vertices[obs_vertex_idx].x) < 0.001f);
        assert(fabs(events[i].ceiling_edge.end.y - env.obstacles[0].vertices[obs_vertex_idx].y) < 0.001f);
    }
    
    free(events);
    free_test_environment(&env);
    printf("extract_events_from_environment direct tests passed!\n");
}

// Test error handling with direct function calls
void test_error_handling_direct() {
    printf("Testing error handling with direct function calls...\n");
    
    // Test populate_event_geometry with NULL inputs
    bcd_event_t event;
    polygon_t poly = create_rectangle_polygon(0.0f, 0.0f, 1.0f, 1.0f, POLYGON_WINDING_CW);
    
    int result = test_populate_event_geometry(NULL, &poly, 0);
    assert(result == -1); // Should return error for NULL event
    
    result = test_populate_event_geometry(&event, NULL, 0);
    assert(result == -1); // Should return error for NULL polygon
    
    result = test_populate_event_geometry(&event, &poly, -1);
    assert(result == -1); // Should return error for invalid vertex index
    
    result = test_populate_event_geometry(&event, &poly, 10);
    assert(result == -1); // Should return error for out-of-bounds vertex index
    
    // Test with polygon that has no edges built
    polygon_t invalid_poly;
    invalid_poly.vertices = malloc(3 * sizeof(polygon_vertex_t));
    invalid_poly.vertex_count = 3;
    invalid_poly.edges = NULL; // No edges built
    invalid_poly.edge_count = 0;
    
    result = test_populate_event_geometry(&event, &invalid_poly, 0);
    assert(result == -3); // Should return error for missing edges
    
    // Test extract_events_from_environment with NULL inputs
    bcd_event_t events[10];
    int event_count;
    input_environment_t env = create_simple_environment();
    
    result = test_extract_events_from_environment(NULL, events, &event_count);
    assert(result == -1); // Should return error for NULL environment
    
    result = test_extract_events_from_environment(&env, NULL, &event_count);
    assert(result == -1); // Should return error for NULL events array
    
    result = test_extract_events_from_environment(&env, events, NULL);
    assert(result == -1); // Should return error for NULL event_count
    
    // Test with environment that has polygon without edges
    input_environment_t invalid_env;
    invalid_env.boundary = invalid_poly;
    invalid_env.obstacles = NULL;
    invalid_env.obstacle_count = 0;
    
    result = test_extract_events_from_environment(&invalid_env, events, &event_count);
    assert(result == -3); // Should return error for polygon without edges
    
    free(invalid_poly.vertices);
    free_polygon(&poly);
    free_test_environment(&env);
    printf("Error handling direct tests passed!\n");
}

// Test with empty environment
void test_empty_environment() {
    printf("Testing with empty environment...\n");
    
    input_environment_t empty_env;
    empty_env.id = 1;
    empty_env.path_width = 1.0f;
    empty_env.path_overlap = 0.1f;
    
    // Empty boundary
    empty_env.boundary.vertices = NULL;
    empty_env.boundary.vertex_count = 0;
    empty_env.boundary.edges = NULL;
    empty_env.boundary.edge_count = 0;
    empty_env.boundary.winding = POLYGON_WINDING_CW;
    
    // No obstacles
    empty_env.obstacles = NULL;
    empty_env.obstacle_count = 0;
    
    bcd_event_t events[10];
    int event_count = 0;
    
    int result = test_extract_events_from_environment(&empty_env, events, &event_count);
    assert(result == 0); // Should succeed
    assert(event_count == 0); // Should have no events
    
    printf("Empty environment tests passed!\n");
}

// Main test function
int main() {
    printf("Starting Comprehensive BCD Event Extraction Tests...\n\n");
    
    test_populate_event_geometry_direct();
    test_extract_events_from_environment_direct();
    test_error_handling_direct();
    test_empty_environment();
    
    printf("\nAll comprehensive BCD Event Extraction tests passed!\n");
    return 0;
}