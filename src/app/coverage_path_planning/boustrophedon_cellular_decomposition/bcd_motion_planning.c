#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "../../../../dependencies/cvector/cvector.h"

#include "coverage_path_planning.h"
#include "bcd_cell_computation.h"
#include "bcd_motion_planning.h"

// --- COMPUTE_BCD_MOTION

static cvector_vector_type(point_t) compute_boustrophedon_motion(const cvector_vector_type(bcd_cell_t) * cell_list,
                                                                 int cell_index,
                                                                 float step_size);

// --- --- COMPUTE_BOUSTROPHEDON_MOTION

static int find_intersecting_edge_index(float x,
                                        const polygon_edge_t *edge_list,
                                        int edge_count);

// --- --- --- FIND_INTERSECTING_EDGE_INDEX

static bool is_x_in_edge_range(float x,
                               const polygon_edge_t *edge);

// --- ---

static point_t find_intersection_point(float x,
                                       const polygon_edge_t *edge_list,
                                       int edge_count);

// --- --- --- FIND_INTERSECTION__POINT

static float find_y_intersection(float x,
                                 const polygon_edge_t *edge);

// --- ---

static void add_edge_transition_path(cvector_vector_type(point_t) * path,
                                     const polygon_edge_t *edge_list,
                                     int edge_count,
                                     int start_edge_index,
                                     int end_edge_index,
                                     float start_x,
                                     float end_x);

// --- --- COMPUTE_BOUSTROPHEDON_MOTION

static cvector_vector_type(point_t) compute_connection_motion(const cvector_vector_type(bcd_cell_t) * cell_list,
                                                              const cvector_vector_type(int) * path_list,
                                                              int begin_cell_index,
                                                              point_t begin_point,
                                                              int end_cell_index,
                                                              point_t end_point);

// IMPLEMENTATION --- compute_bcd_motion ----------------------------

int compute_bcd_motion(cvector_vector_type(bcd_cell_t) * cell_list,
                       const cvector_vector_type(int) * path_list,
                       bcd_motion_plan_t *motion_plan,
                       float step_size)
{
    int begin_cell_index;
    point_t begin_point = {0};
    int end_cell_index;
    point_t end_point = {0};
    bool compute_nav = false;

    size_t i;
    for (i = 0; i < cvector_size(*path_list); ++i)
    {
        if ((*cell_list)[(*path_list)[i]].cleaned == true)
        {
            continue;
        }

        cvector_vector_type(point_t) ox = NULL;
        ox = compute_boustrophedon_motion((const cvector_vector_type(bcd_cell_t) *)cell_list,
                                          (*path_list)[i],
                                          step_size);
        if (ox == NULL)
        {
            return -1;
        }

        cvector_vector_type(point_t) nav = NULL;

        if (compute_nav)
        {
            end_cell_index = (*path_list)[i];
            end_point = *cvector_front(ox);

            nav = compute_connection_motion((const cvector_vector_type(bcd_cell_t) *)cell_list,
                                            (const cvector_vector_type(int) *)path_list,
                                            begin_cell_index,
                                            begin_point,
                                            end_cell_index,
                                            end_point);
        }

        cell_motion_plan_t curr_section;
        curr_section.ox = ox;
        curr_section.nav = nav; // Navigation not implemented yet

        cvector_push_back(motion_plan->section, curr_section);

        (*cell_list)[(*path_list)[i]].cleaned = true;

        begin_cell_index = (*path_list)[i];
        begin_point = *cvector_back(ox);
        compute_nav = true;
    }

    return 0;
}

// --- COMPUTE_BCD_MOTION

