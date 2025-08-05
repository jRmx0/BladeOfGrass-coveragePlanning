#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "webserver.h"
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
                
            case ROUTE_PATH_APP_SCRIPT:
                handle_path_app_script_route(c, hm);
                break;

            case ROUTE_PATH_BOUNDARY_MODEL_SCRIPT:
                handle_path_boundary_model_script_route(c, hm);
                break;

            case ROUTE_PATH_OBSTACLE_MODEL_SCRIPT:
                handle_path_obstacle_model_script_route(c, hm);
                break;

            case ROUTE_PATH_CANVAS_MANAGER_SCRIPT:
                handle_path_canvas_manager_script_route(c, hm);
                break;

            case ROUTE_PATH_UI_CONTROLLER_SCRIPT:
                handle_path_ui_controller_script_route(c, hm);
                break;

            case ROUTE_PATH_DATA_SERVICE_SCRIPT:
                handle_path_data_service_script_route(c, hm);
                break;

            case ROUTE_PATH_UI_STATE_MANAGER_SCRIPT:
                handle_path_ui_state_manager_script_route(c, hm);
                break;

            case ROUTE_PATH_COORDINATE_TRANSFORMER_SCRIPT:
                handle_path_coordinate_transformer_script_route(c, hm);
                break;

            case ROUTE_PATH_ALGORITHM_SERVICE_SCRIPT:
                handle_path_algorithm_service_script_route(c, hm);
                break;
                            
            case ROUTE_PATH_BOUSTROPHEDON_SCRIPT:
                handle_path_boustrophedon_script_route(c, hm);
                break;
            
            case ROUTE_PATH_TESTING_SCRIPT:
                handle_path_testing_script_route(c, hm);
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
            
            case ROUTE_SAVE_MODEL:
                handle_save_model_route(c, hm);
                break;

            case ROUTE_SAVE_DECOMPOSITION:
                handle_save_decomposition_route(c, hm);
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
    mg_http_serve_file(c, hm, "../../web/path_canvas/index.html", &opts);
}

void handle_path_index_route(struct mg_connection *c, struct mg_http_message *hm)
{
    struct mg_http_serve_opts opts = {.root_dir = "../../web/path_canvas"};
    mg_http_serve_file(c, hm, "../../web/path_canvas/index.html", &opts);
}

void handle_path_styles_route(struct mg_connection *c, struct mg_http_message *hm)
{
    struct mg_http_serve_opts opts = {.root_dir = "../../web/path_canvas"};
    mg_http_serve_file(c, hm, "../../web/path_canvas/styles.css", &opts);
}

void handle_path_script_route(struct mg_connection *c, struct mg_http_message *hm)
{
    struct mg_http_serve_opts opts = {.root_dir = "../../web/path_canvas"};
    mg_http_serve_file(c, hm, "../../web/path_canvas/app.js", &opts);
}

void handle_path_app_script_route(struct mg_connection *c, struct mg_http_message *hm)
{
    struct mg_http_serve_opts opts = {.root_dir = "../../web/path_canvas"};
    mg_http_serve_file(c, hm, "../../web/path_canvas/app.js", &opts);
}

void handle_path_boundary_model_script_route(struct mg_connection *c, struct mg_http_message *hm)
{
    struct mg_http_serve_opts opts = {.root_dir = "../../web/path_canvas"};
    mg_http_serve_file(c, hm, "../../web/path_canvas/models/BoundaryModel.js", &opts);
}

void handle_path_obstacle_model_script_route(struct mg_connection *c, struct mg_http_message *hm)
{
    struct mg_http_serve_opts opts = {.root_dir = "../../web/path_canvas"};
    mg_http_serve_file(c, hm, "../../web/path_canvas/models/ObstacleModel.js", &opts);
}

void handle_path_canvas_manager_script_route(struct mg_connection *c, struct mg_http_message *hm)
{
    struct mg_http_serve_opts opts = {.root_dir = "../../web/path_canvas"};
    mg_http_serve_file(c, hm, "../../web/path_canvas/ui/CanvasManager.js", &opts);
}

void handle_path_ui_controller_script_route(struct mg_connection *c, struct mg_http_message *hm)
{
    struct mg_http_serve_opts opts = {.root_dir = "../../web/path_canvas"};
    mg_http_serve_file(c, hm, "../../web/path_canvas/ui/UIController.js", &opts);
}

