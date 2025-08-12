# Requirements Document

## Introduction

The BCD (Boustrophedon Cellular Decomposition) Event List Generation feature is a critical component of the coverage path planning algorithm. This feature processes input_environment_t data (containing boundary and obstacle polygons) to create an ordered list of bcd_event_t structures based on vertex coordinates and geometric analysis. The event list serves as the foundation for the cellular decomposition process, where events represent significant geometric points that determine how the environment is divided into cells for coverage planning. This feature integrates with the existing coverage_path_planning_process pipeline and feeds into the broader BCD algorithm.

## Requirements

### Requirement 1

**User Story:** As a coverage path planning system, I want to generate an ordered event list from input_environment_t data, so that the BCD algorithm can properly decompose the environment into coverage cells.

#### Acceptance Criteria

1. WHEN the system receives input_environment_t containing boundary and obstacles THEN the system SHALL extract all vertices from the boundary polygon and all obstacle polygons
2. WHEN processing vertices THEN the system SHALL create bcd_event_t structures with polygon_type, polygon_vertex, bcd_event_type, floor_edge, and ceiling_edge information
3. WHEN creating the event list THEN the system SHALL sort events by x-coordinate in ascending order
4. WHEN two events have the same x-coordinate THEN the system SHALL ensure no two IN events nor two OUT events share the same x-coordinate
5. WHEN processing the boundary polygon (POLYGON_WINDING_CW) THEN the system SHALL identify boundary-specific event types
6. WHEN processing obstacle polygons (POLYGON_WINDING_CCW) THEN the system SHALL identify obstacle-specific event types
7. WHEN the event list is complete THEN the system SHALL return it via the build_bcd_event_list function interface

### Requirement 2

**User Story:** As a BCD algorithm, I want to identify different event types with precise geometric rules, so that I can correctly classify boundary and obstacle vertices for cellular decomposition.

#### Acceptance Criteria

1. WHEN processing the boundary polygon THEN the system SHALL support boundary-specific event types: BOUND_IN, BOUND_SIDE_IN, BOUND_INIT, BOUND_OUT, BOUND_SIDE_OUT, BOUND_DEINIT
2. WHEN processing the leftmost boundary vertex THEN the system SHALL classify it as BOUND_INIT event type
3. WHEN processing the rightmost boundary vertex THEN the system SHALL classify it as BOUND_DEINIT event type
4. WHEN processing obstacle polygons THEN the system SHALL support obstacle event types: IN, SIDE_IN, OUT, SIDE_OUT, FLOOR, CEILING
5. WHEN determining IN event type THEN the system SHALL verify that (floor_edge_vector_angle ∈ (90, 180] AND ceiling_edge_vector_angle ∈ (270, (floor_edge_vector_angle + 180))) OR (floor_edge_vector_angle ∈ (180, 270) AND ceiling_edge_vector_angle ∈ (270, 360] ∪ [0, (floor_edge_vector_angle - 180)))
6. WHEN determining SIDE_IN event type THEN the system SHALL verify that (floor_edge_vector_angle ∈ (90, 180] AND ceiling_edge_vector_angle ∈ [0, 90) ∪ ((floor_edge_vector_angle + 180), 360]) OR (floor_edge_vector_angle ∈ (180, 270) AND ceiling_edge_vector_angle ∈ ((floor_edge_vector_angle - 180), 270))
7. WHEN determining OUT event type THEN the system SHALL verify that (ceiling_edge_vector_angle ∈ [0, 90) AND ceiling_edge_vector_angle ∈ (90, (90 + ceiling_edge_vector_angle))) OR (ceiling_edge_vector_angle ∈ (270, 360] AND ceiling_edge_vector_angle ∈ (90, (ceiling_edge_vector_angle - 180)))
8. WHEN determining SIDE_OUT event type THEN the system SHALL verify that (ceiling_edge_vector_angle ∈ [0, 90) AND ceiling_edge_vector_angle ∈ ((ceiling_edge_vector_angle + 180), 270)) OR (ceiling_edge_vector_angle ∈ (270, 360] AND ceiling_edge_vector_angle ∈ ((ceiling_edge_vector_angle - 180), 270))
9. WHEN determining FLOOR event type THEN the system SHALL identify events between OUT and IN when traced along polygon vertices in winding order, with only floor_edge defined
10. WHEN determining CEILING event type THEN the system SHALL identify events between IN and OUT when traced along polygon vertices in winding order, with only ceiling_edge defined

### Requirement 3

**User Story:** As the coverage path planning system, I want each event to contain complete geometric information, so that the BCD algorithm can perform accurate edge angle calculations and event classification.

#### Acceptance Criteria

1. WHEN creating each event THEN the system SHALL populate polygon_type (BOUNDARY or OBSTACLE), polygon_vertex, bcd_event_type, floor_edge, and ceiling_edge fields
2. WHEN determining floor_edge THEN the system SHALL identify the polygon_edge_t emanating from the event vertex using the polygon's edge array
3. WHEN determining ceiling_edge THEN the system SHALL identify the polygon_edge_t terminating at the event vertex using the polygon's edge array
4. WHEN computing edge angles THEN the system SHALL use the compute_edge_angle function to calculate vector angles for both floor and ceiling edges
5. WHEN processing FLOOR events THEN the system SHALL populate only the floor_edge field and leave ceiling_edge undefined
6. WHEN processing CEILING events THEN the system SHALL populate only the ceiling_edge field and leave floor_edge undefined
7. WHEN creating events THEN the system SHALL preserve the original float precision from polygon_vertex_t coordinates
8. WHEN accessing polygon edges THEN the system SHALL use the pre-built edges array from polygon_build_edges function

### Requirement 4

**User Story:** As the coverage path planning system, I want the event list to maintain geometric integrity, so that the subsequent cellular decomposition produces valid coverage cells.

#### Acceptance Criteria

1. WHEN sorting events THEN the system SHALL maintain stable ordering for events with identical x-coordinates
2. WHEN the event list is complete THEN the system SHALL contain exactly the same number of events as total vertices in all input polygons
3. WHEN validating the event list THEN the system SHALL ensure each polygon contributes at least one event
4. WHEN processing multiple polygons THEN the system SHALL ensure no two IN events nor two OUT events have the same x-coordinate
5. IF input polygons are malformed or empty THEN the system SHALL return an appropriate error code

### Requirement 5

**User Story:** As a developer integrating with the BCD system, I want clear error handling and validation, so that I can diagnose issues with input data or processing failures.

#### Acceptance Criteria

1. WHEN input_environment_t is NULL or contains no polygons THEN the system SHALL return a specific error code
2. WHEN polygon vertices are NULL or vertex_count is zero THEN the system SHALL return a validation error code
3. WHEN polygon edges are not built (edges array is NULL) THEN the system SHALL return an edge preparation error code
4. WHEN memory allocation fails during event list creation THEN the system SHALL return a memory error code
5. WHEN event list generation succeeds THEN the system SHALL return a success code (0)
6. WHEN errors occur THEN the system SHALL use printf for diagnostic information consistent with existing coverage_path_planning_process error handling
7. WHEN compute_edge_angle function fails THEN the system SHALL return an appropriate error code
8. WHEN find_bcd_event_type function fails THEN the system SHALL return a classification error code
9. WHEN polygon winding is POLYGON_WINDING_UNKNOWN THEN the system SHALL return a winding validation error code