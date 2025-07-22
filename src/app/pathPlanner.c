#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pathPlanner.h"

// Helper functions implementation
PointArray* create_point_array(int initial_capacity) {
    PointArray* array = malloc(sizeof(PointArray));
    if (!array) return NULL;
    
    array->points = malloc(sizeof(Point2D) * initial_capacity);
    if (!array->points) {
        free(array);
        return NULL;
    }
    
    array->count = 0;
    array->capacity = initial_capacity;
    return array;
}

void add_point(PointArray* array, double x, double y) {
    if (!array) return;
    
    // Resize if needed
    if (array->count >= array->capacity) {
        array->capacity *= 2;
        array->points = realloc(array->points, sizeof(Point2D) * array->capacity);
        if (!array->points) return;
    }
    
    array->points[array->count].x = x;
    array->points[array->count].y = y;
    array->count++;
}

void free_point_array(PointArray* array) {
    if (!array) return;
    if (array->points) free(array->points);
    free(array);
}

void free_path_request(PathRequest* request) {
    if (!request) return;
    
    //printf("  Freeing boundary points (%d points)...\n", request->boundary.count);
    // Free the boundary points directly since it's not a pointer to PointArray
    if (request->boundary.points) {
        free(request->boundary.points);
    }
    
    //printf("  Freeing timestamp...\n");
    if (request->timestamp) {
        free(request->timestamp);
    }
    
    //printf("  Freeing request structure...\n");
    free(request);
    //printf("  PathRequest freed successfully\n");
}

void free_path_response(PathResponse* response) {
    if (!response) return;
    
    //printf("  Freeing decomposition points (%d points)...\n", response->decomposition.count);
    // Free the points arrays directly since they are not pointers to PointArray
    if (response->decomposition.points) {
        free(response->decomposition.points);
    }
    
    //printf("  Freeing path points (%d points)...\n", response->path.count);
    if (response->path.points) {
        free(response->path.points);
    }
    
    //printf("  Freeing timestamp...\n");
    if (response->req_timestamp) {
        free(response->req_timestamp);
    }
    
    //printf("  Freeing response structure...\n");
    free(response);
    //printf("  PathResponse freed successfully\n");
}

// JSON parsing functions
PathRequest* parse_path_request(const char* json_string) {
    cJSON *json = cJSON_Parse(json_string);
    if (!json) return NULL;
    
    PathRequest* request = malloc(sizeof(PathRequest));
    if (!request) {
        cJSON_Delete(json);
        return NULL;
    }
    
    // Initialize arrays
    request->boundary.points = NULL;
    request->boundary.count = 0;
    request->boundary.capacity = 0;
    request->timestamp = NULL;
    
    // Parse timestamp
    const cJSON *timestamp = cJSON_GetObjectItemCaseSensitive(json, "timestamp");
    if (cJSON_IsString(timestamp) && timestamp->valuestring) {
        request->timestamp = malloc(strlen(timestamp->valuestring) + 1);
        if (request->timestamp) {
            strcpy(request->timestamp, timestamp->valuestring);
        }
    }
    
    // Parse settings
    const cJSON *settings = cJSON_GetObjectItemCaseSensitive(json, "settings");
    if (cJSON_IsObject(settings)) {
        const cJSON *mowerWidth = cJSON_GetObjectItemCaseSensitive(settings, "mowerWidth");
        const cJSON *pathOverlap = cJSON_GetObjectItemCaseSensitive(settings, "pathOverlap");
        
        request->settings.mowerWidth = cJSON_IsNumber(mowerWidth) ? mowerWidth->valuedouble : 0.25;
        request->settings.pathOverlap = cJSON_IsNumber(pathOverlap) ? pathOverlap->valuedouble : 0.05;
    } else {
        request->settings.mowerWidth = 0.25;
        request->settings.pathOverlap = 0.05;
    }
    
    // Parse boundary
    const cJSON *boundary = cJSON_GetObjectItemCaseSensitive(json, "boundary");
    if (cJSON_IsObject(boundary)) {
        const cJSON *points = cJSON_GetObjectItemCaseSensitive(boundary, "points");
        if (cJSON_IsArray(points)) {
            int point_count = cJSON_GetArraySize(points);
            
            // Initialize boundary array
            request->boundary.points = malloc(sizeof(Point2D) * point_count);
            request->boundary.capacity = point_count;
            request->boundary.count = 0;
            
            if (request->boundary.points) {
                const cJSON *point = NULL;
                cJSON_ArrayForEach(point, points) {
                    if (cJSON_IsObject(point)) {
                        const cJSON *x = cJSON_GetObjectItemCaseSensitive(point, "x");
                        const cJSON *y = cJSON_GetObjectItemCaseSensitive(point, "y");
                        
                        if (cJSON_IsNumber(x) && cJSON_IsNumber(y)) {
                            request->boundary.points[request->boundary.count].x = x->valuedouble;
                            request->boundary.points[request->boundary.count].y = y->valuedouble;
                            request->boundary.count++;
                        }
                    }
                }
            }
        }
    }
    
    cJSON_Delete(json);
    return request;
}

