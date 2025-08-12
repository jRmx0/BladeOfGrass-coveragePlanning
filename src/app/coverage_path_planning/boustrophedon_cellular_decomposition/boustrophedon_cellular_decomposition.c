#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "boustrophedon_cellular_decomposition.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Helper function to normalize angle to [0, 360) degrees
static float normalize_angle_degrees(float angle_degrees) {
    while (angle_degrees < 0.0f) {
        angle_degrees += 360.0f;
    }
    while (angle_degrees >= 360.0f) {
        angle_degrees -= 360.0f;
    }
    return angle_degrees;
}

// Helper function to convert radians to degrees
static float radians_to_degrees(float radians) {
    return radians * 180.0f / M_PI;
}

// Helper function to compute vector angle from begin to end point
static float compute_vector_angle_degrees(polygon_vertex_t begin, polygon_vertex_t end) {
    float dx = end.x - begin.x;
    float dy = end.y - begin.y;
    
    // Handle zero-length vector case
    if (dx == 0.0f && dy == 0.0f) {
        return 0.0f;
    }
    
    // Calculate angle using atan2 (returns radians in [-π, π])
    float angle_radians = atan2(dy, dx);
    
    // Convert to degrees and normalize to [0, 360)
    float angle_degrees = radians_to_degrees(angle_radians);
    return normalize_angle_degrees(angle_degrees);
}

// Main function to compute edge angle
float compute_edge_angle(polygon_edge_t poly_edge) {
    return compute_vector_angle_degrees(poly_edge.begin, poly_edge.end);
}

// Helper function to check if angle is in range (exclusive start, inclusive end)
static int angle_in_range_exclusive_inclusive(float angle, float start, float end) {
    if (start < end) {
        return angle > start && angle <= end;
    } else {
        // Range wraps around 360 degrees
        return angle > start || angle <= end;
    }
}

// Helper function to check if angle is in range (inclusive start, exclusive end)
static int angle_in_range_inclusive_exclusive(float angle, float start, float end) {
    if (start < end) {
        return angle >= start && angle < end;
    } else {
        // Range wraps around 360 degrees
        return angle >= start || angle < end;
    }
}

// Helper function to check if angle is in range (exclusive both ends)
static int angle_in_range_exclusive_exclusive(float angle, float start, float end) {
    if (start < end) {
        return angle > start && angle < end;
    } else {
        // Range wraps around 360 degrees
        return angle > start || angle < end;
    }
}

// Helper function to check if angle is in union of two ranges
static int angle_in_union_ranges(float angle, float range1_start, float range1_end, 
                                 float range2_start, float range2_end, 
                                 int range1_start_inclusive, int range1_end_inclusive,
                                 int range2_start_inclusive, int range2_end_inclusive) {
    int in_range1 = 0, in_range2 = 0;
    
    // Check first range
    if (range1_start_inclusive && range1_end_inclusive) {
        in_range1 = (angle >= range1_start && angle <= range1_end);
    } else if (range1_start_inclusive && !range1_end_inclusive) {
        in_range1 = (angle >= range1_start && angle < range1_end);
    } else if (!range1_start_inclusive && range1_end_inclusive) {
        in_range1 = (angle > range1_start && angle <= range1_end);
    } else {
        in_range1 = (angle > range1_start && angle < range1_end);
    }
    
    // Check second range
    if (range2_start_inclusive && range2_end_inclusive) {
        in_range2 = (angle >= range2_start && angle <= range2_end);
    } else if (range2_start_inclusive && !range2_end_inclusive) {
        in_range2 = (angle >= range2_start && angle < range2_end);
    } else if (!range2_start_inclusive && range2_end_inclusive) {
        in_range2 = (angle > range2_start && angle <= range2_end);
    } else {
        in_range2 = (angle > range2_start && angle < range2_end);
    }
    
    return in_range1 || in_range2;
}

