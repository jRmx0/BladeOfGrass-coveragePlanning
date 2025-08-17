#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../../../../dependencies/cvector/cvector.h"
#include "boustrophedon_cellular_decomposition.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// FORWARD DECLARATIONS ---------------------------------------------

// --- BUILD_BCD_EVENT_LIST

static int preallocate_event_list(const input_environment_t *env,
                                  bcd_event_list_t *event_list);

static int find_polygon_events(const polygon_t polygon,
                               bcd_event_list_t *event_list);

// --- --- FIND_POLYGON_EVENTS

static int find_leftmost_event(const polygon_t polygon,
                               bcd_event_list_t *event_list);

static int find_common_event(const polygon_t polygon,
                             bcd_event_list_t *event_list,
                             int vertex_index);

// --- --- --- FIND_COMMON_EVENT

static bool in_event(polygon_edge_t floor_edge,
                     polygon_edge_t ceil_edge);

static bool side_in_event(polygon_edge_t floor_edge,
                          polygon_edge_t ceil_edge);

static bool out_event(polygon_edge_t floor_edge,
                      polygon_edge_t ceil_edge);

static bool side_out_event(polygon_edge_t floor_edge,
                           polygon_edge_t ceil_edge);

static bcd_event_type_t floor_or_ceiling_event(const bcd_event_list_t *event_list);

static float compute_vector_angle_degrees(polygon_edge_t poly_edge);

// EVENT LIST HELPERS

static int push_to_bcd_event_list(bcd_event_list_t *event_list,
                                  bcd_event_t event);

static bcd_event_t fill_bcd_event(polygon_type_t polygon_type,
                                  point_t polygon_vertex,
                                  bcd_event_type_t bcd_event_type,
                                  polygon_edge_t floor_edge,
                                  polygon_edge_t ceiling_edge);

static void log_vertex_with_angles(const polygon_t poly,
                                   point_t v, int f_e_i,
                                   float floor_angle,
                                   int c_e_i,
                                   float ceil_angle);

static void sort_event_list(bcd_event_list_t *event_list);

// --- --- SORT_EVENT_LIST

static int compare_events(const void *a,
                          const void *b);

// --- COMPUTE_BCD_CELLS

static int handle_side_in(const bcd_event_t curr_evnt,
                          cvector_vector_type(bcd_cell_t) * cell_list);

static int handle_in(const bcd_event_t curr_evnt,
                     cvector_vector_type(bcd_cell_t) * cell_list);

// --- --- HANDLE_IN HELPERS

static void in_find_prev_cell(const bcd_event_t curr_evt,
                              cvector_vector_type(bcd_cell_t) * cell_list,
                              bcd_cell_t **prev_cell,
                              point_t *c_pt,
                              point_t *f_pt);

// ---

static int handle_side_out(const bcd_event_t curr_evnt,
                           cvector_vector_type(bcd_cell_t) * cell_list);

static int handle_out(const bcd_event_t curr_evnt,
                      cvector_vector_type(bcd_cell_t) * cell_list);

static int handle_floor(const bcd_event_t curr_evnt,
                        cvector_vector_type(bcd_cell_t) * cell_list);

static int handle_ceiling(const bcd_event_t curr_evnt,
                          cvector_vector_type(bcd_cell_t) * cell_list);

// --- --- HANDLER HELPERS

static bcd_cell_t fill_bcd_cell(point_t c_begin,
                                polygon_edge_t c_edge,
                                point_t c_end,
                                point_t f_begin,
                                polygon_edge_t f_edge,
                                point_t f_end,
                                bcd_neighbor_list_t neighbor_list,
                                bool open,
                                bool visited,
                                bool cleaned);

static void update_prev_cell(bcd_cell_t *prev_cell,
                             point_t c_pt,
                             point_t f_pt,
                             bcd_event_type_t evt_type,
                             bcd_cell_t *top_cell,
                             bcd_cell_t *bottom_cell);

static float calc_evt_to_edge_dist(const point_t evt_vertex,
                                   const polygon_edge_t cell_edge,
                                   point_t *intersection);

// --- NEIGHBOR_LIST HELPERS

static void add_head_cell_neighbor_list(bcd_neighbor_list_t *neighbor_list,
                                        bcd_cell_t *new_head);

static void add_tail_cell_neighbor_list(bcd_neighbor_list_t *neighbor_list,
                                        bcd_cell_t *new_tail);

static void free_neighbor_list(bcd_neighbor_list_t *neighbor_list);

