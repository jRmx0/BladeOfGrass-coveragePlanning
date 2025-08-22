#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../../../../dependencies/cvector/cvector.h"
#include "../coverage_path_planning.h"
#include "bcd_event_list_building.h"
#include "bcd_cell_computation.h"

// FORWARD DECLARATIONS ---------------------------------------------

// --- COMPUTE_BCD_CELLS

static int handle_side_in(const bcd_event_t curr_evt,
                          cvector_vector_type(bcd_cell_t) * cell_list);

static int handle_in(const bcd_event_t curr_evt,
                     cvector_vector_type(bcd_cell_t) * cell_list);

// --- --- HANDLE_IN HELPERS

static void in_find_prev_cell(const bcd_event_t curr_evt,
                              cvector_vector_type(bcd_cell_t) * cell_list,
                              bcd_cell_t **prev_cell,
                              point_t *c_pt,
                              point_t *f_pt);

// ---

static int handle_side_out(const bcd_event_t curr_evt,
                           cvector_vector_type(bcd_cell_t) * cell_list);

static int handle_out(const bcd_event_t curr_evt,
                      cvector_vector_type(bcd_cell_t) * cell_list);

// --- --- HANDLE_OUT

static void out_find_top_cell(const bcd_event_t curr_evt,
                              cvector_vector_type(bcd_cell_t) * cell_list,
                              bcd_cell_t **top_cell,
                              point_t *c_pt);

static void out_find_bottom_cell(const bcd_event_t curr_evt,
                                 cvector_vector_type(bcd_cell_t) * cell_list,
                                 bcd_cell_t **bottom_cell,
                                 point_t *f_pt);

// ---

static int handle_floor(const bcd_event_t curr_evt,
                        cvector_vector_type(bcd_cell_t) * cell_list);

