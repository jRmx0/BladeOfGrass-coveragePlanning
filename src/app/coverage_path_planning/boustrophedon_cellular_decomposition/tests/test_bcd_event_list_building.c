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

// Test helper function to create environment with multiple obstacles
input_environment_t create_complex_environment() {
    input_environment_t env;
    env.id = 2;
    env.path_width = 1.0f;
    env.path_overlap = 0.1f;
    
    // Create boundary polygon (20x15 rectangle)
    env.boundary = create_rectangle_polygon(0.0f, 0.0f, 20.0f, 15.0f, POLYGON_WINDING_CW);
    
    // Create multiple obstacles
    env.obstacle_count = 3;
    env.obstacles = malloc(3 * sizeof(polygon_t));
    env.obstacles[0] = create_rectangle_polygon(2.0f, 2.0f, 4.0f, 4.0f, POLYGON_WINDING_CCW);
    env.obstacles[1] = create_rectangle_polygon(8.0f, 6.0f, 10.0f, 8.0f, POLYGON_WINDING_CCW);
    env.obstacles[2] = create_rectangle_polygon(15.0f, 10.0f, 17.0f, 12.0f, POLYGON_WINDING_CCW);
    
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

// Test sorting functionality
void test_event_sorting() {
    printf("Testing event sorting by x-coordinate...\n");
    
    // Create test events with different x-coordinates
    bcd_event_t events[5];
    events[0].polygon_vertex = (polygon_vertex_t){5.0f, 1.0f};
    events[1].polygon_vertex = (polygon_vertex_t){2.0f, 2.0f};
    events[2].polygon_vertex = (polygon_vertex_t){8.0f, 3.0f};
    events[3].polygon_vertex = (polygon_vertex_t){1.0f, 4.0f};
    events[4].polygon_vertex = (polygon_vertex_t){6.0f, 5.0f};
    
    // Sort events
    int result = test_sort_events_by_x_coordinate(events, 5);
    assert(result == 0);
    
    // Verify sorting (should be in ascending x order: 1.0, 2.0, 5.0, 6.0, 8.0)
    assert(events[0].polygon_vertex.x == 1.0f);
    assert(events[1].polygon_vertex.x == 2.0f);
    assert(events[2].polygon_vertex.x == 5.0f);
    assert(events[3].polygon_vertex.x == 6.0f);
    assert(events[4].polygon_vertex.x == 8.0f);
    
    printf("Event sorting tests passed!\n");
}

// Test constraint validation
void test_constraint_validation() {
    printf("Testing constraint validation...\n");
    
    // Test valid events (no duplicate IN/OUT at same x)
    bcd_event_t valid_events[4];
    valid_events[0].polygon_vertex = (polygon_vertex_t){1.0f, 1.0f};
    valid_events[0].bcd_event_type = IN;
    valid_events[1].polygon_vertex = (polygon_vertex_t){2.0f, 2.0f};
    valid_events[1].bcd_event_type = OUT;
    valid_events[2].polygon_vertex = (polygon_vertex_t){3.0f, 3.0f};
    valid_events[2].bcd_event_type = SIDE_IN;
    valid_events[3].polygon_vertex = (polygon_vertex_t){4.0f, 4.0f};
    valid_events[3].bcd_event_type = SIDE_OUT;
    
    int result = test_validate_event_constraints(valid_events, 4);
    assert(result == 0); // Should pass validation
    
    // Test invalid events (duplicate IN events at same x)
    bcd_event_t invalid_events[3];
    invalid_events[0].polygon_vertex = (polygon_vertex_t){1.0f, 1.0f};
    invalid_events[0].bcd_event_type = IN;
    invalid_events[1].polygon_vertex = (polygon_vertex_t){1.0f, 2.0f}; // Same x-coordinate
    invalid_events[1].bcd_event_type = IN; // Duplicate IN event
    invalid_events[2].polygon_vertex = (polygon_vertex_t){3.0f, 3.0f};
    invalid_events[2].bcd_event_type = OUT;
    
    result = test_validate_event_constraints(invalid_events, 3);
    assert(result == -6); // Should fail validation
    
    // Test invalid events (duplicate OUT events at same x)
    invalid_events[0].bcd_event_type = OUT;
    invalid_events[1].bcd_event_type = OUT; // Duplicate OUT event
    
    result = test_validate_event_constraints(invalid_events, 3);
    assert(result == -6); // Should fail validation
    
    printf("Constraint validation tests passed!\n");
}

// Test boundary extremes detection
void test_boundary_extremes() {
    printf("Testing boundary extremes detection...\n");
    
    // Create a polygon with known leftmost and rightmost vertices
    polygon_t poly;
    poly.vertex_count = 5;
    poly.vertices = malloc(5 * sizeof(polygon_vertex_t));
    poly.vertices[0] = (polygon_vertex_t){3.0f, 1.0f};
    poly.vertices[1] = (polygon_vertex_t){1.0f, 2.0f}; // Leftmost
    poly.vertices[2] = (polygon_vertex_t){5.0f, 3.0f};
    poly.vertices[3] = (polygon_vertex_t){8.0f, 4.0f}; // Rightmost
    poly.vertices[4] = (polygon_vertex_t){2.0f, 5.0f};
    
    int leftmost_idx, rightmost_idx;
    int result = test_find_boundary_extremes(&poly, &leftmost_idx, &rightmost_idx);
    assert(result == 0);
    assert(leftmost_idx == 1); // Vertex at x=1.0
    assert(rightmost_idx == 3); // Vertex at x=8.0
    
    free(poly.vertices);
    printf("Boundary extremes detection tests passed!\n");
}

// Test complete event list building with simple environment
void test_build_event_list_simple() {
    printf("Testing build_bcd_event_list with simple environment...\n");
    
    input_environment_t env = create_simple_environment();
    
    bcd_event_t *event_list = NULL;
    int event_count = 0;
    
    int result = build_bcd_event_list(&env, &event_list, &event_count);
    assert(result == 0); // Should succeed
    assert(event_list != NULL);
    assert(event_count == 8); // 4 boundary + 4 obstacle vertices
    
    // Verify events are sorted by x-coordinate
    for (int i = 0; i < event_count - 1; i++) {
        assert(event_list[i].polygon_vertex.x <= event_list[i + 1].polygon_vertex.x);
    }
    
    // Verify all events have proper polygon types
    int boundary_count = 0, obstacle_count = 0;
    for (int i = 0; i < event_count; i++) {
        if (event_list[i].polygon_type == BOUNDARY) {
            boundary_count++;
        } else if (event_list[i].polygon_type == OBSTACLE) {
            obstacle_count++;
        }
    }
    assert(boundary_count == 4);
    assert(obstacle_count == 4);
    
    // Verify boundary events include BOUND_INIT and BOUND_DEINIT
    int has_bound_init = 0, has_bound_deinit = 0;
    for (int i = 0; i < event_count; i++) {
        if (event_list[i].polygon_type == BOUNDARY) {
            if (event_list[i].bcd_event_type == BOUND_INIT) {
                has_bound_init = 1;
            }
            if (event_list[i].bcd_event_type == BOUND_DEINIT) {
                has_bound_deinit = 1;
            }
        }
    }
    assert(has_bound_init == 1);
    assert(has_bound_deinit == 1);
    
    free(event_list);
    free_test_environment(&env);
    printf("Simple environment event list building tests passed!\n");
}

// Test complete event list building with complex environment
void test_build_event_list_complex() {
    printf("Testing build_bcd_event_list with complex environment...\n");
    
    input_environment_t env = create_complex_environment();
    
    bcd_event_t *event_list = NULL;
    int event_count = 0;
    
    int result = build_bcd_event_list(&env, &event_list, &event_count);
    assert(result == 0); // Should succeed
    assert(event_list != NULL);
    assert(event_count == 16); // 4 boundary + 3*4 obstacle vertices
    
    // Verify events are sorted by x-coordinate
    for (int i = 0; i < event_count - 1; i++) {
        assert(event_list[i].polygon_vertex.x <= event_list[i + 1].polygon_vertex.x);
    }
    
    // Verify all events have proper polygon types
    int boundary_count = 0, obstacle_count = 0;
    for (int i = 0; i < event_count; i++) {
        if (event_list[i].polygon_type == BOUNDARY) {
            boundary_count++;
        } else if (event_list[i].polygon_type == OBSTACLE) {
            obstacle_count++;
        }
    }
    assert(boundary_count == 4);
    assert(obstacle_count == 12);
    
    free(event_list);
    free_test_environment(&env);
    printf("Complex environment event list building tests passed!\n");
}

// Test error handling
void test_error_handling() {
    printf("Testing error handling...\n");
    
    bcd_event_t *event_list = NULL;
    int event_count = 0;
    
    // Test with NULL environment
    int result = build_bcd_event_list(NULL, &event_list, &event_count);
    assert(result == -1); // Should return invalid input error
    
    // Test with NULL event_list pointer
    input_environment_t env = create_simple_environment();
    result = build_bcd_event_list(&env, NULL, &event_count);
    assert(result == -1); // Should return invalid input error
    
    // Test with NULL event_count pointer
    result = build_bcd_event_list(&env, &event_list, NULL);
    assert(result == -1); // Should return invalid input error
    
    // Test with environment that has polygon without edges
    input_environment_t invalid_env;
    invalid_env.id = 1;
    invalid_env.path_width = 1.0f;
    invalid_env.path_overlap = 0.1f;
    invalid_env.boundary.vertices = malloc(3 * sizeof(polygon_vertex_t));
    invalid_env.boundary.vertex_count = 3;
    invalid_env.boundary.edges = NULL; // No edges built
    invalid_env.boundary.edge_count = 0;
    invalid_env.boundary.winding = POLYGON_WINDING_CW;
    invalid_env.obstacles = NULL;
    invalid_env.obstacle_count = 0;
    
    result = build_bcd_event_list(&invalid_env, &event_list, &event_count);
    assert(result == -3); // Should return polygon validation error
    
    free(invalid_env.boundary.vertices);
    free_test_environment(&env);
    printf("Error handling tests passed!\n");
}

// Test empty environment
void test_empty_environment() {
    printf("Testing empty environment...\n");
    
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
    
    bcd_event_t *event_list = NULL;
    int event_count = 0;
    
    int result = build_bcd_event_list(&empty_env, &event_list, &event_count);
    assert(result == 0); // Should succeed
    assert(event_list == NULL); // Should be NULL for empty environment
    assert(event_count == 0); // Should have no events
    
    printf("Empty environment tests passed!\n");
}

// Main test function
int main() {
    printf("Starting BCD Event List Building Integration Tests...\n\n");
    
    test_event_sorting();
    test_constraint_validation();
    test_boundary_extremes();
    test_build_event_list_simple();
    test_build_event_list_complex();
    test_error_handling();
    test_empty_environment();
    
    printf("\nAll BCD Event List Building integration tests passed!\n");
    return 0;
}