static cvector_vector_type(point_t) compute_boustrophedon_motion(const cvector_vector_type(bcd_cell_t) * cell_list,
                                                                 int cell_index,
                                                                 float step_size) // the distance between two parallel line segments
{
    cvector_vector_type(point_t) ox = NULL;

    if (cell_list == NULL || cell_index < 0 || cell_index >= cvector_size(*cell_list))
    {
        return ox;
    }

    const bcd_cell_t *cell = &(*cell_list)[cell_index];

    // Get cell boundaries
    point_t ceiling_start = cell->c_begin;
    point_t ceiling_end = cell->c_end;
    point_t floor_start = cell->f_begin;
    point_t floor_end = cell->f_end;

    // Calculate the sweep direction (perpendicular to the cell)
    // Assuming cells are oriented vertically, sweep horizontally
    float cell_width = ceiling_end.x - ceiling_start.x;

    if (cell_width <= 0 || step_size <= 0)
    {
        return ox;
    }

    // Calculate number of sweep lines
    int num_lines = (int)(cell_width / step_size) + 1;

    // Generate boustrophedon pattern as a continuous path
    bool going_down = true; // Start by going from ceiling to floor
    int last_ceiling_edge_index = -1;
    int last_floor_edge_index = -1;

    for (int i = 0; i < num_lines; i++)
    {
        float x_offset = i * step_size;
        float current_x = ceiling_start.x + x_offset;

        // Don't exceed the cell boundary
        if (current_x > ceiling_end.x)
        {
            current_x = ceiling_end.x;
        }

        // Find which edges we're intersecting with
        int current_ceiling_edge_index = find_intersecting_edge_index(current_x, cell->ceiling_edge_list, cvector_size(cell->ceiling_edge_list));
        int current_floor_edge_index = find_intersecting_edge_index(current_x, cell->floor_edge_list, cvector_size(cell->floor_edge_list));

        point_t start_point, end_point;

        if (going_down)
        {
            // Going from ceiling to floor
            start_point = find_intersection_point(current_x, cell->ceiling_edge_list, cvector_size(cell->ceiling_edge_list));
            end_point = find_intersection_point(current_x, cell->floor_edge_list, cvector_size(cell->floor_edge_list));
        }
        else
        {
            // Going from floor to ceiling
            start_point = find_intersection_point(current_x, cell->floor_edge_list, cvector_size(cell->floor_edge_list));
            end_point = find_intersection_point(current_x, cell->ceiling_edge_list, cvector_size(cell->ceiling_edge_list));
        }

        // For the first line, add the start point
        if (i == 0)
        {
            cvector_push_back(ox, start_point);
        }

        // Always add the end point (this completes the current sweep line)
        cvector_push_back(ox, end_point);

        // Handle edge transitions and connecting to next line
        if (i < num_lines - 1)
        {
            // Calculate the next line position
            float next_x_offset = (i + 1) * step_size;
            float next_x = ceiling_start.x + next_x_offset;

            // Don't exceed the cell boundary
            if (next_x > ceiling_end.x)
            {
                next_x = ceiling_end.x;
            }

            // Find which edges the next line will intersect with
            int next_ceiling_edge_index = find_intersecting_edge_index(next_x, cell->ceiling_edge_list, cvector_size(cell->ceiling_edge_list));
            int next_floor_edge_index = find_intersecting_edge_index(next_x, cell->floor_edge_list, cvector_size(cell->floor_edge_list));

            // Determine which boundary we need to follow for transition
            if (going_down)
            {
                // Current sweep ended at floor, next will start at floor
                // Check if we need to follow floor edges between current and next position
                if (last_floor_edge_index != -1 && next_floor_edge_index != -1 &&
                    current_floor_edge_index != next_floor_edge_index)
                {
                    // Add transition path following floor boundary
                    add_edge_transition_path(&ox, cell->floor_edge_list, cvector_size(cell->floor_edge_list),
                                             current_floor_edge_index, next_floor_edge_index, current_x, next_x);
                }
            }
            else
            {
                // Current sweep ended at ceiling, next will start at ceiling
                // Check if we need to follow ceiling edges between current and next position
                if (last_ceiling_edge_index != -1 && next_ceiling_edge_index != -1 &&
                    current_ceiling_edge_index != next_ceiling_edge_index)
                {
                    // Add transition path following ceiling boundary
                    add_edge_transition_path(&ox, cell->ceiling_edge_list, cvector_size(cell->ceiling_edge_list),
                                             current_ceiling_edge_index, next_ceiling_edge_index, current_x, next_x);
                }
            }

            // Calculate the start point of the next line
            point_t next_start;
            if (!going_down) // Next line will go down (ceiling to floor)
            {
                next_start = find_intersection_point(next_x, cell->ceiling_edge_list, cvector_size(cell->ceiling_edge_list));
            }
            else // Next line will go up (floor to ceiling)
            {
                next_start = find_intersection_point(next_x, cell->floor_edge_list, cvector_size(cell->floor_edge_list));
            }

            // Add the start point of next line
            cvector_push_back(ox, next_start);
        }

        // Update last edge indices for next iteration
        last_ceiling_edge_index = current_ceiling_edge_index;
        last_floor_edge_index = current_floor_edge_index;

        // Alternate direction for next line
        going_down = !going_down;
    }

    return ox;
}

