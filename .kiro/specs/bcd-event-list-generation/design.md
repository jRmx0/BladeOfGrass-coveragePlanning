# Design Document

## Overview

The BCD Event List Generation system transforms polygon-based environment data into a sorted list of geometric events that drive the cellular decomposition algorithm. The system processes boundary and obstacle polygons from `input_environment_t`, analyzes vertex geometry to determine event types, and produces a sorted `bcd_event_t` array.

The design leverages the existing polygon data structures and integrates with the coverage path planning pipeline. Key geometric concepts include edge vector analysis, event type classification based on angular relationships, and sweep line algorithm preparation.

## Architecture

### High-Level Flow
```
input_environment_t → Event Extraction → Geometric Analysis → Event Classification → Sorted Event List
```

### Core Components
1. **Event Extractor**: Iterates through polygons and extracts vertices as potential events
2. **Geometric Analyzer**: Computes edge vectors and angles for each vertex
3. **Event Classifier**: Determines event type based on geometric rules and polygon context
4. **Event Sorter**: Orders events by x-coordinate for sweep line processing

### Integration Points
- **Input**: Receives `input_environment_t` from `coverage_path_planning_process`
- **Output**: Provides sorted `bcd_event_t` array to downstream BCD algorithms
- **Dependencies**: Uses `polygon_build_edges`, `compute_edge_angle`, `find_bcd_event_type`

## Components and Interfaces

### Primary Function Interface
```c
int build_bcd_event_list(bcd_event_t *bcd_event_list);
```
- **Purpose**: Main entry point for event list generation
- **Input**: Pre-allocated array for event storage
- **Output**: Populated and sorted event list
- **Return**: Success (0) or error code

### Supporting Function Interfaces
```c
int find_bcd_event_type(bcd_event_t bcd_event);
float compute_edge_angle(polygon_edge_t poly_edge);
```
- **find_bcd_event_type**: Classifies events based on geometric analysis
- **compute_edge_angle**: Calculates edge vector angles for classification

### Internal Helper Functions (to be implemented)
```c
static int extract_events_from_environment(input_environment_t *env, bcd_event_t *events, int *event_count);
static int populate_event_geometry(bcd_event_t *event, polygon_t *polygon, int vertex_index);
static int sort_events_by_x_coordinate(bcd_event_t *events, int event_count);
static int validate_event_constraints(bcd_event_t *events, int event_count);
```

## Data Models

### Event Structure Population
Each `bcd_event_t` contains:
- **polygon_type**: BOUNDARY or OBSTACLE based on source polygon
- **polygon_vertex**: Direct copy from polygon vertices array
- **bcd_event_type**: Classified using geometric rules (BOUND_INIT, IN, OUT, etc.)
- **floor_edge**: Edge emanating from vertex (computed from polygon edges)
- **ceiling_edge**: Edge terminating at vertex (computed from polygon edges)

### Geometric Relationships
- **Floor Edge**: For vertex at index i, floor edge is edges[i] (vertex[i] → vertex[i+1])
- **Ceiling Edge**: For vertex at index i, ceiling edge is edges[i-1] (vertex[i-1] → vertex[i])
- **Edge Angles**: Computed as vector angles from edge begin to end points

### Event Type Classification Rules

#### Boundary Events (POLYGON_WINDING_CW)
- **BOUND_INIT**: Leftmost vertex (minimum x-coordinate)
- **BOUND_DEINIT**: Rightmost vertex (maximum x-coordinate)
- **BOUND_IN/BOUND_OUT**: Based on geometric analysis of adjacent edges
- **BOUND_SIDE_IN/BOUND_SIDE_OUT**: Side events for boundary polygon

#### Obstacle Events (POLYGON_WINDING_CCW)
- **IN**: Entry events based on floor/ceiling edge angle relationships
- **OUT**: Exit events based on ceiling edge angle analysis
- **SIDE_IN/SIDE_OUT**: Side events for obstacle polygons
- **FLOOR/CEILING**: Special events between IN/OUT transitions

## Error Handling

### Error Code Strategy
- **0**: Success
- **-1**: Invalid input (NULL pointers, empty data)
- **-2**: Memory allocation failure
- **-3**: Polygon validation failure (missing edges, invalid winding)
- **-4**: Geometric computation failure (edge angle calculation)
- **-5**: Event classification failure
- **-6**: Constraint violation (duplicate IN/OUT events at same x-coordinate)

### Validation Checkpoints
1. **Input Validation**: Verify input_environment_t structure integrity
2. **Polygon Validation**: Ensure edges are built and winding is known
3. **Geometric Validation**: Verify edge angle computations succeed
4. **Event Validation**: Check event type classification results
5. **Constraint Validation**: Verify no duplicate IN/OUT events at same x-coordinate

### Error Recovery
- Early termination on critical failures (memory, invalid input)
- Diagnostic logging using printf consistent with existing error patterns
- Cleanup of partially allocated resources before returning error codes

## Testing Strategy

### Unit Testing Approach
1. **Component Testing**: Test each helper function independently
2. **Integration Testing**: Test complete pipeline with known polygon sets
3. **Edge Case Testing**: Test boundary conditions and error scenarios
4. **Geometric Validation**: Verify mathematical correctness of event classification

### Test Data Sets
1. **Simple Rectangle**: Basic boundary polygon with no obstacles
2. **Rectangle with Single Obstacle**: Boundary + one rectangular obstacle
3. **Complex Environment**: Multiple obstacles with various shapes

### Validation Criteria
1. **Event Count**: Total events equals total vertices across all polygons
2. **Sorting**: Events properly ordered by x-coordinate
3. **Classification**: Event types match expected geometric analysis
4. **Constraints**: No duplicate IN/OUT events at same x-coordinate
5. **Geometric Integrity**: Floor/ceiling edges correctly identified

### Performance Considerations
- **Time Complexity**: O(n log n) for sorting n events
- **Space Complexity**: O(n) for event storage
- **Memory Management**: Efficient allocation and cleanup
- **Integration Impact**: Minimal overhead in coverage planning pipeline