// Path planning algorithms implementation
void decompose_polygon(const PointArray* boundary, PointArray* decomposition) {
    if (!boundary || !decomposition || boundary->count < 3) return;
    
    // Simple decomposition: just copy boundary points for now
    // In a more advanced implementation, this would decompose complex polygons
    // into simpler convex shapes
    for (int i = 0; i < boundary->count; i++) {
        add_point(decomposition, boundary->points[i].x, boundary->points[i].y);
    }
}

void get_bounding_box(const PointArray* boundary, Point2D* min_point, Point2D* max_point) {
    if (!boundary || boundary->count == 0 || !min_point || !max_point) return;
    
    min_point->x = max_point->x = boundary->points[0].x;
    min_point->y = max_point->y = boundary->points[0].y;
    
    for (int i = 1; i < boundary->count; i++) {
        if (boundary->points[i].x < min_point->x) min_point->x = boundary->points[i].x;
        if (boundary->points[i].x > max_point->x) max_point->x = boundary->points[i].x;
        if (boundary->points[i].y < min_point->y) min_point->y = boundary->points[i].y;
        if (boundary->points[i].y > max_point->y) max_point->y = boundary->points[i].y;
    }
}

int get_horizontal_intersections(double y, const PointArray* boundary, double* intersections, int max_intersections) {
    if (!boundary || !intersections || boundary->count < 3) return 0;
    
    int intersection_count = 0;
    
    for (int i = 0; i < boundary->count && intersection_count < max_intersections; i++) {
        Point2D p1 = boundary->points[i];
        Point2D p2 = boundary->points[(i + 1) % boundary->count];
        
        // Check if the line segment crosses the horizontal line
        if ((p1.y <= y && p2.y > y) || (p1.y > y && p2.y <= y)) {
            // Calculate intersection point
            double t = (y - p1.y) / (p2.y - p1.y);
            double x = p1.x + t * (p2.x - p1.x);
            intersections[intersection_count++] = x;
        }
    }
    
    return intersection_count;
}

// Simple comparison function for sorting doubles
int compare_doubles(const void* a, const void* b) {
    double da = *(const double*)a;
    double db = *(const double*)b;
    return (da > db) - (da < db);
}

void generate_boustrophedon_path(const PointArray* boundary, const PathSettings* settings, PointArray* path) {
    if (!boundary || !settings || !path || boundary->count < 3) return;
    
    Point2D min_point, max_point;
    get_bounding_box(boundary, &min_point, &max_point);
    
    // Calculate spacing between path centers: mower width minus overlap
    double path_center_spacing = settings->mowerWidth - settings->pathOverlap;
    double current_y = min_point.y + path_center_spacing;
    int going_right = 1;
    
    // Buffer for intersections (max 100 intersections per line)
    double intersections[100];
    
    while (current_y < max_point.y) {
        int intersection_count = get_horizontal_intersections(current_y, boundary, intersections, 100);
        
        if (intersection_count >= 2) {
            // Sort intersections
            qsort(intersections, intersection_count, sizeof(double), compare_doubles);
            
            // Take the leftmost and rightmost intersections
            double start_x = intersections[0];
            double end_x = intersections[intersection_count - 1];
            
            if (going_right) {
                add_point(path, start_x, current_y);
                add_point(path, end_x, current_y);
            } else {
                add_point(path, end_x, current_y);
                add_point(path, start_x, current_y);
            }
            
            going_right = !going_right;
        }
        
        current_y += path_center_spacing;
    }
}

double calculate_polygon_area(const PointArray* boundary) {
    if (!boundary || boundary->count < 3) return 0.0;
    
    double area = 0.0;
    for (int i = 0; i < boundary->count; i++) {
        int j = (i + 1) % boundary->count;
        area += boundary->points[i].x * boundary->points[j].y;
        area -= boundary->points[j].x * boundary->points[i].y;
    }
    
    return fabs(area) / 2.0;
}

