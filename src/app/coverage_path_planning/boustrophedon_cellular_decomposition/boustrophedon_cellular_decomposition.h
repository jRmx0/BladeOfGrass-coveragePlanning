// Generates events list, cells and adjacency graph using Boustrophedon Cellular Decomposition (BCD)

#ifndef BOUSTROPHEDON_CELLULAR_DECOMPOSITION_H
#define BOUSTROPHEDON_CELLULAR_DECOMPOSITION_H

#include "../coverage_path_planning.h"

typedef enum {
    BOUND_IN,           // IN event for boundary
    BOUND_SIDE_IN,      // SIDE_IN event for boundary
    BOUND_INIT,         // left-most boundary polygon vertex
    
    BOUND_OUT,          // OUT event for boundary
    BOUND_SIDE_OUT,     // SIDE_OUT event for boundary
    BOUND_DEINIT,       // right-most boundary polygon vertex

    IN,                 // left-most obstacle polygon vertex OR
                        // (floor_edge_vector_angle ∈ (90, 180] AND 
                        // ceiling_edge_vector_angle ∈ (270, (floor_edge_vector_angle + 180)) OR
                        // (floor_edge_vector_angle ∈ (180, 270) AND
                        // ceiling_edge_vector_angle ∈ (270, 360] ∪ [0, (floor_edge_vector_angle - 180)))
    
    SIDE_IN,            // (floor_edge_vector_angle ∈ (90, 180] AND 
                        // ceiling_edge_vector_angle ∈ [0, 90) ∪ ((floor_edge_vector_angle + 180), 360]) OR
                        // (floor_edge_vector_angle ∈ (180, 270) AND
                        // ceiling_edge_vector_angle ∈ ((floor_edge_vector_angle - 180), 270))
    
    OUT,                // right-most obstacle polygon vertex OR
                        // (ceiling_edge_vector_angle ∈ [0, 90) AND 
                        // ceiling_edge_vector_angle ∈ (90, (90 + floor_edge_vector_angle)) OR
                        // (ceiling_edge_vector_angle ∈ (270, 360] AND
                        // ceiling_edge_vector_angle ∈ (90, (floor_edge_vector_angle - 180))
    
    SIDE_OUT,           // (ceiling_edge_vector_angle ∈ [0, 90) AND 
                        // ceiling_edge_vector_angle ∈ ((floor_edge_vector_angle + 180), 270)) OR
                        // (ceiling_edge_vector_angle ∈ (270, 360] AND
                        // ceiling_edge_vector_angle ∈ ((floor_edge_vector_angle - 180), 270)) 
    
    FLOOR,              // event between OUT and IN (traced along the polygon verticies in winding order); event has only floor_edge
    CEILING             // event between IN and OUT (traced along the polygon verticies in winding order); event has only ceiling_edge
} bcd_event_type_t;

typedef struct {
    polygon_type_t polygon_type;
    polygon_vertex_t polygon_vertex;
    bcd_event_type_t bcd_event_type;
    polygon_edge_t floor_edge;          // edge emanating from the event
    polygon_edge_t ceiling_edge;        // edge terminating at the event
} bcd_event_t;

// Error codes for BCD event list generation
#define BCD_SUCCESS                     0
#define BCD_ERROR_INVALID_INPUT        -1
#define BCD_ERROR_MEMORY_ALLOCATION    -2
#define BCD_ERROR_POLYGON_VALIDATION   -3
#define BCD_ERROR_GEOMETRIC_COMPUTATION -4
#define BCD_ERROR_EVENT_CLASSIFICATION -5
#define BCD_ERROR_CONSTRAINT_VIOLATION -6
#define BCD_ERROR_WINDING_VALIDATION   -7

int build_bcd_event_list(input_environment_t *env, bcd_event_t **event_list, int *event_count);
int find_bcd_event_type(bcd_event_t *bcd_event);
float compute_edge_angle(polygon_edge_t poly_edge);

// Test wrapper functions (only available when BCD_TESTING is defined)
#ifdef BCD_TESTING
int test_validate_input_environment(input_environment_t *env);
int test_validate_polygon(polygon_t *polygon, const char *polygon_name);
int test_extract_events_from_environment(input_environment_t *env, bcd_event_t *events, int *event_count);
int test_populate_event_geometry(bcd_event_t *event, polygon_t *polygon, int vertex_index);
int test_sort_events_by_x_coordinate(bcd_event_t *events, int event_count);
int test_validate_event_constraints(bcd_event_t *events, int event_count);
int test_find_boundary_extremes(polygon_t *boundary, int *leftmost_idx, int *rightmost_idx);
int test_classify_event_with_context(bcd_event_t *event, polygon_t *boundary, int leftmost_idx, int rightmost_idx, int vertex_idx);
#endif

#endif // BOUSTROPHEDON_CELLULAR_DECOMPOSITION_H
