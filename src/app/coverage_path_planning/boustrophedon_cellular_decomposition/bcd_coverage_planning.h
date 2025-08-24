#ifndef BCD_COVERAGE_H
#define BCD_COVERAGE_H

#include "../../../../dependencies/cvector/cvector.h"
#include "bcd_cell_computation.h"

typedef struct
{
    cvector_vector_type(point_t) ox;
} cell_motion_plan_t;


typedef struct
{
    cvector_vector_type(cell_motion_plan_t) section;
} bcd_motion_plan_t;

int compute_bcd_path_list(cvector_vector_type(bcd_cell_t) * cell_list,
                          int starting_cell_index,
                          cvector_vector_type(int) * path_list);

void log_bcd_path_list(const cvector_vector_type(int) * path_list);

int compute_bcd_motion(cvector_vector_type(bcd_cell_t) * cell_list,
                       const cvector_vector_type(int) * path_list,
                       bcd_motion_plan_t *motion_plan,
                       int step_size);

void log_bcd_motion(const bcd_motion_plan_t motion_plan);

#endif // BCD_COVERAGE_H