// Function to classify boundary events based on geometric rules
static bcd_event_type_t classify_boundary_event(bcd_event_t event, int is_leftmost, int is_rightmost) {
    if (is_leftmost) {
        return BOUND_INIT;
    }
    if (is_rightmost) {
        return BOUND_DEINIT;
    }
    
    // For other boundary events, we need to implement the geometric analysis
    // This would require additional context about the boundary polygon geometry
    // For now, return a default classification - this should be enhanced based on
    // specific geometric rules for boundary events
    float floor_angle = compute_edge_angle(event.floor_edge);
    float ceiling_angle = compute_edge_angle(event.ceiling_edge);
    
    // Placeholder logic - should be refined based on boundary-specific rules
    if (floor_angle > 90.0f && floor_angle <= 180.0f) {
        return BOUND_IN;
    } else if (ceiling_angle >= 0.0f && ceiling_angle < 90.0f) {
        return BOUND_OUT;
    } else {
        return BOUND_SIDE_IN; // Default case
    }
}

// Function to classify obstacle events based on geometric rules from requirements
static bcd_event_type_t classify_obstacle_event(bcd_event_t event) {
    float floor_angle = compute_edge_angle(event.floor_edge);
    float ceiling_angle = compute_edge_angle(event.ceiling_edge);
    
    // OUT event classification (Requirement 2.7) - Check OUT before SIDE_IN for priority
    // (ceiling_edge_vector_angle ∈ [0, 90) AND floor_edge_vector_angle ∈ (90, (90 + ceiling_edge_vector_angle))) OR
    // (ceiling_edge_vector_angle ∈ (270, 360] AND floor_edge_vector_angle ∈ (90, (ceiling_edge_vector_angle - 180)))
    if (angle_in_range_inclusive_exclusive(ceiling_angle, 0.0f, 90.0f)) {
        float floor_range_end = 90.0f + ceiling_angle;
        if (angle_in_range_exclusive_exclusive(floor_angle, 90.0f, floor_range_end)) {
            return OUT;
        }
    } else if (angle_in_range_exclusive_inclusive(ceiling_angle, 270.0f, 360.0f)) {
        float floor_range_end = ceiling_angle - 180.0f;
        if (angle_in_range_exclusive_exclusive(floor_angle, 90.0f, floor_range_end)) {
            return OUT;
        }
    }

    // IN event classification (Requirement 2.5)
    // (floor_edge_vector_angle ∈ (90, 180] AND ceiling_edge_vector_angle ∈ (270, (floor_edge_vector_angle + 180))) OR
    // (floor_edge_vector_angle ∈ (180, 270) AND ceiling_edge_vector_angle ∈ (270, 360] ∪ [0, (floor_edge_vector_angle - 180)))
    if (angle_in_range_exclusive_inclusive(floor_angle, 90.0f, 180.0f)) {
        float ceiling_range_end = floor_angle + 180.0f;
        if (ceiling_range_end >= 360.0f) ceiling_range_end -= 360.0f;
        if (angle_in_range_exclusive_exclusive(ceiling_angle, 270.0f, ceiling_range_end)) {
            return IN;
        }
    } else if (angle_in_range_exclusive_exclusive(floor_angle, 180.0f, 270.0f)) {
        float ceiling_range_start = floor_angle - 180.0f;
        if (ceiling_range_start < 0.0f) ceiling_range_start += 360.0f;
        if (angle_in_union_ranges(ceiling_angle, 270.0f, 360.0f, 0.0f, ceiling_range_start, 
                                  1, 1, 1, 1)) {
            return IN;
        }
    }
    
    // SIDE_IN event classification (Requirement 2.6)
    // (floor_edge_vector_angle ∈ (90, 180] AND ceiling_edge_vector_angle ∈ [0, 90) ∪ ((floor_edge_vector_angle + 180), 360]) OR
    // (floor_edge_vector_angle ∈ (180, 270) AND ceiling_edge_vector_angle ∈ ((floor_edge_vector_angle - 180), 270))
    if (angle_in_range_exclusive_inclusive(floor_angle, 90.0f, 180.0f)) {
        float ceiling_range2_start = floor_angle + 180.0f;
        if (ceiling_range2_start >= 360.0f) ceiling_range2_start -= 360.0f;
        if (angle_in_union_ranges(ceiling_angle, 0.0f, 90.0f, ceiling_range2_start, 360.0f,
                                  1, 0, 0, 1)) {
            return SIDE_IN;
        }
    } else if (angle_in_range_exclusive_exclusive(floor_angle, 180.0f, 270.0f)) {
        float ceiling_range_start = floor_angle - 180.0f;
        if (ceiling_range_start < 0.0f) ceiling_range_start += 360.0f;
        if (angle_in_range_exclusive_exclusive(ceiling_angle, ceiling_range_start, 270.0f)) {
            return SIDE_IN;
        }
    }
    
    // SIDE_OUT event classification (Requirement 2.8)
    // (ceiling_edge_vector_angle ∈ [0, 90) AND floor_edge_vector_angle ∈ ((ceiling_edge_vector_angle + 180), 270)) OR
    // (ceiling_edge_vector_angle ∈ (270, 360] AND floor_edge_vector_angle ∈ ((ceiling_edge_vector_angle - 180), 270))
    if (angle_in_range_inclusive_exclusive(ceiling_angle, 0.0f, 90.0f)) {
        float floor_range_start = ceiling_angle + 180.0f;
        if (angle_in_range_exclusive_exclusive(floor_angle, floor_range_start, 270.0f)) {
            return SIDE_OUT;
        }
    } else if (angle_in_range_exclusive_inclusive(ceiling_angle, 270.0f, 360.0f)) {
        float floor_range_start = ceiling_angle - 180.0f;
        if (angle_in_range_exclusive_exclusive(floor_angle, floor_range_start, 270.0f)) {
            return SIDE_OUT;
        }
    }
    
    // If none of the above conditions are met, check for FLOOR/CEILING events
    // FLOOR events: between OUT and IN (traced along polygon vertices in winding order)
    // CEILING events: between IN and OUT (traced along polygon vertices in winding order)
    // For now, we'll use a simple heuristic based on which edge is defined
    // This should be enhanced with proper polygon traversal context
    
    // Default fallback - this should not happen with proper input
    return FLOOR; // or CEILING based on context
}

