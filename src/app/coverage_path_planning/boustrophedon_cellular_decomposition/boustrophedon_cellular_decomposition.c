#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
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
                             int vertex_index);

static bool in_event(const polygon_t polygon, int vertex_index, int floor_edge_index, int ceiling_edge_index);
static bool side_in_event(const polygon_t polygon, int vertex_index, int floor_edge_index, int ceiling_edge_index);
static bool out_event(const polygon_t polygon, int vertex_index, int floor_edge_index, int ceiling_edge_index);
static bool side_out_event(const polygon_t polygon, int vertex_index, int floor_edge_index, int ceiling_edge_index);
static bool floor_event(const polygon_t polygon, int vertex_index, int floor_edge_index);
static bool ceiling_event(const polygon_t polygon, int vertex_index, int ceiling_edge_index);

static int push_to_bcd_event_list(bcd_event_list_t *event_list,
                                  bcd_event_t event);

static bcd_event_t fill_bcd_event(polygon_type_t polygon_type,
                                  polygon_vertex_t polygon_vertex,
                                  bcd_event_type_t bcd_event_type,
                                  polygon_edge_t floor_edge,
                                  polygon_edge_t ceiling_edge);

static float compute_vector_angle_degrees(polygon_edge_t poly_edge);
static void log_vertex_with_angles(const polygon_t poly, polygon_vertex_t v, int f_e_i, float floor_angle, int c_e_i, float ceil_angle);

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

    bcd_event_t leftmost_event;

    polygon_type_t polygon_type;
    polygon_type = polygon.winding == POLYGON_WINDING_CW ? BOUNDARY : OBSTACLE;
    bcd_event_type_t event_type;
    event_type = polygon_type == BOUNDARY ? BOUND_INIT : IN;

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
                             int vertex_index)
{
    if (polygon.vertex_count == 0 || polygon.vertices == NULL)
    {
        return -1;
    }

    bcd_event_type_t event_type;
    int floor_edge_index = -1;
    int ceiling_edge_index = -1;

    int emanating = vertex_index;
    int terminating = (vertex_index + polygon.vertex_count - 1) % polygon.vertex_count;

    if (in_event(polygon, vertex_index, terminating, emanating))
    {
        event_type = IN;
        floor_edge_index = terminating;
        ceiling_edge_index = emanating;
    }
    else if (side_in_event(polygon, vertex_index, terminating, emanating))
    {
        event_type = SIDE_IN;
        floor_edge_index = terminating;
        ceiling_edge_index = emanating;
    }
    else if (out_event(polygon, vertex_index, emanating, terminating))
    {
        event_type = OUT;
        floor_edge_index = emanating;
        ceiling_edge_index = terminating;
    }
    // else if (side_out_event(polygon, vertex_index, emanating, terminating))
    // {
    //     event_type = SIDE_OUT;
    //     floor_edge_index = emanating;
    //     ceiling_edge_index = terminating;
    // }
    // else if (floor_event(polygon, vertex_index, terminating))
    // {
    //     event_type = FLOOR;
    //     floor_edge_index = terminating;
    // }
    // else if (ceiling_event(polygon, vertex_index, emanating))
    // {
    //     event_type = CEILING;
    //     ceiling_edge_index = emanating;
    // }
    else
    {
        // No matching event type for this vertex; do not push any event
        return 0;
    }

    bcd_event_t common_event;

    polygon_type_t polygon_type;
    polygon_type = polygon.winding == POLYGON_WINDING_CW ? BOUNDARY : OBSTACLE;

    common_event = fill_bcd_event(polygon_type,
                                  polygon.vertices[vertex_index],
                                  event_type,
                                  floor_edge_index > -1 ? polygon.edges[floor_edge_index] : (polygon_edge_t){0},
                                  ceiling_edge_index > -1 ? polygon.edges[ceiling_edge_index] : (polygon_edge_t){0});

    if (push_to_bcd_event_list(event_list, common_event) != 0)
    {
        return -2;
    }

    return 0;
}

