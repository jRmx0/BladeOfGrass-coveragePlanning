#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "boustrophedon_cellular_decomposition.h"

// Test tolerance for floating point comparisons
#define FLOAT_TOLERANCE 0.001f

// Helper function to compare floats with tolerance
static int float_equals(float a, float b) {
    return fabs(a - b) < FLOAT_TOLERANCE;
}

// Test compute_edge_angle with known test cases
void test_compute_edge_angle() {
    printf("Running compute_edge_angle tests...\n");
    
    polygon_edge_t edge;
    float result;
    
    // Test 1: Horizontal edge pointing right (0 degrees)
    edge.begin = (polygon_vertex_t){0.0f, 0.0f};
    edge.end = (polygon_vertex_t){1.0f, 0.0f};
    result = compute_edge_angle(edge);
    assert(float_equals(result, 0.0f));
    printf("✓ Test 1 passed: Horizontal right edge = %.3f degrees\n", result);
    
    // Test 2: Vertical edge pointing up (90 degrees)
    edge.begin = (polygon_vertex_t){0.0f, 0.0f};
    edge.end = (polygon_vertex_t){0.0f, 1.0f};
    result = compute_edge_angle(edge);
    assert(float_equals(result, 90.0f));
    printf("✓ Test 2 passed: Vertical up edge = %.3f degrees\n", result);
    
    // Test 3: Horizontal edge pointing left (180 degrees)
    edge.begin = (polygon_vertex_t){1.0f, 0.0f};
    edge.end = (polygon_vertex_t){0.0f, 0.0f};
    result = compute_edge_angle(edge);
    assert(float_equals(result, 180.0f));
    printf("✓ Test 3 passed: Horizontal left edge = %.3f degrees\n", result);
    
    // Test 4: Vertical edge pointing down (270 degrees)
    edge.begin = (polygon_vertex_t){0.0f, 1.0f};
    edge.end = (polygon_vertex_t){0.0f, 0.0f};
    result = compute_edge_angle(edge);
    assert(float_equals(result, 270.0f));
    printf("✓ Test 4 passed: Vertical down edge = %.3f degrees\n", result);
    
    // Test 5: Diagonal edge (45 degrees)
    edge.begin = (polygon_vertex_t){0.0f, 0.0f};
    edge.end = (polygon_vertex_t){1.0f, 1.0f};
    result = compute_edge_angle(edge);
    assert(float_equals(result, 45.0f));
    printf("✓ Test 5 passed: Diagonal 45° edge = %.3f degrees\n", result);
    
    // Test 6: Diagonal edge (135 degrees)
    edge.begin = (polygon_vertex_t){1.0f, 0.0f};
    edge.end = (polygon_vertex_t){0.0f, 1.0f};
    result = compute_edge_angle(edge);
    assert(float_equals(result, 135.0f));
    printf("✓ Test 6 passed: Diagonal 135° edge = %.3f degrees\n", result);
    
    // Test 7: Diagonal edge (225 degrees)
    edge.begin = (polygon_vertex_t){1.0f, 1.0f};
    edge.end = (polygon_vertex_t){0.0f, 0.0f};
    result = compute_edge_angle(edge);
    assert(float_equals(result, 225.0f));
    printf("✓ Test 7 passed: Diagonal 225° edge = %.3f degrees\n", result);
    
    // Test 8: Diagonal edge (315 degrees)
    edge.begin = (polygon_vertex_t){0.0f, 1.0f};
    edge.end = (polygon_vertex_t){1.0f, 0.0f};
    result = compute_edge_angle(edge);
    assert(float_equals(result, 315.0f));
    printf("✓ Test 8 passed: Diagonal 315° edge = %.3f degrees\n", result);
    
    // Test 9: Zero-length edge (should return 0)
    edge.begin = (polygon_vertex_t){5.0f, 3.0f};
    edge.end = (polygon_vertex_t){5.0f, 3.0f};
    result = compute_edge_angle(edge);
    assert(float_equals(result, 0.0f));
    printf("✓ Test 9 passed: Zero-length edge = %.3f degrees\n", result);
    
    // Test 10: Edge with decimal coordinates
    edge.begin = (polygon_vertex_t){1.5f, 2.3f};
    edge.end = (polygon_vertex_t){4.7f, 6.1f};
    result = compute_edge_angle(edge);
    // Expected angle: atan2(6.1-2.3, 4.7-1.5) = atan2(3.8, 3.2) ≈ 49.87°
    assert(result >= 0.0f && result < 360.0f);
    printf("✓ Test 10 passed: Decimal coordinates edge = %.3f degrees\n", result);
    
    // Test 11: Very small edge (precision test)
    edge.begin = (polygon_vertex_t){0.0f, 0.0f};
    edge.end = (polygon_vertex_t){0.001f, 0.001f};
    result = compute_edge_angle(edge);
    assert(float_equals(result, 45.0f));
    printf("✓ Test 11 passed: Very small edge = %.3f degrees\n", result);
    
    // Test 12: Large coordinates
    edge.begin = (polygon_vertex_t){1000.0f, 2000.0f};
    edge.end = (polygon_vertex_t){3000.0f, 2000.0f};
    result = compute_edge_angle(edge);
    assert(float_equals(result, 0.0f));
    printf("✓ Test 12 passed: Large coordinates edge = %.3f degrees\n", result);
    
    printf("All compute_edge_angle tests passed!\n\n");
}

// Test helper geometric functions
void test_helper_functions() {
    printf("Running helper function tests...\n");
    
    // Test angle normalization (indirectly through compute_edge_angle)
    polygon_edge_t edge;
    float result;
    
    // Test that angles are properly normalized to [0, 360)
    edge.begin = (polygon_vertex_t){0.0f, 0.0f};
    edge.end = (polygon_vertex_t){-1.0f, -1.0f};
    result = compute_edge_angle(edge);
    assert(result >= 0.0f && result < 360.0f);
    assert(float_equals(result, 225.0f));
    printf("✓ Angle normalization test passed: %.3f degrees\n", result);
    
    printf("All helper function tests passed!\n\n");
}

int main() {
    printf("=== BCD Geometric Utility Functions Test Suite ===\n\n");
    
    test_compute_edge_angle();
    test_helper_functions();
    
    printf("=== All tests completed successfully! ===\n");
    return 0;
}