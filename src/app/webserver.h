#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "../../dependencies/mongoose/mongoose.h"

typedef enum {
    ROUTE_PATH,
    ROUTE_PATH_INDEX,
    ROUTE_PATH_STYLES,
    ROUTE_PATH_SCRIPT,
    ROUTE_PATH_BOUNDARY_MODEL_SCRIPT,
    ROUTE_PATH_OBSTACLE_MODEL_SCRIPT,
    ROUTE_PATH_CANVAS_MANAGER_SCRIPT,
    ROUTE_PATH_UI_CONTROLLER_SCRIPT,
    ROUTE_PATH_BOUSTROPHEDON_SCRIPT,
    ROUTE_PATH_TESTING_SCRIPT,
    ROUTE_PATH_CALCULATE,
    ROUTE_SEND,
    ROUTE_TEST,
    ROUTE_STYLES,
    ROUTE_SCRIPT,
    ROUTE_SAVE_MODEL,
    ROUTE_SAVE_DECOMPOSITION,
    ROUTE_UNKNOWN
} route_type_t;

void webserver_init(struct mg_mgr *mgr, const char *listen_url);
void webserver_event_handler(struct mg_connection *c, int ev, void *ev_data);

void handle_path_route(struct mg_connection *c, struct mg_http_message *hm);
void handle_path_index_route(struct mg_connection *c, struct mg_http_message *hm);
void handle_path_styles_route(struct mg_connection *c, struct mg_http_message *hm);
void handle_path_script_route(struct mg_connection *c, struct mg_http_message *hm);
void handle_path_boundary_model_script_route(struct mg_connection *c, struct mg_http_message *hm);
void handle_path_obstacle_model_script_route(struct mg_connection *c, struct mg_http_message *hm);
void handle_path_canvas_manager_script_route(struct mg_connection *c, struct mg_http_message *hm);
void handle_path_ui_controller_script_route(struct mg_connection *c, struct mg_http_message *hm);
void handle_path_boustrophedon_script_route(struct mg_connection *c, struct mg_http_message *hm);
void handle_path_testing_script_route(struct mg_connection *c, struct mg_http_message *hm);
void handle_path_calculate_route(struct mg_connection *c, struct mg_http_message *hm);


void handle_test_route(struct mg_connection *c, struct mg_http_message *hm);
void handle_send_route(struct mg_connection *c, struct mg_http_message *hm);
void handle_styles_route(struct mg_connection *c, struct mg_http_message *hm);
void handle_script_route(struct mg_connection *c, struct mg_http_message *hm);

void handle_save_model_route(struct mg_connection *c, struct mg_http_message *hm);
void handle_save_decomposition_route(struct mg_connection *c, struct mg_http_message *hm);

void handle_not_found(struct mg_connection *c, struct mg_http_message *hm);

route_type_t get_route_type(struct mg_str uri);

#endif
