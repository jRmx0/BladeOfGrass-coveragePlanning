#ifndef BCD_CELL_COMPUTATION_H
#define BCD_CELL_COMPUTATION_H

#include "bcd_event_list_building.h"

int compute_bcd_cells(const bcd_event_list_t *event_list,
                      cvector_vector_type(bcd_cell_t) *cell_list);

void log_bcd_cell_list(const cvector_vector_type(bcd_cell_t) *cell_list);
void free_bcd_cell_list(cvector_vector_type(bcd_cell_t) *cell_list);

#endif // BCD_CELL_COMPUTATION_H