// Forward declarations for static helper functions
static int populate_event_geometry(bcd_event_t *event, polygon_t *polygon, int vertex_index);

// Helper function to validate input environment structure integrity
static int validate_input_environment(input_environment_t *env) {
    if (env == NULL) {
        printf("Error: input_environment_t is NULL\n");
        return BCD_ERROR_INVALID_INPUT;
    }
    
    // Validate boundary polygon
    if (env->boundary.vertices == NULL && env->boundary.vertex_count > 0) {
        printf("Error: Boundary polygon has vertex_count=%u but vertices is NULL\n", env->boundary.vertex_count);
        return BCD_ERROR_INVALID_INPUT;
    }
    
    if (env->boundary.vertex_count == 0 && env->boundary.vertices != NULL) {
        printf("Error: Boundary polygon has vertices but vertex_count=0\n");
        return BCD_ERROR_INVALID_INPUT;
    }
    
    // Validate obstacle polygons array
    if (env->obstacles == NULL && env->obstacle_count > 0) {
        printf("Error: Environment has obstacle_count=%u but obstacles is NULL\n", env->obstacle_count);
        return BCD_ERROR_INVALID_INPUT;
    }
    
    if (env->obstacle_count == 0 && env->obstacles != NULL) {
        printf("Error: Environment has obstacles array but obstacle_count=0\n");
        return BCD_ERROR_INVALID_INPUT;
    }
    
    // Validate individual obstacle polygons
    if (env->obstacles != NULL && env->obstacle_count > 0) {
        for (uint32_t i = 0; i < env->obstacle_count; i++) {
            polygon_t *obstacle = &env->obstacles[i];
            
            if (obstacle->vertices == NULL && obstacle->vertex_count > 0) {
                printf("Error: Obstacle polygon %u has vertex_count=%u but vertices is NULL\n", 
                       i, obstacle->vertex_count);
                return BCD_ERROR_INVALID_INPUT;
            }
            
            if (obstacle->vertex_count == 0 && obstacle->vertices != NULL) {
                printf("Error: Obstacle polygon %u has vertices but vertex_count=0\n", i);
                return BCD_ERROR_INVALID_INPUT;
            }
        }
    }
    
    return BCD_SUCCESS;
}