static int handle_ceiling(const bcd_event_t curr_evt,
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

static void update_bcd_cell(bcd_cell_t *cell,
                            point_t c_pt,
                            point_t f_pt,
                            bcd_event_type_t evt_type,
                            int top_cell_index,
                            int bottom_cell_index);

static float calc_evt_to_edge_dist(const point_t evt_vertex,
                                   const polygon_edge_t cell_edge,
                                   point_t *intersection);

// --- NEIGHBOR_LIST HELPERS

static void add_head_cell_neighbor_list(bcd_neighbor_list_t *neighbor_list,
                                        int cell_index);

static void add_tail_cell_neighbor_list(bcd_neighbor_list_t *neighbor_list,
                                        int cell_index);

static void free_neighbor_list(bcd_neighbor_list_t *neighbor_list);

// IMPLEMENTATION --- compute_bcd_cells -----------------------------

int compute_bcd_cells(const bcd_event_list_t *event_list,
                      cvector_vector_type(bcd_cell_t) * cell_list)
{
    int rc = 0;

    for (int i = 0; i < event_list->length; i++)
    {
        bcd_event_t curr_evt = event_list->bcd_events[i];
        bcd_event_type_t curr_evt_type = curr_evt.bcd_event_type;

        switch (curr_evt_type)
        {
        case SIDE_IN:
            rc = handle_side_in(curr_evt, cell_list);
            break;

        case IN:
            rc = handle_in(curr_evt, cell_list);
            break;

        case SIDE_OUT:
            rc = handle_side_out(curr_evt, cell_list);
            break;

        case OUT:
            rc = handle_out(curr_evt, cell_list);
            break;

        case FLOOR:
            rc = handle_floor(curr_evt, cell_list);
            break;

        case CEILING:
            rc = handle_ceiling(curr_evt, cell_list);
            break;

        case NONE:
            return -2; // Unhandled event

        default:
            return -1; // Invalid event
        }

        if (rc != 0)
        {
            fprintf(stderr, "Error handling event %d (type %d): %d\n", i, curr_evt_type, rc);
            return rc;
        }
    }

    return 0;
}

// --- COMPUTE_BCD_CELLS

static int handle_side_in(const bcd_event_t curr_evt,
                          cvector_vector_type(bcd_cell_t) * cell_list)
{
    bcd_cell_t new_cell = fill_bcd_cell(curr_evt.polygon_vertex,
                                        curr_evt.ceiling_edge,
                                        (point_t){0},
                                        (point_t){0},
                                        curr_evt.floor_edge,
                                        curr_evt.polygon_vertex,
                                        (bcd_neighbor_list_t){0},
                                        true,
                                        false,
                                        false);

    cvector_push_back(*cell_list, new_cell);

    return 0;
}

static int handle_in(const bcd_event_t curr_evt,
                     cvector_vector_type(bcd_cell_t) * cell_list)
{
    bcd_cell_t *prev_cell = NULL;
    point_t c_point;
    point_t f_point;

    in_find_prev_cell(curr_evt, cell_list, &prev_cell, &c_point, &f_point);
    if (prev_cell == NULL)
    {
        printf("Error: No previous cell found for IN event\n");
        return -3;
    }

    size_t prev_cell_index = prev_cell - &(*cell_list)[0];

    if (cvector_size(prev_cell->ceiling_edge_list) == 0)
    {
        printf("Error: prev_cell has empty ceiling_edge_list in handle_in\n");
        return -4;
    }
    if (cvector_size(prev_cell->floor_edge_list) == 0)
    {
        printf("Error: prev_cell has empty floor_edge_list in handle_in\n");
        return -5;
    }

    bcd_neighbor_list_t top_nl = {0};
    add_head_cell_neighbor_list(&top_nl, (int)prev_cell_index);

    bcd_cell_t top_cell = fill_bcd_cell(c_point,
                                        *cvector_back(prev_cell->ceiling_edge_list),
                                        (point_t){0},
                                        (point_t){0},
                                        curr_evt.floor_edge,
                                        curr_evt.polygon_vertex,
                                        top_nl,
                                        true,
                                        false,
                                        false);

    cvector_push_back(*cell_list, top_cell);
    size_t top_cell_index = cvector_size(*cell_list) - 1;

    bcd_neighbor_list_t bottom_nl = {0};
    add_head_cell_neighbor_list(&bottom_nl, (int)prev_cell_index);

    bcd_cell_t bottom_cell = fill_bcd_cell(curr_evt.polygon_vertex,
                                           curr_evt.ceiling_edge,
                                           (point_t){0},
                                           (point_t){0},
                                           *cvector_back(prev_cell->floor_edge_list),
                                           f_point,
                                           bottom_nl,
                                           true,
                                           false,
                                           false);

    cvector_push_back(*cell_list, bottom_cell);
    size_t bottom_cell_index = cvector_size(*cell_list) - 1;

    prev_cell = &(*cell_list)[prev_cell_index];
    bcd_cell_t *top_cell_ptr = &(*cell_list)[top_cell_index];
    bcd_cell_t *bottom_cell_ptr = &(*cell_list)[bottom_cell_index];

    update_bcd_cell(prev_cell,
                    c_point,
                    f_point,
                    IN,
                    (int)top_cell_index,
                    (int)bottom_cell_index);

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

        // Safety checks for empty edge lists
        if (cvector_size(cell->ceiling_edge_list) == 0 ||
            cvector_size(cell->floor_edge_list) == 0)
            continue;

        polygon_edge_t ceil_edge = cell->ceiling_edge_list[(cvector_size(cell->ceiling_edge_list)) - 1];
        point_t c_intersection;
        float evt_to_ceil_dist = calc_evt_to_edge_dist(curr_evt.polygon_vertex, ceil_edge, &c_intersection);

        polygon_edge_t floor_edge = cell->floor_edge_list[(cvector_size(cell->floor_edge_list)) - 1];
        point_t f_intersection;
        float evt_to_floor_dist = calc_evt_to_edge_dist(curr_evt.polygon_vertex, floor_edge, &f_intersection);

        if (evt_to_ceil_dist < min_evt_to_ceil_dist &&
            evt_to_floor_dist < min_evt_to_floor_dist &&
            c_intersection.y < curr_evt.polygon_vertex.y &&
            f_intersection.y > curr_evt.polygon_vertex.y)
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

static int handle_side_out(const bcd_event_t curr_evt,
                           cvector_vector_type(bcd_cell_t) * cell_list)
{
    bcd_cell_t *cell = NULL;
    size_t i;
    for (i = 0; i < cvector_size(*cell_list); ++i)
    {
        cell = &(*cell_list)[i];

        // Safety checks for empty edge lists
        if (cvector_size(cell->ceiling_edge_list) == 0 ||
            cvector_size(cell->floor_edge_list) == 0)
            continue;

        point_t ceil_edge_end = cvector_back(cell->ceiling_edge_list)->end;
        point_t floor_edge_begin = cvector_back(cell->floor_edge_list)->begin;

        if (are_equal_points(curr_evt.polygon_vertex, ceil_edge_end) &&
            are_equal_points(curr_evt.polygon_vertex, floor_edge_begin))
        {
            break;
        }
    }

    // Check if we found a valid cell
    if (i >= cvector_size(*cell_list))
    {
        printf("Error: No matching cell found in handle_side_out\n");
        return -1;
    }

    update_bcd_cell(cell,
                    curr_evt.polygon_vertex,
                    curr_evt.polygon_vertex,
                    SIDE_OUT,
                    -1,
                    -1);

    return 0;
}

static int handle_out(const bcd_event_t curr_evt,
                      cvector_vector_type(bcd_cell_t) * cell_list)
{
    // TOP CELL
    bcd_cell_t *top_cell = NULL;
    point_t c_pt;

    out_find_top_cell(curr_evt, cell_list, &top_cell, &c_pt);

    if (top_cell == NULL)
    {
        printf("Error: Failed to find top cell in handle_out\n");
        return -1;
    }

    // BOTTOM CELL
    bcd_cell_t *bottom_cell = NULL;
    point_t f_pt;

    out_find_bottom_cell(curr_evt, cell_list, &bottom_cell, &f_pt);

    if (bottom_cell == NULL)
    {
        printf("Error: Failed to find bottom cell in handle_out\n");
        return -1;
    }

    // NEW CELL
    bcd_cell_t new_cell;
    bcd_neighbor_list_t new_nl = {0};
    
    // Find indices of top_cell and bottom_cell
    int top_cell_index = -1;
    int bottom_cell_index = -1;
    
    for (size_t k = 0; k < cvector_size(*cell_list); ++k) {
        if (&(*cell_list)[k] == top_cell) {
            top_cell_index = (int)k;
        }
        if (&(*cell_list)[k] == bottom_cell) {
            bottom_cell_index = (int)k;
        }
    }
    
    if (top_cell_index != -1) {
        add_tail_cell_neighbor_list(&new_nl, top_cell_index);
    }
    if (bottom_cell_index != -1) {
        add_tail_cell_neighbor_list(&new_nl, bottom_cell_index);
    }

    new_cell = fill_bcd_cell(c_pt,
                             *cvector_back(top_cell->ceiling_edge_list),
                             (point_t){0},
                             (point_t){0},
                             *cvector_back(bottom_cell->floor_edge_list),
                             f_pt,
                             new_nl,
                             true,
                             false,
                             false);

    cvector_push_back(*cell_list, new_cell);

    // UPDATING CELLS
    size_t new_cell_index = cvector_size(*cell_list) - 1;

    update_bcd_cell(top_cell,
                    c_pt,
                    curr_evt.polygon_vertex,
                    OUT,
                    (int)new_cell_index,
                    -1);

    update_bcd_cell(bottom_cell,
                    curr_evt.polygon_vertex,
                    f_pt,
                    OUT,
                    (int)new_cell_index,
                    -1);

    return 0;
}

// --- --- HANDLE_OUT

static void out_find_top_cell(const bcd_event_t curr_evt,
                              cvector_vector_type(bcd_cell_t) * cell_list,
                              bcd_cell_t **top_cell,
                              point_t *c_pt)
{
    if (!cell_list || !*cell_list)
    {
        printf("BCD Cell List: NULL or empty\n");
        return;
    }

    bcd_cell_t *cell = NULL;
    point_t floor_edge_begin;

    size_t i;
    for (i = 0; i < cvector_size(*cell_list); ++i)
    {
        cell = &(*cell_list)[i];

        if (!cell->open)
            continue;

        // Safety check for empty edge list
        if (cvector_size(cell->floor_edge_list) == 0)
            continue;

        floor_edge_begin = cvector_back(cell->floor_edge_list)->begin;

        if (!are_equal_points(curr_evt.polygon_vertex, floor_edge_begin))
            continue;

        break;
    }

    // Check if we found a valid cell
    if (i >= cvector_size(*cell_list))
    {
        printf("Error: No matching top cell found in out_find_top_cell\n");
        *top_cell = NULL;
        return;
    }

    *top_cell = cell;

    // Safety check for valid cell and non-empty edge list
    if (*top_cell == NULL || cvector_size((*top_cell)->ceiling_edge_list) == 0)
    {
        printf("Error: Invalid top_cell or empty ceiling_edge_list in out_find_top_cell\n");
        return;
    }

    polygon_edge_t ceil_edge = (*top_cell)->ceiling_edge_list[(cvector_size((*top_cell)->ceiling_edge_list)) - 1];
    float evt_to_ceil_dist = calc_evt_to_edge_dist(curr_evt.polygon_vertex, ceil_edge, c_pt);
}

static void out_find_bottom_cell(const bcd_event_t curr_evt,
                                 cvector_vector_type(bcd_cell_t) * cell_list,
                                 bcd_cell_t **bottom_cell,
                                 point_t *f_pt)
{
    if (!cell_list || !*cell_list)
    {
        printf("BCD Cell List: NULL or empty\n");
        return;
    }

    bcd_cell_t *cell = NULL;
    point_t ceil_edge_end;

    size_t i;
    for (i = 0; i < cvector_size(*cell_list); ++i)
    {
        cell = &(*cell_list)[i];

        if (!cell->open)
            continue;

        // Safety check for empty edge list
        if (cvector_size(cell->ceiling_edge_list) == 0)
            continue;

        ceil_edge_end = cvector_back(cell->ceiling_edge_list)->end;

        if (!are_equal_points(curr_evt.polygon_vertex, ceil_edge_end))
            continue;

        break;
    }

    // Check if we found a valid cell
    if (i >= cvector_size(*cell_list))
    {
        printf("Error: No matching bottom cell found in out_find_bottom_cell\n");
        *bottom_cell = NULL;
        return;
    }

    *bottom_cell = cell;

    // Safety check for valid cell and non-empty edge list
    if (*bottom_cell == NULL || cvector_size((*bottom_cell)->floor_edge_list) == 0)
    {
        printf("Error: Invalid bottom_cell or empty floor_edge_list in out_find_bottom_cell\n");
        return;
    }

    polygon_edge_t floor_edge = (*bottom_cell)->floor_edge_list[(cvector_size((*bottom_cell)->floor_edge_list)) - 1];
    float evt_to_ceil_dist = calc_evt_to_edge_dist(curr_evt.polygon_vertex, floor_edge, f_pt);
}

// ---

static int handle_floor(const bcd_event_t curr_evt,
                        cvector_vector_type(bcd_cell_t) * cell_list)
{
    bcd_cell_t *cell = NULL;

    size_t i;
    for (i = 0; i < cvector_size(*cell_list); ++i)
    {
        cell = &(*cell_list)[i];

        if (!cell->open)
            continue;

        point_t f_edge_begin;
        f_edge_begin = cvector_back(cell->floor_edge_list)->begin;

        if (are_equal_points(curr_evt.polygon_vertex, f_edge_begin))
        {
            break;
        }
    }

    cvector_push_back(cell->floor_edge_list, curr_evt.floor_edge);

    return 0;
}

static int handle_ceiling(const bcd_event_t curr_evt,
                          cvector_vector_type(bcd_cell_t) * cell_list)
{
    bcd_cell_t *cell = NULL;

    size_t i;
    for (i = 0; i < cvector_size(*cell_list); ++i)
    {
        cell = &(*cell_list)[i];

        if (!cell->open)
            continue;

        point_t c_edge_end;
        c_edge_end = cvector_back(cell->ceiling_edge_list)->end;

        if (are_equal_points(curr_evt.polygon_vertex, c_edge_end))
            break;
    }

    cvector_push_back(cell->ceiling_edge_list, curr_evt.ceiling_edge);

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

static void update_bcd_cell(bcd_cell_t *cell,
                            point_t c_pt,
                            point_t f_pt,
                            bcd_event_type_t evt_type,
                            int top_cell_index,
                            int bottom_cell_index)
{
    cell->c_end = c_pt;
    cell->f_begin = f_pt;

    bcd_neighbor_list_t *neighbor_list = &cell->neighbor_list;

    if (evt_type == IN)
    {
        if (top_cell_index >= 0)
            add_head_cell_neighbor_list(neighbor_list, top_cell_index);
        if (bottom_cell_index >= 0)
            add_head_cell_neighbor_list(neighbor_list, bottom_cell_index);
    }

    if (evt_type == OUT)
    {
        if (top_cell_index >= 0)
            add_tail_cell_neighbor_list(neighbor_list, top_cell_index);
        if (bottom_cell_index >= 0)
            add_tail_cell_neighbor_list(neighbor_list, bottom_cell_index);
    }

    cell->open = false;
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
                                        int cell_index)
{
    if (!neighbor_list || cell_index < 0)
        return;

    bcd_neighbor_node_t *new_node = (bcd_neighbor_node_t *)malloc(sizeof(bcd_neighbor_node_t));
    if (!new_node)
        return;

    new_node->cell_index = cell_index;
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
                                        int cell_index)
{
    if (!neighbor_list || cell_index < 0)
        return;

    bcd_neighbor_node_t *new_node = (bcd_neighbor_node_t *)malloc(sizeof(bcd_neighbor_node_t));
    if (!new_node)
        return;

    new_node->cell_index = cell_index;
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
        printf("    open: %s\n", cell->open ? "true" : "false");
        printf("    visited: %s\n", cell->visited ? "true" : "false");
        printf("    cleaned: %s\n", cell->cleaned ? "true" : "false");

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

        printf("    neighbor_list: count=%d\n", cell->neighbor_list.count);

        if (cell->neighbor_list.head)
        {
            printf("    neighbor nodes:\n");
            bcd_neighbor_node_t *current = cell->neighbor_list.head;
            int node_index = 0;
            while (current)
            {
                int cell_index = current->cell_index;
                
                if (cell_index >= 0 && cell_index < (int)cvector_size(*cell_list))
                {
                    const bcd_cell_t *neighbor_cell = &(*cell_list)[cell_index];
                    printf("      [%d]: cell_index=%d cell_pos=(%.2f,%.2f)", 
                           node_index, cell_index, 
                           neighbor_cell->c_begin.x, neighbor_cell->c_begin.y);
                }
                else
                {
                    printf("      [%d]: cell_index=%d (INVALID INDEX)", node_index, cell_index);
                }
                printf("\n");
                current = current->next;
                node_index++;

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