//
// IMPLEMENTATION --- build_bcd_event_list --------------------------
//

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

    sort_event_list(event_list);

    // --- BUILD_BCD_EVENT_LIST helpers (order per forward declarations)

    return 0;
}

// --- BUILD_BCD_EVENT_LIST

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

// --- --- FIND_POLYGON_EVENTS

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
    event_type = polygon_type == BOUNDARY ? SIDE_IN : IN;

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

    if (in_event(polygon.edges[terminating], polygon.edges[emanating]))
    {
        event_type = IN;
        floor_edge_index = terminating;
        ceiling_edge_index = emanating;
    }
    else if (side_in_event(polygon.edges[terminating], polygon.edges[emanating]))
    {
        event_type = SIDE_IN;
        floor_edge_index = terminating;
        ceiling_edge_index = emanating;
    }
    else if (out_event(polygon.edges[emanating], polygon.edges[terminating]))
    {
        event_type = OUT;
        floor_edge_index = emanating;
        ceiling_edge_index = terminating;
    }
    else if (side_out_event(polygon.edges[emanating], polygon.edges[terminating]))
    {
        event_type = SIDE_OUT;
        floor_edge_index = emanating;
        ceiling_edge_index = terminating;
    }
    else if ((event_type = floor_or_ceiling_event(event_list)) != NONE)
    {
        if (event_type == FLOOR)
        {
            floor_edge_index = terminating;
            ceiling_edge_index = -1;
        }
        else if (event_type == CEILING)
        {
            floor_edge_index = -1;
            ceiling_edge_index = emanating;
        }
    }
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

// --- --- --- FIND_COMMON_EVENT

