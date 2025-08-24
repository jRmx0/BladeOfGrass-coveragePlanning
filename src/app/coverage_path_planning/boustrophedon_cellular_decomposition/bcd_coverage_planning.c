#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "../../../../dependencies/cvector/cvector.h"
#include "bcd_coverage_planning.h"

// COMPUTE_BCD_PATH_LIST

static void add_cell_to_path(cvector_vector_type(int) * path_list,
                             cvector_vector_type(bcd_cell_t) * cell_list,
                             int cell_index,
                             int *visited_count);

static bool all_cells_visited(int visited_count, int total_cells);

int find_unvisited_neighbor(int curr_cell_index,
                            cvector_vector_type(bcd_cell_t) * cell_list);

static void add_shortest_path_to_list(cvector_vector_type(int) * path_list,
                                      cvector_vector_type(bcd_cell_t) * cell_list,
                                      int target_cell_index,
                                      int *visited_count);

// --- ADD_SHORTEST_PATH_TO_LIST

cvector_vector_type(int) find_shortest_path(int cell_index_from,
                                            int cell_index_to,
                                            cvector_vector_type(bcd_cell_t) * cell_list);

// ---

static bool should_backtrack(int *curr_path_index);

// COMPUTE_BCD_MOTION

// --- COMPUTE_BCD_MOTION

static cvector_vector_type(point_t) compute_boustrophedon_motion(const cvector_vector_type(bcd_cell_t) * cell_list,
                                                                 int cell_index,
                                                                 float step_size);

// IMPLEMENTATION --- compute_bcd_path_list -------------------------

int compute_bcd_path_list(cvector_vector_type(bcd_cell_t) * cell_list,
                          int starting_cell_index,
                          cvector_vector_type(int) * path_list)
{
    if (cell_list == NULL || path_list == NULL)
    {
        printf("compute_bcd_path_list: Invalid input parameters\n");
        return -1;
    }

    int cell_count = cvector_size(*cell_list);
    if (cell_count == 0)
    {
        printf("compute_bcd_path_list: No cells to process\n");
        return 0;
    }

    if (starting_cell_index == -1)
        starting_cell_index = 0;

    int visited_count = 0;
    bool search_shortest_path = false;
    int curr_path_index = 0;

    // Initialize with starting cell
    add_cell_to_path(path_list,
                     cell_list,
                     starting_cell_index,
                     &visited_count);

    while (!all_cells_visited(visited_count, cell_count))
    {
        int current_cell = (*path_list)[curr_path_index];
        int next_cell = find_unvisited_neighbor(current_cell, cell_list);

        if (next_cell != -1)
        {
            if (search_shortest_path)
            {
                add_shortest_path_to_list(path_list,
                                          cell_list,
                                          next_cell,
                                          &visited_count);
                search_shortest_path = false;
            }
            else
            {
                add_cell_to_path(path_list,
                                 cell_list,
                                 next_cell,
                                 &visited_count);
            }

            curr_path_index = cvector_size(*path_list) - 1;
        }
        else
        {
            search_shortest_path = true;

            if (should_backtrack(&curr_path_index))
                return -2;
        }
    }

    add_shortest_path_to_list(path_list,
                              cell_list,
                              starting_cell_index,
                              &visited_count);

    return 0;
}

// --- COMPUTE_BCD_PATH_LIST

static void add_cell_to_path(cvector_vector_type(int) * path_list,
                             cvector_vector_type(bcd_cell_t) * cell_list,
                             int cell_index,
                             int *visited_count)
{
    cvector_push_back(*path_list, cell_index);
    (*cell_list)[cell_index].visited = true;
    (*visited_count)++;
}

static bool all_cells_visited(int visited_count, int total_cells)
{
    return visited_count == total_cells;
}