// Helper function to validate polygon structure and requirements
static int validate_polygon(polygon_t *polygon, const char *polygon_name) {
    if (polygon == NULL) {
        printf("Error: %s polygon is NULL\n", polygon_name);
        return BCD_ERROR_INVALID_INPUT;
    }
    
    // Check for empty polygon
    if (polygon->vertex_count == 0) {
        printf("Error: %s polygon has no vertices (vertex_count=0)\n", polygon_name);
        return BCD_ERROR_POLYGON_VALIDATION;
    }
    
    if (polygon->vertices == NULL) {
        printf("Error: %s polygon vertices array is NULL\n", polygon_name);
        return BCD_ERROR_POLYGON_VALIDATION;
    }
    
    // Check if edges are built
    if (polygon->edges == NULL) {
        printf("Error: %s polygon edges not built (edges array is NULL)\n", polygon_name);
        return BCD_ERROR_POLYGON_VALIDATION;
    }
    
    if (polygon->edge_count == 0) {
        printf("Error: %s polygon has no edges (edge_count=0)\n", polygon_name);
        return BCD_ERROR_POLYGON_VALIDATION;
    }
    
    // Validate edge count matches vertex count
    if (polygon->edge_count != polygon->vertex_count) {
        printf("Error: %s polygon edge_count=%u does not match vertex_count=%u\n", 
               polygon_name, polygon->edge_count, polygon->vertex_count);
        return BCD_ERROR_POLYGON_VALIDATION;
    }
    
    // Check polygon winding
    if (polygon->winding == POLYGON_WINDING_UNKNOWN) {
        printf("Error: %s polygon has unknown winding direction\n", polygon_name);
        return BCD_ERROR_WINDING_VALIDATION;
    }
    
    // Validate minimum vertex count for a polygon (at least 3 vertices)
    if (polygon->vertex_count < 3) {
        printf("Error: %s polygon has insufficient vertices (count=%u, minimum=3)\n", 
               polygon_name, polygon->vertex_count);
        return BCD_ERROR_POLYGON_VALIDATION;
    }
    
    return BCD_SUCCESS;
}

// Helper function to extract events from input environment
static int extract_events_from_environment(input_environment_t *env, bcd_event_t *events, int *event_count) {
    if (env == NULL || events == NULL || event_count == NULL) {
        printf("Error: NULL parameter passed to extract_events_from_environment\n");
        return BCD_ERROR_INVALID_INPUT;
    }
    
    int total_events = 0;
    
    // Extract events from boundary polygon
    if (env->boundary.vertices != NULL && env->boundary.vertex_count > 0) {
        int result = validate_polygon(&env->boundary, "Boundary");
        if (result != BCD_SUCCESS) {
            return result;
        }
        
        for (uint32_t i = 0; i < env->boundary.vertex_count; i++) {
            events[total_events].polygon_type = BOUNDARY;
            events[total_events].polygon_vertex = env->boundary.vertices[i];
            
            // Populate geometry using helper function
            result = populate_event_geometry(&events[total_events], &env->boundary, i);
            if (result != BCD_SUCCESS) {
                printf("Error: Failed to populate geometry for boundary vertex %u\n", i);
                return result;
            }
            
            total_events++;
        }
    }
    
    // Extract events from obstacle polygons
    if (env->obstacles != NULL && env->obstacle_count > 0) {
        for (uint32_t obs_idx = 0; obs_idx < env->obstacle_count; obs_idx++) {
            polygon_t *obstacle = &env->obstacles[obs_idx];
            
            if (obstacle->vertices != NULL && obstacle->vertex_count > 0) {
                char polygon_name[64];
                snprintf(polygon_name, sizeof(polygon_name), "Obstacle %u", obs_idx);
                
                int result = validate_polygon(obstacle, polygon_name);
                if (result != BCD_SUCCESS) {
                    return result;
                }
                
                for (uint32_t i = 0; i < obstacle->vertex_count; i++) {
                    events[total_events].polygon_type = OBSTACLE;
                    events[total_events].polygon_vertex = obstacle->vertices[i];
                    
                    // Populate geometry using helper function
                    result = populate_event_geometry(&events[total_events], obstacle, i);
                    if (result != BCD_SUCCESS) {
                        printf("Error: Failed to populate geometry for obstacle %u vertex %u\n", obs_idx, i);
                        return result;
                    }
                    
                    total_events++;
                }
            }
        }
    }
    
    *event_count = total_events;
    return BCD_SUCCESS;
}