void handle_path_data_service_script_route(struct mg_connection *c, struct mg_http_message *hm)
{
    struct mg_http_serve_opts opts = {.root_dir = "../../web/path_canvas"};
    mg_http_serve_file(c, hm, "../../web/path_canvas/services/DataService.js", &opts);
}

void handle_path_ui_state_manager_script_route(struct mg_connection *c, struct mg_http_message *hm)
{
    struct mg_http_serve_opts opts = {.root_dir = "../../web/path_canvas"};
    mg_http_serve_file(c, hm, "../../web/path_canvas/ui/UIStateManager.js", &opts);
}

void handle_path_coordinate_transformer_script_route(struct mg_connection *c, struct mg_http_message *hm)
{
    struct mg_http_serve_opts opts = {.root_dir = "../../web/path_canvas"};
    mg_http_serve_file(c, hm, "../../web/path_canvas/services/CoordinateTransformer.js", &opts);
}

void handle_path_algorithm_service_script_route(struct mg_connection *c, struct mg_http_message *hm)
{
    struct mg_http_serve_opts opts = {.root_dir = "../../web/path_canvas"};
    mg_http_serve_file(c, hm, "../../web/path_canvas/services/AlgorithmService.js", &opts);
}

void handle_path_boustrophedon_script_route(struct mg_connection *c, struct mg_http_message *hm)
{
    struct mg_http_serve_opts opts = {.root_dir = "../../web/path_canvas"};
    mg_http_serve_file(c, hm, "../../web/path_canvas/algorithms/boustrophedon.js", &opts);
}

void handle_path_testing_script_route(struct mg_connection *c, struct mg_http_message *hm)
{
    struct mg_http_serve_opts opts = {.root_dir = "../../web/path_canvas"};
    mg_http_serve_file(c, hm, "../../web/path_canvas/utils/testing.js", &opts);
}

void handle_path_calculate_route(struct mg_connection *c, struct mg_http_message *hm)
{
    
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


void handle_save_model_route(struct mg_connection *c, struct mg_http_message *hm) {
    char *body_str = malloc(hm->body.len + 1);
    memcpy(body_str, hm->body.buf, hm->body.len);
    body_str[hm->body.len] = '\0';

    cJSON *json = cJSON_Parse(body_str);
    if (json == NULL) {
        mg_http_reply(c, 400, "Content-Type: application/json\r\n", "{\"error\":\"Invalid JSON\"}");
        free(body_str);
        return;
    }

    const cJSON *id_json = cJSON_GetObjectItemCaseSensitive(json, "id");
    if (!cJSON_IsString(id_json) || (id_json->valuestring == NULL)) {
        mg_http_reply(c, 400, "Content-Type: application/json\r\n", "{\"error\":\"Missing or invalid model ID\"}");
        cJSON_Delete(json);
        free(body_str);
        return;
    }

    char filename[256];
    snprintf(filename, sizeof(filename), "../../web/path_canvas/state/%s.json", id_json->valuestring);

    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        mg_http_reply(c, 500, "Content-Type: application/json\r\n", "{\"error\":\"Could not open file for writing\"}");
        cJSON_Delete(json);
        free(body_str);
        return;
    }

    char *json_str = cJSON_Print(json);
    fprintf(file, "%s", json_str);
    fclose(file);

    mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"action\":\"model file saved\", \"status\":\"success\"}");
    cJSON_Delete(json);
    free(body_str);
    free(json_str);
}

