#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON/cJSON.h"
#include "mongoose/mongoose.h"

typedef enum {
    ROUTE_SEND,
    ROUTE_TEST,
    ROUTE_STYLES,
    ROUTE_SCRIPT,
    ROUTE_UNKNOWN
} route_type_t;

static void fn(struct mg_connection *c, int ev, void *ev_data);
static void handle_test_route(struct mg_connection *c, struct mg_http_message *hm);
static void handle_send_route(struct mg_connection *c, struct mg_http_message *hm);
static void handle_styles_route(struct mg_connection *c, struct mg_http_message *hm);
static void handle_script_route(struct mg_connection *c, struct mg_http_message *hm);
static void handle_not_found(struct mg_connection *c, struct mg_http_message *hm);
static route_type_t get_route_type(struct mg_str uri);

int main()
{
    struct mg_mgr mgr;
    
    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, "http://0.0.0.0:8000", fn, NULL);
    printf("Server started on http://localhost:8000\n");
    
    for (;;) mg_mgr_poll(&mgr, 1000);
    
    mg_mgr_free(&mgr);
    return 0;
}

static void fn(struct mg_connection *c, int ev, void *ev_data) 
{
    if (ev == MG_EV_HTTP_MSG) 
    {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        
        switch (get_route_type(hm->uri)) 
        {
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

static void handle_test_route(struct mg_connection *c, struct mg_http_message *hm)
{
    struct mg_http_serve_opts opts = {.root_dir = "../../web/test"};
    mg_http_serve_file(c, hm, "../../web/test/test-index.html", &opts);
}

static void handle_send_route(struct mg_connection *c, struct mg_http_message *hm)
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

static void handle_styles_route(struct mg_connection *c, struct mg_http_message *hm)
{
    struct mg_http_serve_opts opts = {.root_dir = "../../web/test"};
    mg_http_serve_file(c, hm, "../../web/test/test-styles.css", &opts);
}

static void handle_script_route(struct mg_connection *c, struct mg_http_message *hm)
{
    struct mg_http_serve_opts opts = {.root_dir = "../../web/test"};
    mg_http_serve_file(c, hm, "../../web/test/test-script.js", &opts);
}

static void handle_not_found(struct mg_connection *c, struct mg_http_message *hm)
{
    mg_http_reply(c, 404, "Access-Control-Allow-Origin: *\r\n", "Not found");
}

static route_type_t get_route_type(struct mg_str uri)
{
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
    return ROUTE_UNKNOWN;
}