static bool in_event(const polygon_t poly, int vertex_index, int floor_edge_index, int ceiling_edge_index)
{
    float floor_angle = compute_vector_angle_degrees(poly.edges[floor_edge_index]);
    float ceil_angle = compute_vector_angle_degrees(poly.edges[ceiling_edge_index]);

    if (90.0f < floor_angle && floor_angle <= 180.0f)
    {
        float min_ceil_angle = floor_angle + 180.0f;
        return ((0.0f <= ceil_angle && ceil_angle < 90.0f) ||
                (min_ceil_angle < ceil_angle && ceil_angle <= 360.0f));
    }
    else if (180.0f < floor_angle && floor_angle < 270.0f)
    {
        float min_ceil_angle = floor_angle - 180.0f;
        return (min_ceil_angle < ceil_angle && ceil_angle < 90.0f);
    }

    return false;
}

static bool side_in_event(const polygon_t poly, int vertex_index, int floor_edge_index, int ceiling_edge_index)
{
    float floor_angle = compute_vector_angle_degrees(poly.edges[floor_edge_index]);
    float ceil_angle = compute_vector_angle_degrees(poly.edges[ceiling_edge_index]);

    if (180.0f <= floor_angle && floor_angle < 270.0f)
    {
        float max_ceil_angle = floor_angle - 180.0f;
        return ((0.0f <= ceil_angle && ceil_angle < max_ceil_angle) ||
                (270.0f < ceil_angle && ceil_angle <= 360.0f));
    }

    return false;
}

static bool out_event(const polygon_t poly, int vertex_index, int floor_edge_index, int ceiling_edge_index)
{
    float floor_angle = compute_vector_angle_degrees(poly.edges[floor_edge_index]);
    float ceil_angle = compute_vector_angle_degrees(poly.edges[ceiling_edge_index]);

    if (0.0f <= ceil_angle && ceil_angle < 90.0f)
    {
        float min_floor_angle = ceil_angle + 180.0f;
        return (min_floor_angle < floor_angle && floor_angle < 270.0f);
    }

    if (270.0f < ceil_angle && ceil_angle <= 360.0f)
    {
        float min_floor_angle = ceil_angle - 180.0f;
        return (min_floor_angle < floor_angle && floor_angle < 270.0f);
    }

    return false;
}

static bool side_out_event(const polygon_t poly, int vertex_index, int floor_edge_index, int ceiling_edge_index)
{
    float floor_angle = compute_vector_angle_degrees(poly.edges[floor_edge_index]);
    float ceil_angle = compute_vector_angle_degrees(poly.edges[ceiling_edge_index]);
}

static bool floor_event(const polygon_t poly, int vertex_index, int floor_edge_index)
{
}

static bool ceiling_event(const polygon_t poly, int vertex_index, int ceiling_edge_index)
{
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

// log_vertex_with_angles(poly, poly.vertices[vertex_index], floor_edge_index, floor_angle, ceiling_edge_index, ceil_angle);
static void log_vertex_with_angles(const polygon_t poly, polygon_vertex_t v, int f_e_i, float floor_angle, int c_e_i, float ceil_angle)
{
    printf("BCD debug: vertex=(%.3f, %.3f)\n          f_e_b=(%.3f, %.3f), f_e_e=(%.3f, %.3f), f_a=%.2f deg,\n          c_e_b=(%.3f, %.3f), c_e_e=(%.3f, %.3f), c_a=%.2f deg\n",
           v.x, v.y,
           poly.edges[f_e_i].begin.x, poly.edges[f_e_i].begin.y, poly.edges[f_e_i].end.x, poly.edges[f_e_i].end.y, floor_angle,
           poly.edges[c_e_i].begin.x, poly.edges[c_e_i].begin.y, poly.edges[c_e_i].end.y, poly.edges[c_e_i].end.y, ceil_angle);
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