void handle_save_decomposition_route(struct mg_connection *c, struct mg_http_message *hm) {
    char *body_str = malloc(hm->body.len + 1);
    memcpy(body_str, hm->body.buf, hm->body.len);
    body_str[hm->body.len] = '\0';

    cJSON *json = cJSON_Parse(body_str);
    if (json == NULL) {
        mg_http_reply(c, 400, "Content-Type: application/json\r\n", "{\"error\":\"Invalid JSON\"}");
        free(body_str);
        return;
    }

    const cJSON *id_json = cJSON_GetObjectItemCaseSensitive(json, "id");
    if (!cJSON_IsString(id_json) || (id_json->valuestring == NULL)) {
        mg_http_reply(c, 400, "Content-Type: application/json\r\n", "{\"error\":\"Missing or invalid ID\"}");
        cJSON_Delete(json);
        free(body_str);
        return;
    }

    char filename[256];
    snprintf(filename, sizeof(filename), "../../web/path_canvas/state/%s.json", id_json->valuestring);

    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        mg_http_reply(c, 500, "Content-Type: application/json\r\n", "{\"error\":\"Could not open file for writing\"}");
        cJSON_Delete(json);
        free(body_str);
        return;
    }

    char *json_str = cJSON_Print(json);
    fprintf(file, "%s", json_str);
    fclose(file);

    mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"action\":\"cells file saved\", \"status\":\"success\"}");
    cJSON_Delete(json);
    free(body_str);
    free(json_str);
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
    if (mg_strcmp(uri, mg_str("/styles.css")) == 0) {
        return ROUTE_PATH_STYLES;
    }
    if (mg_strcmp(uri, mg_str("/path-script.js")) == 0) {
        return ROUTE_PATH_APP_SCRIPT;
    }
    if (mg_strcmp(uri, mg_str("/app.js")) == 0) {
        return ROUTE_PATH_APP_SCRIPT;
    }
    if (mg_strcmp(uri, mg_str("/BoundaryModel.js")) == 0) {
        return ROUTE_PATH_BOUNDARY_MODEL_SCRIPT;
    }
    if (mg_strcmp(uri, mg_str("/models/BoundaryModel.js")) == 0) {
        return ROUTE_PATH_BOUNDARY_MODEL_SCRIPT;
    }
    if (mg_strcmp(uri, mg_str("/ObstacleModel.js")) == 0) {
        return ROUTE_PATH_OBSTACLE_MODEL_SCRIPT;
    }
    if (mg_strcmp(uri, mg_str("/models/ObstacleModel.js")) == 0) {
        return ROUTE_PATH_OBSTACLE_MODEL_SCRIPT;
    }
    if (mg_strcmp(uri, mg_str("/CanvasManager.js")) == 0) {
        return ROUTE_PATH_CANVAS_MANAGER_SCRIPT;
    }
    if (mg_strcmp(uri, mg_str("/ui/CanvasManager.js")) == 0) {
        return ROUTE_PATH_CANVAS_MANAGER_SCRIPT;
    }
    if (mg_strcmp(uri, mg_str("/UIController.js")) == 0) {
        return ROUTE_PATH_UI_CONTROLLER_SCRIPT;
    }
    if (mg_strcmp(uri, mg_str("/ui/UIController.js")) == 0) {
        return ROUTE_PATH_UI_CONTROLLER_SCRIPT;
    }
    if (mg_strcmp(uri, mg_str("/services/DataService.js")) == 0) {
        return ROUTE_PATH_DATA_SERVICE_SCRIPT;
    }
    if (mg_strcmp(uri, mg_str("/ui/UIStateManager.js")) == 0) {
        return ROUTE_PATH_UI_STATE_MANAGER_SCRIPT;
    }
    if (mg_strcmp(uri, mg_str("/services/CoordinateTransformer.js")) == 0) {
        return ROUTE_PATH_COORDINATE_TRANSFORMER_SCRIPT;
    }
    if (mg_strcmp(uri, mg_str("/services/AlgorithmService.js")) == 0) {
        return ROUTE_PATH_ALGORITHM_SERVICE_SCRIPT;
    }
    if (mg_strcmp(uri, mg_str("/path-calculate")) == 0) {
        return ROUTE_PATH_CALCULATE;
    }
    if (mg_strcmp(uri, mg_str("/boustrophedon.js")) == 0) {
        return ROUTE_PATH_BOUSTROPHEDON_SCRIPT;
    }
    if (mg_strcmp(uri, mg_str("/algorithms/boustrophedon.js")) == 0) {
        return ROUTE_PATH_BOUSTROPHEDON_SCRIPT;
    }
    if (mg_strcmp(uri, mg_str("/testing.js")) == 0) {
        return ROUTE_PATH_TESTING_SCRIPT;
    }
    if (mg_strcmp(uri, mg_str("/utils/testing.js")) == 0) {
        return ROUTE_PATH_TESTING_SCRIPT;
    }
    if (mg_strcmp(uri, mg_str("/save-model")) == 0) {
        return ROUTE_SAVE_MODEL;
    }
    if (mg_strcmp(uri, mg_str("/save-decomposition")) == 0) {
        return ROUTE_SAVE_DECOMPOSITION;
    }
    return ROUTE_UNKNOWN;
}
