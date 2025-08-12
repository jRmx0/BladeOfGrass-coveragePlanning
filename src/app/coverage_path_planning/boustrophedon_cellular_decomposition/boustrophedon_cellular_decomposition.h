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

    IN,                 // (floor_edge_vector_angle ∈ (90, 180] AND 
                        // ceiling_edge_vector_angle ∈ (270, (floor_edge_vector_angle + 180)) OR
                        // (floor_edge_vector_angle ∈ (180, 270) AND
                        // ceiling_edge_vector_angle ∈ (270, 360] ∪ [0, (floor_edge_vector_angle - 180)))
    
    SIDE_IN,            // (floor_edge_vector_angle ∈ (90, 180] AND 
                        // ceiling_edge_vector_angle ∈ [0, 90) ∪ ((floor_edge_vector_angle + 180), 360]) OR
                        // (floor_edge_vector_angle ∈ (180, 270) AND
                        // ceiling_edge_vector_angle ∈ ((floor_edge_vector_angle - 180), 270))
    
    OUT,                // (floor_edge_vector_angle ∈ [0, 90) AND 
                        // ceiling_edge_vector_angle ∈ (90, (90 + floor_edge_vector_angle)) OR
                        // (floor_edge_vector_angle ∈ (270, 360] AND
                        // ceiling_edge_vector_angle ∈ (90, (floor_edge_vector_angle - 180))
    
    SIDE_OUT,           // (floor_edge_vector_angle ∈ [0, 90) AND 
                        // ceiling_edge_vector_angle ∈ ((floor_edge_vector_angle + 180), 270)) OR
                        // (floor_edge_vector_angle ∈ (270, 360] AND
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

void bcd_run_demo(void);

int build_bcd_event_list(bcd_event_t *bcd_event_list);
int find_bcd_event_type(bcd_event_t bcd_event);
float compute_edge_angle(polygon_edge_t poly_edge);

#endif // BOUSTROPHEDON_CELLULAR_DECOMPOSITION_H