int find_unvisited_neighbor(int curr_cell_index,
                            cvector_vector_type(bcd_cell_t) * cell_list)
{
    bcd_neighbor_node_t curr_neighbor_node;
    int curr_neighbor_index = -1;

    if ((*cell_list)[curr_cell_index].neighbor_list.count == 0)
    {
        return -1;
    }

    curr_neighbor_node = *(*cell_list)[curr_cell_index].neighbor_list.head;
    curr_neighbor_index = curr_neighbor_node.cell_index;

    if ((*cell_list)[curr_neighbor_index].visited == false)
    {
        return curr_neighbor_index;
    }

    for (int i = 1; i < (*cell_list)[curr_cell_index].neighbor_list.count; i++)
    {
        curr_neighbor_node = *curr_neighbor_node.next;
        curr_neighbor_index = curr_neighbor_node.cell_index;

        if ((*cell_list)[curr_neighbor_index].visited == false)
        {
            return curr_neighbor_index;
        }
    }

    return -1;
}

static void add_shortest_path_to_list(cvector_vector_type(int) * path_list,
                                      cvector_vector_type(bcd_cell_t) * cell_list,
                                      int target_cell_index,
                                      int *visited_count)
{
    int last_cell_index = (*path_list)[cvector_size(*path_list) - 1];
    cvector_vector_type(int) shortest_path = find_shortest_path(last_cell_index, target_cell_index, cell_list);

    // Add intermediate cells from shortest path (skip first and last)
    for (size_t i = 1; i < cvector_size(shortest_path) - 1; ++i)
    {
        cvector_push_back(*path_list, shortest_path[i]);
    }

    cvector_free(shortest_path);
    add_cell_to_path(path_list, cell_list, target_cell_index, visited_count);
}

// --- --- ADD_SHORTEST_PATH_TO_LIST

cvector_vector_type(int) find_shortest_path(int cell_index_from,
                                            int cell_index_to,
                                            cvector_vector_type(bcd_cell_t) * cell_list)
{
    cvector_vector_type(int) path = NULL;

    if (cell_list == NULL || cell_index_from < 0 || cell_index_to < 0)
    {
        return path;
    }

    int cell_count = cvector_size(*cell_list);
    if (cell_index_from >= cell_count || cell_index_to >= cell_count)
    {
        return path;
    }

    // If source and destination are the same
    if (cell_index_from == cell_index_to)
    {
        cvector_push_back(path, cell_index_from);
        return path;
    }

    // BFS data structures
    cvector_vector_type(int) queue = NULL;
    cvector_vector_type(int) parent = NULL;
    cvector_vector_type(bool) visited = NULL;

    // Initialize arrays
    for (int i = 0; i < cell_count; i++)
    {
        cvector_push_back(parent, -1);
        cvector_push_back(visited, false);
    }

    // Start BFS
    cvector_push_back(queue, cell_index_from);
    visited[cell_index_from] = true;

    bool found = false;

    while (cvector_size(queue) > 0 && !found)
    {
        int current_cell = queue[0];

        // Remove first element from queue
        for (int i = 0; i < cvector_size(queue) - 1; i++)
        {
            queue[i] = queue[i + 1];
        }
        cvector_pop_back(queue);

        // Check all neighbors
        bcd_neighbor_node_t *neighbor_node = (*cell_list)[current_cell].neighbor_list.head;

        while (neighbor_node != NULL)
        {
            int neighbor_index = neighbor_node->cell_index;

            if (!visited[neighbor_index])
            {
                visited[neighbor_index] = true;
                parent[neighbor_index] = current_cell;
                cvector_push_back(queue, neighbor_index);

                if (neighbor_index == cell_index_to)
                {
                    found = true;
                    break;
                }
            }

            neighbor_node = neighbor_node->next;
        }
    }

    // Reconstruct path if found
    if (found)
    {
        cvector_vector_type(int) temp_path = NULL;
        int current = cell_index_to;

        while (current != -1)
        {
            cvector_push_back(temp_path, current);
            current = parent[current];
        }

        // Reverse the path
        int path_length = cvector_size(temp_path);
        for (int i = path_length - 1; i >= 0; i--)
        {
            cvector_push_back(path, temp_path[i]);
        }

        cvector_free(temp_path);
    }

    // Cleanup
    cvector_free(queue);
    cvector_free(parent);
    cvector_free(visited);

    return path;
}

// ---

static bool should_backtrack(int *curr_path_index)
{
    (*curr_path_index)--;
    return (*curr_path_index) < 0;
}

// PATH_LIST HELPERS

