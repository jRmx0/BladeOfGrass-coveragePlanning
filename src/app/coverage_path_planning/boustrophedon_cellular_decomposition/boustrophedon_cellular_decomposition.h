// Generates events list, cells and adjacency graph using Boustrophedon Cellular Decomposition (BCD)

#ifndef BOUSTROPHEDON_CELLULAR_DECOMPOSITION_H
#define BOUSTROPHEDON_CELLULAR_DECOMPOSITION_H

#include "../coverage_path_planning.h"

typedef enum {
    BOUND_IN,
    BOUND_SIDE_IN,
    BOUND_INIT,
    
    BOUND_OUT,
    BOUND_SIDE_OUT,
    BOUND_DEINIT,

    IN,
    SIDE_IN,
    OUT,
    SIDE_OUT,
    FLOOR,
    CEILING
} bcd_event_type_t;

typedef struct {
    polygon_type_t polygon_type;
    polygon_vertex_t polygon_vertex;
    bcd_event_type_t bcd_event_type;
    polygon_edge_t floor_edge;
    polygon_edge_t ceiling_edge;
} bcd_event_t;

void bcd_run_demo(void);

#endif // BOUSTROPHEDON_CELLULAR_DECOMPOSITION_H