// Helper function to populate event geometry (floor_edge and ceiling_edge)
static int populate_event_geometry(bcd_event_t *event, polygon_t *polygon, int vertex_index) {
    if (event == NULL || polygon == NULL) {
        printf("Error: NULL parameter passed to populate_event_geometry\n");
        return BCD_ERROR_INVALID_INPUT;
    }
    
    if (vertex_index < 0 || vertex_index >= (int)polygon->vertex_count) {
        printf("Error: Invalid vertex_index=%d for polygon with vertex_count=%u\n", 
               vertex_index, polygon->vertex_count);
        return BCD_ERROR_INVALID_INPUT;
    }
    
    if (polygon->edges == NULL || polygon->edge_count == 0) {
        printf("Error: Polygon edges not built (edges=NULL or edge_count=0)\n");
        return BCD_ERROR_POLYGON_VALIDATION;
    }
    
    if (polygon->edge_count != polygon->vertex_count) {
        printf("Error: Edge count mismatch (edge_count=%u, vertex_count=%u)\n", 
               polygon->edge_count, polygon->vertex_count);
        return BCD_ERROR_POLYGON_VALIDATION;
    }
    
    // Floor edge: edge emanating from the event vertex (edges[vertex_index])
    // This is the edge from vertex[vertex_index] to vertex[(vertex_index + 1) % vertex_count]
    event->floor_edge = polygon->edges[vertex_index];
    
    // Ceiling edge: edge terminating at the event vertex (edges[(vertex_index - 1 + vertex_count) % vertex_count])
    // This is the edge from vertex[(vertex_index - 1 + vertex_count) % vertex_count] to vertex[vertex_index]
    int ceiling_edge_index = (vertex_index - 1 + (int)polygon->vertex_count) % (int)polygon->vertex_count;
    event->ceiling_edge = polygon->edges[ceiling_edge_index];
    
    return BCD_SUCCESS;
}

// Main function to determine BCD event type
int find_bcd_event_type(bcd_event_t *bcd_event) {
    // Validate input
    if (bcd_event == NULL) {
        printf("Error: bcd_event is NULL in find_bcd_event_type\n");
        return BCD_ERROR_INVALID_INPUT;
    }
    
    if (bcd_event->polygon_type != BOUNDARY && bcd_event->polygon_type != OBSTACLE) {
        printf("Error: Invalid polygon_type=%d in bcd_event\n", bcd_event->polygon_type);
        return BCD_ERROR_INVALID_INPUT;
    }
    
    bcd_event_type_t event_type;
    
    if (bcd_event->polygon_type == BOUNDARY) {
        // For boundary events, we need additional context to determine if this is leftmost/rightmost
        // This is a simplified implementation - in practice, this would require access to the
        // full polygon data to determine min/max x coordinates
        int is_leftmost = 0; // Would need to be determined from polygon context
        int is_rightmost = 0; // Would need to be determined from polygon context
        
        event_type = classify_boundary_event(*bcd_event, is_leftmost, is_rightmost);
    } else {
        // OBSTACLE polygon
        event_type = classify_obstacle_event(*bcd_event);
    }
    
    // Update the event structure with the determined type
    bcd_event->bcd_event_type = event_type;
    
    return BCD_SUCCESS;
}

