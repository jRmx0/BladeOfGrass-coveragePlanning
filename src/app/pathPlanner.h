#ifndef PATH_PLANNER_H
#define PATH_PLANNER_H

#include "../../dependencies/cJSON/cJSON.h"

// Data structures for path planning
typedef struct {
    double x;
    double y;
} Point2D;

typedef struct {
    Point2D* points;
    int count;
    int capacity;
} PointArray;

typedef struct {
    double mowerWidth;
    double pathOverlap;
} PathSettings;

typedef struct {
    PointArray boundary;
    PathSettings settings;
    char* timestamp;
} PathRequest;

typedef struct {
    PointArray decomposition;
    PointArray path;
    char* req_timestamp;
} PathResponse;

// Function declarations
PathRequest* parse_path_request(const char* json_string);
PathResponse* calculate_coverage_path(const PathRequest* request);
char* generate_path_response_json(const PathResponse* response);

// Helper functions
PointArray* create_point_array(int initial_capacity);
void add_point(PointArray* array, double x, double y);
void free_point_array(PointArray* array);
void free_path_request(PathRequest* request);
void free_path_response(PathResponse* response);

// Path planning algorithms
void decompose_polygon(const PointArray* boundary, PointArray* decomposition);
void generate_boustrophedon_path(const PointArray* boundary, const PathSettings* settings, PointArray* path);
double calculate_polygon_area(const PointArray* boundary);
void get_bounding_box(const PointArray* boundary, Point2D* min_point, Point2D* max_point);
int get_horizontal_intersections(double y, const PointArray* boundary, double* intersections, int max_intersections);
int point_in_polygon(const Point2D* point, const PointArray* boundary);

#endif // PATH_PLANNER_H