// --- --- COMPUTE_BOUSTROPHEDON_MOTION

static int find_intersecting_edge_index(float x, const polygon_edge_t *edge_list, int edge_count)
{
    if (edge_list == NULL || edge_count == 0)
    {
        return -1;
    }

    for (int i = 0; i < edge_count; i++)
    {
        if (is_x_in_edge_range(x, &edge_list[i]))
        {
            return i;
        }
    }

    return -1; // No intersecting edge found
}

// --- --- --- FIND_INTERSECTING_EDGE_INDEX

static bool is_x_in_edge_range(float x, const polygon_edge_t *edge)
{
    float min_x = (edge->begin.x < edge->end.x) ? edge->begin.x : edge->end.x;
    float max_x = (edge->begin.x > edge->end.x) ? edge->begin.x : edge->end.x;

    return (x >= min_x && x <= max_x);
}

// --- ---

static point_t find_intersection_point(float x, const polygon_edge_t *edge_list, int edge_count)
{
    point_t result = {x, 0.0f}; // Default point with x coordinate and y=0

    int edge_index = find_intersecting_edge_index(x, edge_list, edge_count);
    if (edge_index >= 0)
    {
        result.y = find_y_intersection(x, &edge_list[edge_index]);
    }
    else
    {
        // Fallback: if no edge found, try to use the first or last edge
        if (edge_list != NULL && edge_count > 0)
        {
            // Use the first edge as fallback
            result.y = find_y_intersection(x, &edge_list[0]);
        }
    }

    return result;
}

// --- --- --- FIND_INTERSECTION__POINT

static float find_y_intersection(float x, const polygon_edge_t *edge)
{
    // Linear interpolation between edge start and end points
    if (edge->end.x == edge->begin.x) // Vertical edge
    {
        return edge->begin.y; // Could be either begin or end y, they should be the same for vertical
    }

    float t = (x - edge->begin.x) / (edge->end.x - edge->begin.x);
    return edge->begin.y + t * (edge->end.y - edge->begin.y);
}

// --- ---

static void add_edge_transition_path(cvector_vector_type(point_t) * path,
                                     const polygon_edge_t *edge_list,
                                     int edge_count,
                                     int start_edge_index,
                                     int end_edge_index,
                                     float start_x,
                                     float end_x)
{
    if (start_edge_index == end_edge_index || start_edge_index == -1 || end_edge_index == -1)
    {
        return; // No transition needed
    }

    // Determine direction of traversal
    bool forward = start_edge_index < end_edge_index;

    if (forward)
    {
        // Moving forward through edges
        for (int i = start_edge_index; i < end_edge_index; i++)
        {
            const polygon_edge_t *edge = &edge_list[i];

            if (i == start_edge_index)
            {
                // For the first edge, add the endpoint closer to end_x
                if (fabs(edge->end.x - end_x) < fabs(edge->begin.x - end_x))
                {
                    cvector_push_back(*path, edge->end);
                }
                else
                {
                    cvector_push_back(*path, edge->begin);
                }
            }
            else
            {
                // For intermediate edges, add both endpoints to follow the boundary
                cvector_push_back(*path, edge->begin);
                cvector_push_back(*path, edge->end);
            }
        }
    }
    else
    {
        // Moving backward through edges
        for (int i = start_edge_index; i > end_edge_index; i--)
        {
            const polygon_edge_t *edge = &edge_list[i];

            if (i == start_edge_index)
            {
                // For the first edge, add the endpoint closer to end_x
                if (fabs(edge->begin.x - end_x) < fabs(edge->end.x - end_x))
                {
                    cvector_push_back(*path, edge->begin);
                }
                else
                {
                    cvector_push_back(*path, edge->end);
                }
            }
            else
            {
                // For intermediate edges, add both endpoints to follow the boundary
                cvector_push_back(*path, edge->end);
                cvector_push_back(*path, edge->begin);
            }
        }
    }
}