// Helper function to compare events for sorting by x-coordinate
static int compare_events_by_x(const void *a, const void *b) {
    const bcd_event_t *event_a = (const bcd_event_t *)a;
    const bcd_event_t *event_b = (const bcd_event_t *)b;
    
    if (event_a->polygon_vertex.x < event_b->polygon_vertex.x) {
        return -1;
    } else if (event_a->polygon_vertex.x > event_b->polygon_vertex.x) {
        return 1;
    } else {
        // For stable sorting when x-coordinates are equal, maintain original order
        return 0;
    }
}

// Helper function to sort events by x-coordinate
static int sort_events_by_x_coordinate(bcd_event_t *events, int event_count) {
    if (events == NULL) {
        printf("Error: events array is NULL in sort_events_by_x_coordinate\n");
        return BCD_ERROR_INVALID_INPUT;
    }
    
    if (event_count <= 0) {
        printf("Error: Invalid event_count=%d in sort_events_by_x_coordinate\n", event_count);
        return BCD_ERROR_INVALID_INPUT;
    }
    
    // Use qsort for stable sorting by x-coordinate
    qsort(events, (size_t)event_count, sizeof(bcd_event_t), compare_events_by_x);
    
    return BCD_SUCCESS;
}

// Helper function to validate event constraints
static int validate_event_constraints(bcd_event_t *events, int event_count) {
    if (events == NULL) {
        if (event_count > 0) {
            printf("Error: events array is NULL but event_count=%d\n", event_count);
            return BCD_ERROR_INVALID_INPUT;
        }
        return BCD_SUCCESS; // No events to validate
    }
    
    if (event_count <= 0) {
        return BCD_SUCCESS; // No events to validate
    }
    
    // Check for duplicate IN/OUT events at same x-coordinate
    for (int i = 0; i < event_count - 1; i++) {
        for (int j = i + 1; j < event_count; j++) {
            // Check if events have the same x-coordinate (within tolerance)
            if (fabs(events[i].polygon_vertex.x - events[j].polygon_vertex.x) < 0.0001f) {
                // Check if both are IN events or both are OUT events
                if ((events[i].bcd_event_type == IN && events[j].bcd_event_type == IN) ||
                    (events[i].bcd_event_type == OUT && events[j].bcd_event_type == OUT)) {
                    printf("Error: Duplicate %s events at x-coordinate %.6f (events %d and %d)\n",
                           events[i].bcd_event_type == IN ? "IN" : "OUT",
                           events[i].polygon_vertex.x, i, j);
                    return BCD_ERROR_CONSTRAINT_VIOLATION;
                }
            }
        }
    }
    
    return BCD_SUCCESS;
}

// Helper function to find leftmost and rightmost vertices in boundary polygon
static void find_boundary_extremes(polygon_t *boundary, int *leftmost_idx, int *rightmost_idx) {
    if (boundary == NULL || boundary->vertices == NULL || boundary->vertex_count == 0) {
        *leftmost_idx = -1;
        *rightmost_idx = -1;
        return;
    }
    
    *leftmost_idx = 0;
    *rightmost_idx = 0;
    
    for (uint32_t i = 1; i < boundary->vertex_count; i++) {
        if (boundary->vertices[i].x < boundary->vertices[*leftmost_idx].x) {
            *leftmost_idx = (int)i;
        }
        if (boundary->vertices[i].x > boundary->vertices[*rightmost_idx].x) {
            *rightmost_idx = (int)i;
        }
    }
}

