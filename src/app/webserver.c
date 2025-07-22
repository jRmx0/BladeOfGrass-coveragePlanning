#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "webserver.h"
#include "pathPlanner.h"
#include "../../dependencies/cJSON/cJSON.h"

void webserver_init(struct mg_mgr *mgr, const char *listen_url)
{
    mg_mgr_init(mgr);
    mg_http_listen(mgr, listen_url, webserver_event_handler, NULL);
    printf("Server started on %s\n", listen_url);
}

void webserver_event_handler(struct mg_connection *c, int ev, void *ev_data) 
{
    if (ev == MG_EV_HTTP_MSG) 
    {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        
        switch (get_route_type(hm->uri)) 
        {
            case ROUTE_PATH:
                handle_path_route(c, hm);
                break;
            
            case ROUTE_PATH_INDEX:
                handle_path_index_route(c, hm);
                break;
            
            case ROUTE_PATH_STYLES:
                handle_path_styles_route(c, hm);
                break;
                
            case ROUTE_PATH_SCRIPT:
                handle_path_script_route(c, hm);
                break;
                
            case ROUTE_PATH_CALCULATE:
                handle_path_calculate_route(c, hm);
                break;

            case ROUTE_SEND:
                handle_send_route(c, hm);
                break;
            
            case ROUTE_TEST:
                handle_test_route(c, hm);
                break;

            case ROUTE_STYLES:
                handle_styles_route(c, hm);
                break;
                
            case ROUTE_SCRIPT:
                handle_script_route(c, hm);
                break;
                
            case ROUTE_UNKNOWN:
            default:
                handle_not_found(c, hm);
                break;
        }
    }
}


void handle_path_route(struct mg_connection *c, struct mg_http_message *hm)
{
    struct mg_http_serve_opts opts = {.root_dir = "../../web/path_canvas"};
    mg_http_serve_file(c, hm, "../../web/path_canvas/path-index.html", &opts);
}

void handle_path_index_route(struct mg_connection *c, struct mg_http_message *hm)
{
    struct mg_http_serve_opts opts = {.root_dir = "../../web/path_canvas"};
    mg_http_serve_file(c, hm, "../../web/path_canvas/path-index.html", &opts);
}

void handle_path_styles_route(struct mg_connection *c, struct mg_http_message *hm)
{
    struct mg_http_serve_opts opts = {.root_dir = "../../web/path_canvas"};
    mg_http_serve_file(c, hm, "../../web/path_canvas/path-styles.css", &opts);
}

void handle_path_script_route(struct mg_connection *c, struct mg_http_message *hm)
{
    struct mg_http_serve_opts opts = {.root_dir = "../../web/path_canvas"};
    mg_http_serve_file(c, hm, "../../web/path_canvas/path-script.js", &opts);
}

void handle_path_calculate_route(struct mg_connection *c, struct mg_http_message *hm)
{
    char *body_str = malloc(hm->body.len + 1);
    memcpy(body_str, hm->body.buf, hm->body.len);
    body_str[hm->body.len] = '\0';
    
    printf("Received path calculation request\n");
    
    // Parse the request using the path planner
    PathRequest* request = parse_path_request(body_str);
    if (!request) {
        free(body_str);
        mg_http_reply(c, 400, "Content-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\n", 
                      "{\"error\":\"Invalid JSON or failed to parse request\"}");
        return;
    }
        
    // Calculate the coverage path
    PathResponse* response = calculate_coverage_path(request);
    if (!response) {
        free_path_request(request);
        free(body_str);
        mg_http_reply(c, 500, "Content-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\n", 
                      "{\"error\":\"Failed to calculate path\"}");
        return;
    }
    
    // Generate JSON response
    char* response_json = generate_path_response_json(response);
        
    // Send response with safer format string
    const char* headers = "Content-Type: application/json\r\n"
                         "Access-Control-Allow-Origin: *\r\n"
                         "Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n"
                         "Access-Control-Allow-Headers: Content-Type\r\n";
    
    mg_http_reply(c, 200, headers, "%s", response_json);
    
    free(response_json);
    free_path_response(response);
    free_path_request(request);
    free(body_str);
}


void handle_test_route(struct mg_connection *c, struct mg_http_message *hm)
{
    struct mg_http_serve_opts opts = {.root_dir = "../../web/test"};
    mg_http_serve_file(c, hm, "../../web/test/test-index.html", &opts);
}

void handle_send_route(struct mg_connection *c, struct mg_http_message *hm)
{
    char *body_str = malloc(hm->body.len + 1);
    memcpy(body_str, hm->body.buf, hm->body.len);
    body_str[hm->body.len] = '\0';
    
    cJSON *json = cJSON_Parse(body_str);
    const cJSON *msg = cJSON_GetObjectItemCaseSensitive(json, "msg");
    char response[256] = {0};
    
    if (cJSON_IsString(msg) && (msg->valuestring != NULL)) 
    {
        snprintf(response, sizeof(response), "{\"reply\":\"Received: %s\"}", msg->valuestring);
    } 
    else 
    {
        snprintf(response, sizeof(response), "{\"error\":\"No msg\"}");
    }
    
    printf("Sending response: %s\n", response);
    
    cJSON_Delete(json);
    free(body_str);
    mg_http_reply(c, 200, "Content-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nAccess-Control-Allow-Methods: POST, GET, OPTIONS\r\nAccess-Control-Allow-Headers: Content-Type\r\n", "%s", response);
}

void handle_styles_route(struct mg_connection *c, struct mg_http_message *hm)
{
    struct mg_http_serve_opts opts = {.root_dir = "../../web/test"};
    mg_http_serve_file(c, hm, "../../web/test/test-styles.css", &opts);
}

void handle_script_route(struct mg_connection *c, struct mg_http_message *hm)
{
    struct mg_http_serve_opts opts = {.root_dir = "../../web/test"};
    mg_http_serve_file(c, hm, "../../web/test/test-script.js", &opts);
}


void handle_not_found(struct mg_connection *c, struct mg_http_message *hm)
{
    mg_http_reply(c, 404, "Access-Control-Allow-Origin: *\r\n", "Not found");
}


route_type_t get_route_type(struct mg_str uri)
{
    if (mg_strcmp(uri, mg_str("/path")) == 0) {
        return ROUTE_PATH;
    }
    if (mg_strcmp(uri, mg_str("/send")) == 0) {
        return ROUTE_SEND;
    }
    if (mg_strcmp(uri, mg_str("/test")) == 0) {
        return ROUTE_TEST;
    }
    if (mg_strcmp(uri, mg_str("/test-styles.css")) == 0) {
        return ROUTE_STYLES;
    }
    if (mg_strcmp(uri, mg_str("/test-script.js")) == 0) {
        return ROUTE_SCRIPT;
    }
    if (mg_strcmp(uri, mg_str("/path-index.html")) == 0) {
        return ROUTE_PATH_INDEX;
    }
    if (mg_strcmp(uri, mg_str("/path-styles.css")) == 0) {
        return ROUTE_PATH_STYLES;
    }
    if (mg_strcmp(uri, mg_str("/path-script.js")) == 0) {
        return ROUTE_PATH_SCRIPT;
    }
    if (mg_strcmp(uri, mg_str("/path-calculate")) == 0) {
        return ROUTE_PATH_CALCULATE;
    }
    return ROUTE_UNKNOWN;
}
