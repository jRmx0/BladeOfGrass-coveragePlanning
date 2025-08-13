#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "boustrophedon_cellular_decomposition.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static float compute_vector_angle_degrees(polygon_edge_t poly_edge);

int build_bcd_event_list(input_environment_t *env, bcd_event_list_t *event_list)
{
    return 0;
}

static float compute_vector_angle_degrees(polygon_edge_t poly_edge)
{
    polygon_vertex_t begin = poly_edge.begin;
    polygon_vertex_t end = poly_edge.end;

    float dx = end.x - begin.x;
    float dy = end.y - begin.y;

    float angle_radians = atan2(dy, dx);

    // Convert to degrees and normalize to [0, 360)
    float angle_degrees = angle_radians * 180.0f / M_PI;

    while (angle_degrees < 0.0f)
    {
        angle_degrees += 360.0f;
    }
    while (angle_degrees >= 360.0f)
    {
        angle_degrees -= 360.0f;
    }
    return angle_degrees;
}

void free_bcd_event_list(bcd_event_list_t *event_list)
{
    if (!event_list)
        return;

    if (event_list->bcd_events)
    {
        free(event_list->bcd_events);
    }
    event_list->bcd_events = NULL;
    event_list->length = 0;
}