#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "../../dependencies/mongoose/mongoose.h"

typedef enum {
    ROUTE_PATH,
    ROUTE_PATH_INDEX,
    ROUTE_PATH_STYLES,
    ROUTE_PATH_SCRIPT,
    ROUTE_PATH_CALCULATE,
    ROUTE_SEND,
    ROUTE_TEST,
    ROUTE_STYLES,
    ROUTE_SCRIPT,
    ROUTE_UNKNOWN
} route_type_t;

void webserver_init(struct mg_mgr *mgr, const char *listen_url);
void webserver_event_handler(struct mg_connection *c, int ev, void *ev_data);

void handle_path_route(struct mg_connection *c, struct mg_http_message *hm);
void handle_path_index_route(struct mg_connection *c, struct mg_http_message *hm);
void handle_path_styles_route(struct mg_connection *c, struct mg_http_message *hm);
void handle_path_script_route(struct mg_connection *c, struct mg_http_message *hm);
void handle_path_calculate_route(struct mg_connection *c, struct mg_http_message *hm);

void handle_test_route(struct mg_connection *c, struct mg_http_message *hm);
void handle_send_route(struct mg_connection *c, struct mg_http_message *hm);
void handle_styles_route(struct mg_connection *c, struct mg_http_message *hm);
void handle_script_route(struct mg_connection *c, struct mg_http_message *hm);

void handle_not_found(struct mg_connection *c, struct mg_http_message *hm);

route_type_t get_route_type(struct mg_str uri);

#endif