void log_bcd_path_list(const cvector_vector_type(int) * path_list)
{
    if (path_list == NULL)
    {
        printf("BCD Path List: NULL\n");
        return;
    }

    int path_count = cvector_size(*path_list);
    printf("BCD Path List (%d cells):\n", path_count);

    if (path_count == 0)
    {
        printf("  (empty)\n");
        return;
    }

    for (int i = 0; i < path_count; i++)
    {
        printf("  [%d]: Cell %d\n", i, (*path_list)[i]);
    }
}

// IMPLEMENTATION --- compute_bcd_motion ----------------------------

int compute_bcd_motion(cvector_vector_type(bcd_cell_t) * cell_list,
                       const cvector_vector_type(int) * path_list,
                       bcd_motion_plan_t *motion_plan,
                       float step_size)
{
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

        cell_motion_plan_t curr_section;
        curr_section.ox = ox;
        curr_section.nav = NULL; // Navigation not implemented yet

        cvector_push_back(motion_plan->section, curr_section);

        (*cell_list)[(*path_list)[i]].cleaned = true;
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
    point_t floor_start = cell->f_end;
    point_t floor_end = cell->f_begin;
    
    // Calculate the sweep direction (perpendicular to the cell)
    // Assuming cells are oriented vertically, sweep horizontally
    float cell_width = ceiling_end.x - ceiling_start.x;
    float cell_height = fabs(ceiling_start.y - floor_start.y);
    
    if (cell_width <= 0 || step_size <= 0)
    {
        return ox;
    }
    
    // Calculate number of sweep lines
    int num_lines = (int)(cell_width / step_size) + 1;
    
    // Generate boustrophedon pattern as a continuous path
    bool going_down = true; // Start by going from ceiling to floor
    
    for (int i = 0; i < num_lines; i++)
    {
        float x_offset = i * step_size;
        float current_x = ceiling_start.x + x_offset;
        
        // Don't exceed the cell boundary
        if (current_x > ceiling_end.x)
        {
            current_x = ceiling_end.x;
        }
        
        // Calculate y-coordinates for this vertical line
        // Interpolate between ceiling and floor based on x position
        float t = (current_x - ceiling_start.x) / (ceiling_end.x - ceiling_start.x);
        
        float ceiling_y = ceiling_start.y + t * (ceiling_end.y - ceiling_start.y);
        float floor_y = floor_start.y + t * (floor_end.y - floor_start.y);
        
        point_t start_point, end_point;
        
        if (going_down)
        {
            // Go from ceiling to floor
            start_point.x = current_x;
            start_point.y = ceiling_y;
            end_point.x = current_x;
            end_point.y = floor_y;
        }
        else
        {
            // Go from floor to ceiling
            start_point.x = current_x;
            start_point.y = floor_y;
            end_point.x = current_x;
            end_point.y = ceiling_y;
        }
        
        // For the first line, add the start point
        if (i == 0)
        {
            cvector_push_back(ox, start_point);
        }
        
        // Always add the end point (this creates the continuous path)
        cvector_push_back(ox, end_point);
        
        // For lines except the last one, add the connecting point to next line
        if (i < num_lines - 1)
        {
            // Calculate the start point of the next line
            float next_x_offset = (i + 1) * step_size;
            float next_x = ceiling_start.x + next_x_offset;
            
            // Don't exceed the cell boundary
            if (next_x > ceiling_end.x)
            {
                next_x = ceiling_end.x;
            }
            
            // Calculate y-coordinates for next vertical line
            float next_t = (next_x - ceiling_start.x) / (ceiling_end.x - ceiling_start.x);
            float next_ceiling_y = ceiling_start.y + next_t * (ceiling_end.y - ceiling_start.y);
            float next_floor_y = floor_start.y + next_t * (floor_end.y - floor_start.y);
            
            point_t next_start;
            if (!going_down) // Next line will go down
            {
                next_start.x = next_x;
                next_start.y = next_ceiling_y;
            }
            else // Next line will go up
            {
                next_start.x = next_x;
                next_start.y = next_floor_y;
            }
            
            // Add the connecting point (same as next line's start)
            cvector_push_back(ox, next_start);
        }
        
        // Alternate direction for next line
        going_down = !going_down;
    }

    return ox;
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