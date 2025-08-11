#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "../../dependencies/mongoose/mongoose.h"

typedef enum {
    ROUTE_HOME,
    // /environment
    ROUTE_PATH_INPUT_ENVIRONMENT_SCRIPT,
    ROUTE_PATH_INPUT_ENVIRONMENT_EXPORT,
    // /services
    ROUTE_PATH_COORDINATE_TRANSFORMER_SCRIPT,
    ROUTE_PATH_DATA_SERVICE_SCRIPT,
    // /ui
    ROUTE_PATH_CANVAS_MANAGER_SCRIPT,
    ROUTE_PATH_UI_CONTROLLER_SCRIPT,
    ROUTE_PATH_UI_STATE_MANAGER_SCRIPT,
    // /utils
    ROUTE_PATH_TESTING_SCRIPT,
    // /
    ROUTE_PATH_INDEX,
    ROUTE_PATH_STYLES,
    ROUTE_PATH_APP_SCRIPT,
    // template
    ROUTE_SEND,
    // test
    ROUTE_TEST_INDEX,
    ROUTE_TEST_STYLES,
    ROUTE_TEST_SCRIPT,
    // fallback
    ROUTE_UNKNOWN
} route_type_t;

void webserver_init(struct mg_mgr *mgr, const char *listen_url);
void webserver_event_handler(struct mg_connection *c, int ev, void *ev_data);



// /environment
void handle_path_input_environment_script_route(struct mg_connection *c, struct mg_http_message *hm);

// /services
void handle_path_coordinate_transformer_script_route(struct mg_connection *c, struct mg_http_message *hm);
void handle_path_data_service_script_route(struct mg_connection *c, struct mg_http_message *hm);

// /ui
void handle_path_canvas_manager_script_route(struct mg_connection *c, struct mg_http_message *hm);
void handle_path_ui_controller_script_route(struct mg_connection *c, struct mg_http_message *hm);
void handle_path_ui_state_manager_script_route(struct mg_connection *c, struct mg_http_message *hm);

// /utils
void handle_path_testing_script_route(struct mg_connection *c, struct mg_http_message *hm);

// /
void handle_path_index_route(struct mg_connection *c, struct mg_http_message *hm);
void handle_path_styles_route(struct mg_connection *c, struct mg_http_message *hm);
void handle_path_app_script_route(struct mg_connection *c, struct mg_http_message *hm);

// /template
void handle_send_route(struct mg_connection *c, struct mg_http_message *hm);

// /environment export
void handle_path_input_environment_export_route(struct mg_connection *c, struct mg_http_message *hm);

// /test
void handle_test_route(struct mg_connection *c, struct mg_http_message *hm);
void handle_styles_route(struct mg_connection *c, struct mg_http_message *hm);
void handle_script_route(struct mg_connection *c, struct mg_http_message *hm);

// fallback
void handle_not_found(struct mg_connection *c, struct mg_http_message *hm);

route_type_t get_route_type(struct mg_str uri);

#endif