// --- --- HELPER FUNCTIONS FOR COMPUTE_CONNECTION_MOTION

// Find the best exit point from a cell boundary toward a target point
static point_t find_exit_point(const bcd_cell_t *cell, point_t target_point)
{
    point_t exit_point = {0};
    float min_distance = INFINITY;
    
    // Check ceiling edges
    if (cell->ceiling_edge_list != NULL)
    {
        for (size_t i = 0; i < cvector_size(cell->ceiling_edge_list); ++i)
        {
            const polygon_edge_t *edge = &cell->ceiling_edge_list[i];
            
            // Test both endpoints of the edge
            float dist_begin = sqrt(pow(edge->begin.x - target_point.x, 2) + pow(edge->begin.y - target_point.y, 2));
            float dist_end = sqrt(pow(edge->end.x - target_point.x, 2) + pow(edge->end.y - target_point.y, 2));
            
            if (dist_begin < min_distance)
            {
                min_distance = dist_begin;
                exit_point = edge->begin;
            }
            if (dist_end < min_distance)
            {
                min_distance = dist_end;
                exit_point = edge->end;
            }
        }
    }
    
    // Check floor edges
    if (cell->floor_edge_list != NULL)
    {
        for (size_t i = 0; i < cvector_size(cell->floor_edge_list); ++i)
        {
            const polygon_edge_t *edge = &cell->floor_edge_list[i];
            
            // Test both endpoints of the edge
            float dist_begin = sqrt(pow(edge->begin.x - target_point.x, 2) + pow(edge->begin.y - target_point.y, 2));
            float dist_end = sqrt(pow(edge->end.x - target_point.x, 2) + pow(edge->end.y - target_point.y, 2));
            
            if (dist_begin < min_distance)
            {
                min_distance = dist_begin;
                exit_point = edge->begin;
            }
            if (dist_end < min_distance)
            {
                min_distance = dist_end;
                exit_point = edge->end;
            }
        }
    }
    
    // Fallback to cell center if no edges found
    if (min_distance == INFINITY)
    {
        exit_point.x = (cell->c_begin.x + cell->c_end.x) / 2.0f;
        exit_point.y = (cell->c_begin.y + cell->c_end.y) / 2.0f;
    }
    
    return exit_point;
}

// Find the best entry point into a cell boundary from a source point
static point_t find_entry_point(const bcd_cell_t *cell, point_t source_point)
{
    // For simplicity, use the same logic as find_exit_point
    // In a more sophisticated implementation, this could consider
    // the direction of approach and cell geometry
    return find_exit_point(cell, source_point);
}

// Check if two cells are adjacent (share a boundary)
static bool are_cells_adjacent(const bcd_cell_t *cell1, const bcd_cell_t *cell2)
{
    // Check if cell2 is in cell1's neighbor list
    bcd_neighbor_node_t *neighbor = cell1->neighbor_list.head;
    while (neighbor != NULL)
    {
        // We need to find the cell index somehow - this is a limitation
        // For now, we'll use a simpler geometric check
        neighbor = neighbor->next;
    }
    
    // Simplified geometric adjacency check - cells are adjacent if they share x coordinates
    const float tolerance = 0.001f;
    
    // Check if cell boundaries are close (indicating shared boundary)
    bool x_overlap = (fabs(cell1->c_end.x - cell2->c_begin.x) < tolerance) ||
                     (fabs(cell1->c_begin.x - cell2->c_end.x) < tolerance);
                     
    if (x_overlap)
    {
        // Check if y ranges overlap
        float cell1_y_min = fmin(cell1->c_begin.y, cell1->c_end.y);
        float cell1_y_max = fmax(cell1->c_begin.y, cell1->c_end.y);
        float cell2_y_min = fmin(cell2->c_begin.y, cell2->c_end.y);
        float cell2_y_max = fmax(cell2->c_begin.y, cell2->c_end.y);
        
        return !(cell1_y_max < cell2_y_min || cell2_y_max < cell1_y_min);
    }
    
    return false;
}