// Enhanced find_bcd_event_type function with boundary extreme detection
static int classify_event_with_context(bcd_event_t *event, polygon_t *boundary, int leftmost_idx, int rightmost_idx, int vertex_idx) {
    if (event == NULL) {
        printf("Error: event is NULL in classify_event_with_context\n");
        return BCD_ERROR_INVALID_INPUT;
    }
    
    bcd_event_type_t event_type;
    
    if (event->polygon_type == BOUNDARY) {
        // Check if this is the leftmost or rightmost boundary vertex
        int is_leftmost = (vertex_idx == leftmost_idx);
        int is_rightmost = (vertex_idx == rightmost_idx);
        
        event_type = classify_boundary_event(*event, is_leftmost, is_rightmost);
    } else if (event->polygon_type == OBSTACLE) {
        // OBSTACLE polygon
        event_type = classify_obstacle_event(*event);
    } else {
        printf("Error: Invalid polygon_type=%d in event\n", event->polygon_type);
        return BCD_ERROR_INVALID_INPUT;
    }
    
    // Update the event structure with the determined type
    event->bcd_event_type = event_type;
    
    return BCD_SUCCESS;
}

// Main function to build BCD event list from input environment
int build_bcd_event_list(input_environment_t *env, bcd_event_t **event_list, int *event_count) {
    // Input validation
    if (env == NULL || event_list == NULL || event_count == NULL) {
        printf("Error: Invalid input parameters to build_bcd_event_list (env=%p, event_list=%p, event_count=%p)\n",
               (void*)env, (void*)event_list, (void*)event_count);
        return BCD_ERROR_INVALID_INPUT;
    }
    
    // Initialize output parameters to safe defaults
    *event_list = NULL;
    *event_count = 0;
    
    // Comprehensive input environment validation
    int result = validate_input_environment(env);
    if (result != BCD_SUCCESS) {
        printf("Error: Input environment validation failed with code %d\n", result);
        return result;
    }
    
    // Calculate total number of events needed
    int total_events = 0;
    
    // Count boundary vertices
    if (env->boundary.vertices != NULL && env->boundary.vertex_count > 0) {
        result = validate_polygon(&env->boundary, "Boundary");
        if (result != BCD_SUCCESS) {
            printf("Error: Boundary polygon validation failed with code %d\n", result);
            return result;
        }
        total_events += (int)env->boundary.vertex_count;
    }
    
    // Count obstacle vertices
    if (env->obstacles != NULL && env->obstacle_count > 0) {
        for (uint32_t i = 0; i < env->obstacle_count; i++) {
            if (env->obstacles[i].vertices != NULL && env->obstacles[i].vertex_count > 0) {
                char polygon_name[64];
                snprintf(polygon_name, sizeof(polygon_name), "Obstacle %u", i);
                
                result = validate_polygon(&env->obstacles[i], polygon_name);
                if (result != BCD_SUCCESS) {
                    printf("Error: Obstacle %u polygon validation failed with code %d\n", i, result);
                    return result;
                }
                total_events += (int)env->obstacles[i].vertex_count;
            }
        }
    }
    
    // Handle empty environment
    if (total_events == 0) {
        printf("Warning: No events to generate (empty environment)\n");
        return BCD_SUCCESS; // Success with empty event list
    }
    
    printf("Info: Allocating memory for %d events\n", total_events);
    
    // Allocate memory for events
    bcd_event_t *events = malloc((size_t)total_events * sizeof(bcd_event_t));
    if (events == NULL) {
        printf("Error: Failed to allocate memory for %d events (%zu bytes)\n", 
               total_events, (size_t)total_events * sizeof(bcd_event_t));
        return BCD_ERROR_MEMORY_ALLOCATION;
    }
    
    // Initialize allocated memory to prevent undefined behavior
    memset(events, 0, (size_t)total_events * sizeof(bcd_event_t));
    
    // Extract events from environment
    int extracted_count = 0;
    result = extract_events_from_environment(env, events, &extracted_count);
    if (result != BCD_SUCCESS) {
        printf("Error: Event extraction failed with code %d\n", result);
        free(events);
        return result;
    }
    
    // Verify extracted count matches expected count
    if (extracted_count != total_events) {
        printf("Error: Event count mismatch (expected=%d, extracted=%d)\n", total_events, extracted_count);
        free(events);
        return BCD_ERROR_INVALID_INPUT;
    }
    
    // Find boundary extremes for proper event classification
    int leftmost_idx = -1, rightmost_idx = -1;
    find_boundary_extremes(&env->boundary, &leftmost_idx, &rightmost_idx);
    
    printf("Info: Classifying %d events\n", extracted_count);
    
    // Classify all events with proper context
    for (int i = 0; i < extracted_count; i++) {
        int vertex_idx = -1;
        polygon_t *source_polygon = NULL;
        
        // Determine source polygon and vertex index for boundary events
        if (events[i].polygon_type == BOUNDARY) {
            source_polygon = &env->boundary;
            // Find vertex index by comparing coordinates
            for (uint32_t j = 0; j < env->boundary.vertex_count; j++) {
                if (fabs(events[i].polygon_vertex.x - env->boundary.vertices[j].x) < 0.0001f &&
                    fabs(events[i].polygon_vertex.y - env->boundary.vertices[j].y) < 0.0001f) {
                    vertex_idx = (int)j;
                    break;
                }
            }
            
            if (vertex_idx == -1) {
                printf("Error: Could not find vertex index for boundary event %d at (%.6f, %.6f)\n",
                       i, events[i].polygon_vertex.x, events[i].polygon_vertex.y);
                free(events);
                return BCD_ERROR_EVENT_CLASSIFICATION;
            }
        }
        
        result = classify_event_with_context(&events[i], source_polygon, leftmost_idx, rightmost_idx, vertex_idx);
        if (result != BCD_SUCCESS) {
            printf("Error: Failed to classify event %d with code %d\n", i, result);
            free(events);
            return BCD_ERROR_EVENT_CLASSIFICATION;
        }
    }
    
    printf("Info: Sorting %d events by x-coordinate\n", extracted_count);
    
    // Sort events by x-coordinate
    result = sort_events_by_x_coordinate(events, extracted_count);
    if (result != BCD_SUCCESS) {
        printf("Error: Failed to sort events with code %d\n", result);
        free(events);
        return result;
    }
    
    printf("Info: Validating event constraints\n");
    
    // Validate event constraints
    result = validate_event_constraints(events, extracted_count);
    if (result != BCD_SUCCESS) {
        printf("Error: Event constraint validation failed with code %d\n", result);
        free(events);
        return result;
    }
    
    // Set output parameters
    *event_list = events;
    *event_count = extracted_count;
    
    printf("Info: Successfully generated %d events\n", extracted_count);
    return BCD_SUCCESS;
}

