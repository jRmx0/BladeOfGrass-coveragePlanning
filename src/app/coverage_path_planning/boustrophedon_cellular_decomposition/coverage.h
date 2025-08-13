// Coverage path generation from decomposition cells and adjacency graph

#ifndef BCD_COVERAGE_H
#define BCD_COVERAGE_H

// Include the BCD header for event structure definition
#include "boustrophedon_cellular_decomposition.h"

// Placeholder types and functions for future coverage routines
typedef struct {
	int dummy;
} bcd_cell_t;

// Connection point function to pass event list to downstream BCD algorithms
int bcd_process_event_list(bcd_event_list_t event_list);

void bcd_compute(void);

#endif // BCD_COVERAGE_H
