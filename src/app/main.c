#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON/cJSON.h"
#include "mongoose/mongoose.h"

static void fn(struct mg_connection *c, int ev, void *ev_data);

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

static void fn(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        if (mg_strcmp(hm->uri, mg_str("/send")) == 0) {
            char *body_str = malloc(hm->body.len + 1);
            memcpy(body_str, hm->body.buf, hm->body.len);
            body_str[hm->body.len] = '\0';
            
            cJSON *json = cJSON_Parse(body_str);
            const cJSON *msg = cJSON_GetObjectItemCaseSensitive(json, "msg");
            char response[256] = {0};
            if (cJSON_IsString(msg) && (msg->valuestring != NULL)) {
                snprintf(response, sizeof(response), "{\"reply\":\"Received: %s\"}", msg->valuestring);
            } else {
                snprintf(response, sizeof(response), "{\"error\":\"No msg\"}");
            }
            printf("Sending response: %s\n", response);
            cJSON_Delete(json);
            free(body_str);
            mg_http_reply(c, 200, "Content-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nAccess-Control-Allow-Methods: POST, GET, OPTIONS\r\nAccess-Control-Allow-Headers: Content-Type\r\n", "%s", response);
        } else {
            mg_http_reply(c, 404, "Access-Control-Allow-Origin: *\r\n", "Not found");
        }
    }
}