static bool in_event(polygon_edge_t floor_edge,
                     polygon_edge_t ceil_edge)
{
    float floor_angle = compute_vector_angle_degrees(floor_edge);
    float ceil_angle = compute_vector_angle_degrees(ceil_edge);

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

static bool side_in_event(polygon_edge_t floor_edge,
                          polygon_edge_t ceil_edge)
{
    float floor_angle = compute_vector_angle_degrees(floor_edge);
    float ceil_angle = compute_vector_angle_degrees(ceil_edge);

    if (90.0f < floor_angle && floor_angle <= 180.0f)
    {
        float max_ceil_angle = floor_angle + 180.0f;
        return (270.0f <= ceil_angle && ceil_angle < max_ceil_angle);
    }
    if (180.0f <= floor_angle && floor_angle < 270.0f)
    {
        float max_ceil_angle = floor_angle - 180.0f;
        return ((0.0f <= ceil_angle && ceil_angle < max_ceil_angle) ||
                (270.0f < ceil_angle && ceil_angle <= 360.0f));
    }

    return false;
}

static bool out_event(polygon_edge_t floor_edge,
                      polygon_edge_t ceil_edge)
{
    float floor_angle = compute_vector_angle_degrees(floor_edge);
    float ceil_angle = compute_vector_angle_degrees(ceil_edge);

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

static bool side_out_event(polygon_edge_t floor_edge,
                           polygon_edge_t ceil_edge)
{
    float floor_angle = compute_vector_angle_degrees(floor_edge);
    float ceil_angle = compute_vector_angle_degrees(ceil_edge);

    if (0.0f <= ceil_angle && ceil_angle < 90.0f)
    {
        float max_floor_angle = ceil_angle + 180.0f;
        return (90.0f < floor_angle && floor_angle < max_floor_angle);
    }

    // Fix chained comparison: use logical AND
    if (270.0f < ceil_angle && ceil_angle <= 360.0f)
    {
        float max_floor_angle = ceil_angle - 180.0f;
        return (90.0f < floor_angle && floor_angle < max_floor_angle);
    }

    return false;
}

static bcd_event_type_t floor_or_ceiling_event(const bcd_event_list_t *event_list)
{
    for (int i = event_list->length - 1; i >= 0; i--)
    {
        bcd_event_type_t prev_event = event_list->bcd_events[i].bcd_event_type;

        if (prev_event == B_INIT || prev_event == B_IN || prev_event == B_SIDE_IN || prev_event == IN || prev_event == SIDE_IN)
            return CEILING;

        if (prev_event == B_DEINIT || prev_event == B_OUT || prev_event == B_SIDE_OUT || prev_event == OUT || prev_event == SIDE_OUT)
            return FLOOR;
    }

    return NONE;
}

static float compute_vector_angle_degrees(polygon_edge_t poly_edge)
{
    point_t begin = poly_edge.begin;
    point_t end = poly_edge.end;

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

// EVENT LIST HELPERS

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

static bcd_event_t fill_bcd_event(polygon_type_t polygon_type,
                                  point_t polygon_vertex,
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

static void sort_event_list(bcd_event_list_t *event_list)
{
    if (!event_list || event_list->length <= 1)
        return;

    qsort(event_list->bcd_events, event_list->length, sizeof(bcd_event_t), compare_events);
}

// --- --- SORT_EVENT_LIST

static int compare_events(const void *a,
                          const void *b)
{
    const bcd_event_t *event_a = (const bcd_event_t *)a;
    const bcd_event_t *event_b = (const bcd_event_t *)b;

    if (event_a->polygon_vertex.x < event_b->polygon_vertex.x)
        return -1;
    else if (event_a->polygon_vertex.x > event_b->polygon_vertex.x)
        return 1;
    else
        return 0;
}

// log_vertex_with_angles(poly, poly.vertices[vertex_index], floor_edge_index, floor_angle, ceiling_edge_index, ceil_angle);
static void log_vertex_with_angles(const polygon_t poly,
                                   point_t v,
                                   int f_e_i,
                                   float floor_angle,
                                   int c_e_i,
                                   float ceil_angle)
{
    printf("BCD debug: vertex=(%.3f, %.3f)\n          f_e_b=(%.3f, %.3f), f_e_e=(%.3f, %.3f), f_a=%.2f deg,\n          c_e_b=(%.3f, %.3f), c_e_e=(%.3f, %.3f), c_a=%.2f deg\n",
           v.x, v.y,
           poly.edges[f_e_i].begin.x, poly.edges[f_e_i].begin.y, poly.edges[f_e_i].end.x, poly.edges[f_e_i].end.y, floor_angle,
           poly.edges[c_e_i].begin.x, poly.edges[c_e_i].begin.y, poly.edges[c_e_i].end.x, poly.edges[c_e_i].end.y, ceil_angle);
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

// IMPLEMENTATION --- compute_bcd_cells -----------------------------

int compute_bcd_cells(const bcd_event_list_t *event_list,
                      cvector_vector_type(bcd_cell_t) * cell_list)
{
    int rc = 0;

    for (int i = 0; i < event_list->length; i++)
    {
        bcd_event_t curr_evnt = event_list->bcd_events[i];
        bcd_event_type_t curr_evnt_type = curr_evnt.bcd_event_type;

        switch (curr_evnt_type)
        {
        case SIDE_IN:
            rc = handle_side_in(curr_evnt, cell_list);
            break;

        case IN:
            rc = handle_in(curr_evnt, cell_list);
            break;

        case SIDE_OUT:
            rc = handle_side_out(curr_evnt, cell_list);
            break;

        case OUT:
            rc = handle_out(curr_evnt, cell_list);
            break;

        case FLOOR:
            rc = handle_floor(curr_evnt, cell_list);
            break;

        case CEILING:
            rc = handle_ceiling(curr_evnt, cell_list);
            break;

        case NONE:
            return -2; // Unhandled event

        default:
            return -1; // Invalid event
        }

        if (rc != 0)
        {
            fprintf(stderr, "Error handling event %d: %d\n", curr_evnt_type, rc);
            return rc;
        }
    }

    return 0;
}

// --- COMPUTE_BCD_CELLS

static int handle_side_in(const bcd_event_t curr_evnt,
                          cvector_vector_type(bcd_cell_t) * cell_list)
{
    bcd_cell_t new_cell = fill_bcd_cell(curr_evnt.polygon_vertex,
                                        curr_evnt.ceiling_edge,
                                        (point_t){0},
                                        curr_evnt.polygon_vertex,
                                        curr_evnt.floor_edge,
                                        (point_t){0},
                                        (bcd_neighbor_list_t){0},
                                        true,
                                        false,
                                        false);

    cvector_push_back(*cell_list, new_cell);

    return 0;
}

static int handle_in(const bcd_event_t curr_evnt,
                     cvector_vector_type(bcd_cell_t) * cell_list)
{
    bcd_cell_t *prev_cell = NULL;
    point_t c_point;
    point_t f_point;

    in_find_prev_cell(curr_evnt, cell_list, &prev_cell, &c_point, &f_point);
    if (prev_cell == NULL) {
        printf("Error: No previous cell found for IN event\n");
        return -3;
    }
    
    // Store the index of the previous cell before vector operations
    size_t prev_cell_index = prev_cell - &(*cell_list)[0];
    
    bcd_cell_t top_cell = fill_bcd_cell(c_point,
                             *cvector_back(prev_cell->ceiling_edge_list),
                             (point_t){0},
                             curr_evnt.polygon_vertex,
                             curr_evnt.floor_edge,
                             (point_t){0},
                             (bcd_neighbor_list_t){0},
                             true,
                             false,
                             false);
    
    cvector_push_back(*cell_list, top_cell);
    size_t top_cell_index = cvector_size(*cell_list) - 1;
    
    bcd_cell_t bottom_cell = fill_bcd_cell(curr_evnt.polygon_vertex,
                                           curr_evnt.ceiling_edge,
                                           (point_t){0},
                                           f_point,
                                           *cvector_back(prev_cell->floor_edge_list),
                                           (point_t){0},
                                           (bcd_neighbor_list_t){0},
                                           true,
                                           false,
                                           false);

    cvector_push_back(*cell_list, bottom_cell);
    size_t bottom_cell_index = cvector_size(*cell_list) - 1;

    prev_cell = &(*cell_list)[prev_cell_index];
    bcd_cell_t *top_cell_ptr = &(*cell_list)[top_cell_index];
    bcd_cell_t *bottom_cell_ptr = &(*cell_list)[bottom_cell_index];

    update_prev_cell(prev_cell, c_point, f_point, IN, top_cell_ptr, bottom_cell_ptr);

    return 0;
}

// --- --- HANDLE_IN

static void in_find_prev_cell(const bcd_event_t curr_evt,
                              cvector_vector_type(bcd_cell_t) * cell_list,
                              bcd_cell_t **prev_cell,
                              point_t *c_pt,
                              point_t *f_pt)
{
    if (!cell_list || !*cell_list)
    {
        printf("BCD Cell List: NULL or empty\n");
        return;
    }

    float min_evt_to_ceil_dist = INFINITY;
    float min_evt_to_floor_dist = INFINITY;
    bcd_cell_t *closest_cell = NULL;
    point_t closest_c_pt;
    point_t closest_f_pt;

    size_t i;
    for (i = 0; i < cvector_size(*cell_list); ++i)
    {
        bcd_cell_t *cell = &(*cell_list)[i];

        if (!cell->open)
            continue;

        polygon_edge_t ceil_edge = cell->ceiling_edge_list[(cvector_size(cell->ceiling_edge_list)) - 1];
        point_t c_intersection;
        float evt_to_ceil_dist = calc_evt_to_edge_dist(curr_evt.polygon_vertex, ceil_edge, &c_intersection);

        polygon_edge_t floor_edge = cell->floor_edge_list[(cvector_size(cell->floor_edge_list)) - 1];
        point_t f_intersection;
        float evt_to_floor_dist = calc_evt_to_edge_dist(curr_evt.polygon_vertex, floor_edge, &f_intersection);

        if ((evt_to_ceil_dist < min_evt_to_ceil_dist &&
             evt_to_floor_dist < min_evt_to_floor_dist))
        {
            min_evt_to_ceil_dist = evt_to_ceil_dist;
            min_evt_to_floor_dist = evt_to_floor_dist;

            closest_cell = cell;
            closest_c_pt = c_intersection;
            closest_f_pt = f_intersection;
        }
    }

    *prev_cell = closest_cell;
    *c_pt = closest_c_pt;
    *f_pt = closest_f_pt;
}

// ---

static int handle_side_out(const bcd_event_t curr_evnt,
                           cvector_vector_type(bcd_cell_t) * cell_list)
{
    return 0;
}

static int handle_out(const bcd_event_t curr_evnt,
                      cvector_vector_type(bcd_cell_t) * cell_list)
{
    return 0;
}

static int handle_floor(const bcd_event_t curr_evnt,
                        cvector_vector_type(bcd_cell_t) * cell_list)
{
    return 0;
}

static int handle_ceiling(const bcd_event_t curr_evnt,
                          cvector_vector_type(bcd_cell_t) * cell_list)
{
    return 0;
}

// --- --- HANDLER HELPERS

static bcd_cell_t fill_bcd_cell(point_t c_begin,
                                polygon_edge_t c_edge,
                                point_t c_end,
                                point_t f_begin,
                                polygon_edge_t f_edge,
                                point_t f_end,
                                bcd_neighbor_list_t neighbor_list,
                                bool open,
                                bool visited,
                                bool cleaned)
{
    bcd_cell_t cell;

    cell.c_begin = c_begin;

    cvector_vector_type(polygon_edge_t) c_edge_list = NULL;
    cvector_push_back(c_edge_list, c_edge);
    cell.ceiling_edge_list = c_edge_list;

    cell.c_end = c_end;

    cell.f_begin = f_begin;

    cvector_vector_type(polygon_edge_t) f_edge_list = NULL;
    cvector_push_back(f_edge_list, f_edge);
    cell.floor_edge_list = f_edge_list;

    cell.f_end = f_end;

    cell.neighbor_list = neighbor_list;

    cell.open = open;
    cell.visited = visited;
    cell.cleaned = cleaned;

    return cell;
}

static void update_prev_cell(bcd_cell_t *prev_cell,
                             point_t c_pt,
                             point_t f_pt,
                             bcd_event_type_t evt_type,
                             bcd_cell_t *top_cell,
                             bcd_cell_t *bottom_cell)
{
    prev_cell->c_end = c_pt;
    prev_cell->f_end = f_pt;

    bcd_neighbor_list_t *neighbor_list = &prev_cell->neighbor_list;

    if (evt_type == IN)
    {
        add_head_cell_neighbor_list(neighbor_list, top_cell);
        add_head_cell_neighbor_list(neighbor_list, bottom_cell);
    }

    if (evt_type == OUT)
    {
        add_tail_cell_neighbor_list(neighbor_list, top_cell);
        add_tail_cell_neighbor_list(neighbor_list, bottom_cell);
    }

    prev_cell->open = false;
}

static float calc_evt_to_edge_dist(point_t evt_vertex, polygon_edge_t cell_edge, point_t *intersection)
{
    float slice = evt_vertex.x;

    point_t edge_start = cell_edge.begin;
    point_t edge_end = cell_edge.end;

    float min_edge_x = edge_start.x < edge_end.x ? edge_start.x : edge_end.x;
    float max_edge_x = edge_start.x > edge_end.x ? edge_start.x : edge_end.x;

    if (slice < min_edge_x || slice > max_edge_x)
    {
        return INFINITY;
    }

    // Calculate intersection point on the edge at x = slice
    float dx = edge_end.x - edge_start.x;

    if (fabsf(dx) < 1e-9f)
    {
        // Vertical edge case
        intersection->x = edge_start.x;
        intersection->y = edge_start.y;
    }
    else
    {
        // Linear interpolation to find y at x = slice
        float t = (slice - edge_start.x) / dx;
        intersection->x = slice;
        intersection->y = edge_start.y + t * (edge_end.y - edge_start.y);
    }

    float dist_dy = intersection->y - evt_vertex.y;
    return fabsf(dist_dy);
}

// --- NEIGHBOR_LIST HELPERS

static void add_head_cell_neighbor_list(bcd_neighbor_list_t *neighbor_list,
                                        bcd_cell_t *new_head)
{
    if (!neighbor_list || !new_head)
        return;

    bcd_neighbor_node_t *new_node = (bcd_neighbor_node_t *)malloc(sizeof(bcd_neighbor_node_t));
    if (!new_node)
        return;

    new_node->cell = new_head;
    new_node->prev = NULL;
    new_node->next = neighbor_list->head;

    if (neighbor_list->head)
    {
        neighbor_list->head->prev = new_node;
    }

    neighbor_list->head = new_node;

    if (!neighbor_list->tail)
    {
        neighbor_list->tail = new_node;
    }

    neighbor_list->count++;
}

static void add_tail_cell_neighbor_list(bcd_neighbor_list_t *neighbor_list,
                                        bcd_cell_t *new_tail)
{
    if (!neighbor_list || !new_tail)
        return;

    bcd_neighbor_node_t *new_node = (bcd_neighbor_node_t *)malloc(sizeof(bcd_neighbor_node_t));
    if (!new_node)
        return;

    new_node->cell = new_tail;
    new_node->next = NULL;
    new_node->prev = neighbor_list->tail;

    if (neighbor_list->tail)
    {
        neighbor_list->tail->next = new_node;
    }

    neighbor_list->tail = new_node;

    if (!neighbor_list->head)
    {
        neighbor_list->head = new_node;
    }

    neighbor_list->count++;
}

static void free_neighbor_list(bcd_neighbor_list_t *neighbor_list)
{
    if (!neighbor_list)
        return;

    bcd_neighbor_node_t *current = neighbor_list->head;
    while (current)
    {
        bcd_neighbor_node_t *next = current->next;
        free(current);
        current = next;
    }

    neighbor_list->head = NULL;
    neighbor_list->tail = NULL;
    neighbor_list->count = 0;
}

// CELL LIST HELPERS

// log_bcd_cell_list((const cvector_vector_type(bcd_cell_t) *) &cell_list);
void log_bcd_cell_list(const cvector_vector_type(bcd_cell_t) * cell_list)
{
    if (!cell_list || !*cell_list)
    {
        printf("BCD Cell List: NULL or empty\n");
        return;
    }

    size_t count = cvector_size(*cell_list);
    printf("BCD Cell List: %zu cells\n", count);

    for (size_t i = 0; i < count; ++i)
    {
        const bcd_cell_t *cell = &(*cell_list)[i];
        printf("  Cell %zu:\n", i);
        printf("    c_begin: (%.2f, %.2f)\n", cell->c_begin.x, cell->c_begin.y);
        printf("    c_end: (%.2f, %.2f)\n", cell->c_end.x, cell->c_end.y);
        printf("    f_begin: (%.2f, %.2f)\n", cell->f_begin.x, cell->f_begin.y);
        printf("    f_end: (%.2f, %.2f)\n", cell->f_end.x, cell->f_end.y);
        printf("    visited: %s\n", cell->visited ? "true" : "false");

        // Log ceiling edge list
        if (cell->ceiling_edge_list)
        {
            size_t ceiling_count = cvector_size(cell->ceiling_edge_list);
            printf("    ceiling_edges: %zu edges\n", ceiling_count);
            for (size_t j = 0; j < ceiling_count; ++j)
            {
                printf("      [%zu]: (%.2f,%.2f) -> (%.2f,%.2f)\n", j,
                       cell->ceiling_edge_list[j].begin.x, cell->ceiling_edge_list[j].begin.y,
                       cell->ceiling_edge_list[j].end.x, cell->ceiling_edge_list[j].end.y);
            }
        }
        else
        {
            printf("    ceiling_edges: NULL\n");
        }

        // Log floor edge list
        if (cell->floor_edge_list)
        {
            size_t floor_count = cvector_size(cell->floor_edge_list);
            printf("    floor_edges: %zu edges\n", floor_count);
            for (size_t j = 0; j < floor_count; ++j)
            {
                printf("      [%zu]: (%.2f,%.2f) -> (%.2f,%.2f)\n", j,
                       cell->floor_edge_list[j].begin.x, cell->floor_edge_list[j].begin.y,
                       cell->floor_edge_list[j].end.x, cell->floor_edge_list[j].end.y);
            }
        }
        else
        {
            printf("    floor_edges: NULL\n");
        }

        printf("    neighbor_list: head=%p, tail=%p, count=%d\n",
               (void *)cell->neighbor_list.head,
               (void *)cell->neighbor_list.tail,
               cell->neighbor_list.count);

        // Log all neighbor nodes in the list
        if (cell->neighbor_list.head)
        {
            printf("    neighbor nodes:\n");
            bcd_neighbor_node_t *current = cell->neighbor_list.head;
            int node_index = 0;
            while (current)
            {
                printf("      [%d]: node=%p, cell=%p", node_index, (void *)current, (void *)current->cell);
                if (current->cell)
                {
                    printf(" cell_pos=(%.2f,%.2f)", current->cell->c_begin.x, current->cell->c_begin.y);
                }
                printf(" prev=%p, next=%p\n", (void *)current->prev, (void *)current->next);
                current = current->next;
                node_index++;

                // Safety check to prevent infinite loops
                if (node_index > cell->neighbor_list.count + 1)
                {
                    printf("      [WARNING]: Potential infinite loop detected in neighbor list!\n");
                    break;
                }
            }
        }
        else
        {
            printf("    neighbor nodes: (empty list)\n");
        }
        printf("\n");
    }
}

void free_bcd_cell_list(cvector_vector_type(bcd_cell_t) * cell_list)
{
    if (*cell_list)
    {
        size_t i;
        for (i = 0; i < cvector_size(*cell_list); ++i)
        {
            cvector_free((*cell_list)[i].ceiling_edge_list);
            cvector_free((*cell_list)[i].floor_edge_list);
            free_neighbor_list(&(*cell_list)[i].neighbor_list);
        }
    }
    cvector_free(*cell_list);
}