// Find intersection point between a point and cell boundary (placeholder)
static point_t find_boundary_intersection(const bcd_cell_t *cell, point_t from_point, point_t to_point)
{
    // Simplified implementation - return midpoint of cell boundary
    point_t intersection;
    intersection.x = (cell->c_begin.x + cell->c_end.x) / 2.0f;
    intersection.y = (cell->c_begin.y + cell->c_end.y) / 2.0f;
    return intersection;
}

// --- --- HELPER FUNCTIONS FOR COMPUTE_CONNECTION_MOTION

// Find the best exit point from a cell boundary toward a target point
static point_t find_exit_point(const bcd_cell_t *cell, point_t target_point);

// Find the best entry point into a cell boundary from a source point  
static point_t find_entry_point(const bcd_cell_t *cell, point_t source_point);

// Check if two cells are adjacent (share a boundary)
static bool are_cells_adjacent(const bcd_cell_t *cell1, const bcd_cell_t *cell2);

// Find intersection point between a point and cell boundary
static point_t find_boundary_intersection(const bcd_cell_t *cell, point_t from_point, point_t to_point);

// --- --- COMPUTE_CONNECTION_MOTION

/**
 * Compute navigation path between two points in potentially different BCD cells.
 * 
 * This function implements a pathfinding algorithm that can handle:
 * 1. Same-cell navigation: Direct path when both points are in the same cell
 * 2. Adjacent-cell navigation: Path through shared boundaries between neighboring cells
 * 3. Multi-cell transit: Uses BFS shortest-path algorithm through intermediate cells
 * 
 * The algorithm finds exit and entry points on cell boundaries to create an optimal
 * navigation path that respects the BCD cell structure and boundaries.
 * 
 * @param cell_list Array of all BCD cells with boundary and neighbor information
 * @param path_list Sequence of cells to visit (provides context but not directly used)
 * @param begin_cell_index Index of the starting cell
 * @param begin_point Exact starting point within the beginning cell
 * @param end_cell_index Index of the destination cell  
 * @param end_point Exact destination point within the ending cell
 * @return Navigation path as array of points from begin_point to end_point, or NULL on error
 */
static cvector_vector_type(point_t) compute_connection_motion(const cvector_vector_type(bcd_cell_t) * cell_list,
                                                              const cvector_vector_type(int) * path_list,
                                                              int begin_cell_index,
                                                              point_t begin_point,
                                                              int end_cell_index,
                                                              point_t end_point)
{
    cvector_vector_type(point_t) nav_path = NULL;
    
    // Input validation
    if (!cell_list || begin_cell_index < 0 || end_cell_index < 0 ||
        begin_cell_index >= cvector_size(*cell_list) || end_cell_index >= cvector_size(*cell_list))
    {
        return nav_path;
    }
    
    // Case 1: Same cell - direct path
    if (begin_cell_index == end_cell_index)
    {
        cvector_push_back(nav_path, begin_point);
        cvector_push_back(nav_path, end_point);
        return nav_path;
    }
    
    const bcd_cell_t *begin_cell = &(*cell_list)[begin_cell_index];
    const bcd_cell_t *end_cell = &(*cell_list)[end_cell_index];
    
    // Case 2: Adjacent cells - find shared boundary
    if (are_cells_adjacent(begin_cell, end_cell))
    {
        cvector_push_back(nav_path, begin_point);
        
        // Find exit point from begin_cell toward end_point
        point_t exit_point = find_exit_point(begin_cell, end_point);
        cvector_push_back(nav_path, exit_point);
        
        // Find entry point into end_cell from exit_point
        point_t entry_point = find_entry_point(end_cell, exit_point);
        if (exit_point.x != entry_point.x || exit_point.y != entry_point.y)
        {
            cvector_push_back(nav_path, entry_point);
        }
        
        cvector_push_back(nav_path, end_point);
        return nav_path;
    }
    
    // Case 3: Non-adjacent cells - find path through transit cells
    cvector_vector_type(int) cell_path = find_shortest_path(begin_cell_index, end_cell_index, 
                                                            (cvector_vector_type(bcd_cell_t) *)cell_list);
    
    if (cell_path == NULL || cvector_size(cell_path) == 0)
    {
        // No path found - return direct line as fallback
        cvector_push_back(nav_path, begin_point);
        cvector_push_back(nav_path, end_point);
        cvector_free(cell_path);
        return nav_path;
    }
    
    cvector_push_back(nav_path, begin_point);
    
    // Navigate through each transit cell
    for (size_t i = 0; i < cvector_size(cell_path) - 1; ++i)
    {
        int current_cell_idx = cell_path[i];
        int next_cell_idx = cell_path[i + 1];
        
        const bcd_cell_t *current_cell = &(*cell_list)[current_cell_idx];
        const bcd_cell_t *next_cell = &(*cell_list)[next_cell_idx];
        
        // Find exit point from current cell
        point_t target;
        if (i == cvector_size(cell_path) - 2) 
        {
            target = end_point;
        } 
        else 
        {
            target.x = (next_cell->c_begin.x + next_cell->c_end.x) / 2.0f;
            target.y = (next_cell->c_begin.y + next_cell->c_end.y) / 2.0f;
        }
        
        point_t exit_point = find_exit_point(current_cell, target);
        cvector_push_back(nav_path, exit_point);
        
        // Find entry point into next cell
        point_t entry_point = find_entry_point(next_cell, exit_point);
        if (exit_point.x != entry_point.x || exit_point.y != entry_point.y)
        {
            cvector_push_back(nav_path, entry_point);
        }
    }
    
    cvector_push_back(nav_path, end_point);
    cvector_free(cell_path);
    
    return nav_path;
}

