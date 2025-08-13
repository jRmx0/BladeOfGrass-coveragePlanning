// Orchestrates the coverage path planning process

#ifndef COVERAGE_PATH_PLANNING_H
#define COVERAGE_PATH_PLANNING_H

#include <stdint.h>

typedef struct
{
    float x;
    float y;
} polygon_vertex_t;

typedef struct
{
    polygon_vertex_t begin;
    polygon_vertex_t end;
} polygon_edge_t;

typedef enum {
    POLYGON_WINDING_UNKNOWN = 0,
    POLYGON_WINDING_CW = 1,
    POLYGON_WINDING_CCW = 2
} polygon_winding_t;

typedef enum {
    BOUNDARY,
    OBSTACLE
} polygon_type_t;

typedef struct
{
    polygon_winding_t winding;

    polygon_vertex_t *vertices;
    uint32_t vertex_count;

    polygon_edge_t *edges;
    uint32_t edge_count;   
} polygon_t;

typedef struct
{
    uint32_t id;
    float path_width;
    float path_overlap;

    polygon_t boundary;

    polygon_t *obstacles;
    uint32_t obstacle_count;
} input_environment_t;

void coverage_path_planning_process(const char *input_environment_json);

void free_polygon(polygon_t *polygon);
void free_input_environment(input_environment_t *env);

#endif // COVERAGE_PATH_PLANNING_H