// Main path calculation function
PathResponse* calculate_coverage_path(const PathRequest* request) {
    if (!request || request->boundary.count < 3) return NULL;
    
    PathResponse* response = malloc(sizeof(PathResponse));
    if (!response) return NULL;
    
    // Initialize response
    response->decomposition.points = NULL;
    response->decomposition.count = 0;
    response->decomposition.capacity = 0;
    response->path.points = NULL;
    response->path.count = 0;
    response->path.capacity = 0;
    response->req_timestamp = NULL;
    
    // Copy timestamp
    if (request->timestamp) {
        response->req_timestamp = malloc(strlen(request->timestamp) + 1);
        if (response->req_timestamp) {
            strcpy(response->req_timestamp, request->timestamp);
        }
    }
    
    // Create arrays
    PointArray* decomposition = create_point_array(request->boundary.count);
    PointArray* path = create_point_array(100); // Initial capacity for path points
    
    if (!decomposition || !path) {
        free_point_array(decomposition);
        free_point_array(path);
        free_path_response(response);
        return NULL;
    }
    
    // Perform decomposition
    decompose_polygon(&request->boundary, decomposition);
    
    // Generate coverage path
    generate_boustrophedon_path(&request->boundary, &request->settings, path);
    
    // Transfer ownership to response
    response->decomposition = *decomposition;
    response->path = *path;
    
    // Free the array structures but not the points (now owned by response)
    free(decomposition);
    free(path);
    
    //printf("Path calculation completed: %d decomposition points, %d path points\n", 
    //       response->decomposition.count, response->path.count);
    
    return response;
}

// JSON generation function
char* generate_path_response_json(const PathResponse* response) {
    if (!response) return NULL;
    
    cJSON *root = cJSON_CreateObject();
    if (!root) return NULL;
    
    // Add timestamp
    if (response->req_timestamp) {
        cJSON *timestamp = cJSON_CreateString(response->req_timestamp);
        cJSON_AddItemToObject(root, "req_timestamp", timestamp);
    }
    
    // Add decomposition
    cJSON *decomposition = cJSON_CreateObject();
    cJSON *dec_point_count = cJSON_CreateNumber(response->decomposition.count);
    cJSON_AddItemToObject(decomposition, "decPointCount", dec_point_count);
    
    cJSON *dec_points = cJSON_CreateArray();
    for (int i = 0; i < response->decomposition.count; i++) {
        cJSON *point = cJSON_CreateObject();
        cJSON *x = cJSON_CreateNumber(response->decomposition.points[i].x);
        cJSON *y = cJSON_CreateNumber(response->decomposition.points[i].y);
        cJSON_AddItemToObject(point, "x", x);
        cJSON_AddItemToObject(point, "y", y);
        cJSON_AddItemToArray(dec_points, point);
    }
    cJSON_AddItemToObject(decomposition, "decPoints", dec_points);
    cJSON_AddItemToObject(root, "decomposition", decomposition);
    
    // Add path
    cJSON *path = cJSON_CreateObject();
    cJSON *path_point_count = cJSON_CreateNumber(response->path.count);
    cJSON_AddItemToObject(path, "pathPointCount", path_point_count);
    
    cJSON *path_points = cJSON_CreateArray();
    for (int i = 0; i < response->path.count; i++) {
        cJSON *point = cJSON_CreateObject();
        cJSON *x = cJSON_CreateNumber(response->path.points[i].x);
        cJSON *y = cJSON_CreateNumber(response->path.points[i].y);
        cJSON_AddItemToObject(point, "x", x);
        cJSON_AddItemToObject(point, "y", y);
        cJSON_AddItemToArray(path_points, point);
    }
    cJSON_AddItemToObject(path, "pathPoints", path_points);
    cJSON_AddItemToObject(root, "path", path);
    
    // Convert to string
    char *json_string = cJSON_Print(root);
    cJSON_Delete(root);
    
    // Validate JSON string
    if (!json_string) {
        printf("ERROR: cJSON_Print returned NULL\n");
        return NULL;
    }
    
    // Check for reasonable size (prevent extremely large responses)
    size_t json_len = strlen(json_string);
    if (json_len > 1000000) { // 1MB limit
        printf("WARNING: JSON response is very large (%zu bytes), truncating...\n", json_len);
        free(json_string);
        return NULL;
    }
    
    //printf("JSON generation successful, size: %zu bytes\n", json_len);
    return json_string;
}