// MOTION_PLAN HELPERS

void log_bcd_motion(const bcd_motion_plan_t motion_plan)
{
    printf("BCD Motion Plan:\n");

    if (motion_plan.section == NULL)
    {
        printf("  (NULL motion plan)\n");
        return;
    }

    int section_count = cvector_size(motion_plan.section);
    printf("  Total sections: %d\n", section_count);

    if (section_count == 0)
    {
        printf("  (no sections)\n");
        return;
    }

    for (int i = 0; i < section_count; i++)
    {
        const cell_motion_plan_t *section = &motion_plan.section[i];
        printf("  Section %d:\n", i);

        // Log coverage motion (ox)
        if (section->ox == NULL)
        {
            printf("    Coverage: (NULL point list)\n");
        }
        else
        {
            int point_count = cvector_size(section->ox);
            printf("    Coverage points: %d (continuous path)\n", point_count);

            if (point_count == 0)
            {
                printf("    Coverage: (no points)\n");
            }
            else
            {
                // Log the continuous path points
                printf("    Path: ");
                for (int j = 0; j < point_count; j++)
                {
                    point_t point = section->ox[j];
                    printf("(%.2f, %.2f)", point.x, point.y);
                    if (j < point_count - 1)
                    {
                        printf(" -> ");
                    }

                    // Break line every 4 points for readability
                    if ((j + 1) % 4 == 0 && j < point_count - 1)
                    {
                        printf("\n          ");
                    }
                }
                printf("\n");
            }
        }

        // Log navigation motion (nav)
        if (section->nav == NULL)
        {
            printf("    Navigation: (NULL point list)\n");
        }
        else
        {
            int nav_count = cvector_size(section->nav);
            printf("    Navigation points: %d\n", nav_count);

            if (nav_count == 0)
            {
                printf("    Navigation: (no points)\n");
            }
            else
            {
                for (int j = 0; j < nav_count; j++)
                {
                    point_t nav_point = section->nav[j];
                    printf("      Nav %d: (%.2f, %.2f)\n", j, nav_point.x, nav_point.y);
                }
            }
        }
    }
}

void free_bcd_motion(bcd_motion_plan_t *motion_plan)
{
    if (motion_plan == NULL)
        return;

    if (motion_plan->section != NULL)
    {
        // Free each section's point vectors
        for (int i = 0; i < cvector_size(motion_plan->section); i++)
        {
            cvector_free(motion_plan->section[i].ox);
            cvector_free(motion_plan->section[i].nav);
        }

        cvector_free(motion_plan->section);
        motion_plan->section = NULL;
    }
}