// Test wrapper functions to expose static functions for unit testing
#ifdef BCD_TESTING
int test_validate_input_environment(input_environment_t *env) {
    return validate_input_environment(env);
}

int test_validate_polygon(polygon_t *polygon, const char *polygon_name) {
    return validate_polygon(polygon, polygon_name);
}

int test_extract_events_from_environment(input_environment_t *env, bcd_event_t *events, int *event_count) {
    return extract_events_from_environment(env, events, event_count);
}

int test_populate_event_geometry(bcd_event_t *event, polygon_t *polygon, int vertex_index) {
    return populate_event_geometry(event, polygon, vertex_index);
}

int test_sort_events_by_x_coordinate(bcd_event_t *events, int event_count) {
    return sort_events_by_x_coordinate(events, event_count);
}

int test_validate_event_constraints(bcd_event_t *events, int event_count) {
    return validate_event_constraints(events, event_count);
}

int test_find_boundary_extremes(polygon_t *boundary, int *leftmost_idx, int *rightmost_idx) {
    find_boundary_extremes(boundary, leftmost_idx, rightmost_idx);
    return BCD_SUCCESS;
}

int test_classify_event_with_context(bcd_event_t *event, polygon_t *boundary, int leftmost_idx, int rightmost_idx, int vertex_idx) {
    return classify_event_with_context(event, boundary, leftmost_idx, rightmost_idx, vertex_idx);
}
#endif
