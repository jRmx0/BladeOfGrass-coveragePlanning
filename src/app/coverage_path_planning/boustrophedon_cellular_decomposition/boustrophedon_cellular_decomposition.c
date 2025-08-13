#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "boustrophedon_cellular_decomposition.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static int preallocate_event_list(const input_environment_t *env,
                                  bcd_event_list_t *event_list);

static int find_polygon_events(const polygon_t polygon,
                                bcd_event_list_t *event_list);

static int find_leftmost_event(const polygon_t polygon,
                               bcd_event_list_t *event_list);

static int find_common_event(const polygon_t polygon,
                             bcd_event_list_t *event_list,
                             int common_index);

static int push_to_bcd_event_list(bcd_event_list_t *event_list,
                                  bcd_event_t event);

static bcd_event_t fill_bcd_event(polygon_type_t polygon_type,
                                  polygon_vertex_t polygon_vertex,
                                  bcd_event_type_t bcd_event_type,
                                  polygon_edge_t floor_edge,
                                  polygon_edge_t ceiling_edge);

static float compute_vector_angle_degrees(polygon_edge_t poly_edge);

int build_bcd_event_list(const input_environment_t *env,
                         bcd_event_list_t *event_list)
{
    int rc = 0;

    if (preallocate_event_list(env, event_list) != 0)
    {
        return -4;
    }

    rc = find_polygon_events(env->boundary, event_list);
    if (rc != 0)
    {
        return rc;
    }

    for (int i = 0; i < env->obstacle_count; i++)
    {
        rc = find_polygon_events(env->obstacles[i], event_list);
        if (rc != 0)
        {
            return rc;
        }
    }

    return 0;
}

static int preallocate_event_list(const input_environment_t *env,
                                  bcd_event_list_t *event_list)
{
    int total = (int)env->boundary.vertex_count;
    for (uint32_t i = 0; i < env->obstacle_count; ++i)
    {
        total += (int)env->obstacles[i].vertex_count;
    }
    if (total < 0)
        total = 0;
    event_list->length = 0;
    event_list->capacity = total;
    if (total > 0)
    {
        event_list->bcd_events = (bcd_event_t *)malloc((size_t)total * sizeof(bcd_event_t));
        if (!event_list->bcd_events)
        {
            event_list->capacity = 0;
            return -4;
        }
    }
    else
    {
        event_list->bcd_events = NULL;
    }
    return 0;
}

static int find_polygon_events(const polygon_t polygon,
                               bcd_event_list_t *event_list)
{
    int leftmost_index = find_leftmost_event(polygon, event_list);
    if (leftmost_index < 0)
    {
        free_bcd_event_list(event_list);
        return leftmost_index;
    }

    for (int i = (leftmost_index + 1) % (int)polygon.vertex_count;
         i != leftmost_index;
         i = (i + 1) % (int)polygon.vertex_count)
    {
        int rc = find_common_event(polygon, event_list, i);
        if (rc != 0)
        {
            free_bcd_event_list(event_list);
            return rc;
        }
    }

    return 0;
}

static int find_leftmost_event(const polygon_t polygon,
                               bcd_event_list_t *event_list)
{
    if (polygon.vertex_count == 0 || polygon.vertices == NULL)
    {
        return -1;
    }

    int leftmost_index = 0;
    float min_x = polygon.vertices[0].x;

    for (int i = 1; i < (int)polygon.vertex_count; i++)
    {
        float x = polygon.vertices[i].x;
        if (x < min_x)
        {
            min_x = x;
            leftmost_index = i;
        }
    }

    polygon_type_t polygon_type;
    polygon_type = polygon.winding == POLYGON_WINDING_CW ? BOUNDARY : OBSTACLE;
    bcd_event_type_t event_type;
    event_type = polygon_type == BOUNDARY ? BOUND_IN : IN;

    bcd_event_t leftmost_event;

    // Per contract: floor_edge = edge terminating from vertex i (edges[i-1])
    //               ceiling_edge = edge emanating at vertex i (edges[i])
    leftmost_event = fill_bcd_event(polygon_type,
                                    polygon.vertices[leftmost_index],
                                    event_type,
                                    polygon.edges[(leftmost_index + polygon.vertex_count - 1) % polygon.vertex_count],
                                    polygon.edges[leftmost_index]);

    if (push_to_bcd_event_list(event_list, leftmost_event) != 0)
    {
        return -2;
    }

    return leftmost_index;
}

static int find_common_event(const polygon_t polygon,
                             bcd_event_list_t *event_list,
                             int common_index)
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

static int push_to_bcd_event_list(bcd_event_list_t *event_list,
                                  bcd_event_t event)
{
    if (!event_list)
        return -1;
    if (event_list->length >= event_list->capacity)
        return -2;
    event_list->bcd_events[event_list->length] = event;
    event_list->length += 1;
    return 0;
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
    event_list->capacity = 0;
}

static bcd_event_t fill_bcd_event(polygon_type_t polygon_type,
                                  polygon_vertex_t polygon_vertex,
                                  bcd_event_type_t bcd_event_type,
                                  polygon_edge_t floor_edge,
                                  polygon_edge_t ceiling_edge)
{
    bcd_event_t bcd_event;
    bcd_event.polygon_type = polygon_type;
    bcd_event.polygon_vertex = polygon_vertex;
    bcd_event.bcd_event_type = bcd_event_type;
    bcd_event.floor_edge = floor_edge;
    bcd_event.ceiling_edge = ceiling_edge;
    return bcd_event;
}
