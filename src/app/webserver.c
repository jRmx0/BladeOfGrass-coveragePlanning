#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "webserver.h"
#include "coverage_path_planning/coverage_path_planning.h"
#include "../../dependencies/cJSON/cJSON.h"
#include <sys/stat.h>
#include <direct.h>
#include <windows.h>

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
        struct mg_http_message *hm = (struct mg_http_message *)ev_data;

        switch (get_route_type(hm->uri))
        {
        case ROUTE_HOME:
            handle_path_index_route(c, hm);
            break;

        case ROUTE_PATH_INPUT_ENVIRONMENT_SCRIPT:
            handle_path_input_environment_script_route(c, hm);
            break;

        case ROUTE_PATH_COORDINATE_TRANSFORMER_SCRIPT:
            handle_path_coordinate_transformer_script_route(c, hm);
            break;

        case ROUTE_PATH_DATA_SERVICE_SCRIPT:
            handle_path_data_service_script_route(c, hm);
            break;

        case ROUTE_PATH_CANVAS_MANAGER_SCRIPT:
            handle_path_canvas_manager_script_route(c, hm);
            break;

        case ROUTE_PATH_UI_CONTROLLER_SCRIPT:
            handle_path_ui_controller_script_route(c, hm);
            break;

        case ROUTE_PATH_UI_STATE_MANAGER_SCRIPT:
            handle_path_ui_state_manager_script_route(c, hm);
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

        case ROUTE_SEND:
            handle_send_route(c, hm);
            break;
        case ROUTE_PATH_INPUT_ENVIRONMENT_EXPORT:
            handle_path_input_environment_export_route(c, hm);
            break;
        case ROUTE_PATH_INPUT_ENVIRONMENT_SAVE:
            handle_path_input_environment_save_route(c, hm);
            break;
        case ROUTE_PATH_INPUT_ENVIRONMENT_SAVES_LIST:
            handle_path_input_environment_saves_list_route(c, hm);
            break;
        case ROUTE_PATH_INPUT_ENVIRONMENT_LOAD:
            handle_path_input_environment_load_route(c, hm);
            break;
        case ROUTE_PATH_INPUT_ENVIRONMENT_DELETE:
            handle_path_input_environment_delete_route(c, hm);
            break;

        case ROUTE_TEST_INDEX:
            handle_test_route(c, hm);
            break;

        case ROUTE_TEST_STYLES:
            handle_styles_route(c, hm);
            break;

        case ROUTE_TEST_SCRIPT:
            handle_script_route(c, hm);
            break;

        case ROUTE_UNKNOWN:
        default:
            handle_not_found(c, hm);
            break;
        }
    }
}

void handle_path_input_environment_script_route(struct mg_connection *c, struct mg_http_message *hm)
{
    struct mg_http_serve_opts opts = {.root_dir = "../../web/path_canvas"};
    mg_http_serve_file(c, hm, "../../web/path_canvas/environment/InputEnvironment.js", &opts);
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

void handle_path_app_script_route(struct mg_connection *c, struct mg_http_message *hm)
{
    struct mg_http_serve_opts opts = {.root_dir = "../../web/path_canvas"};
    mg_http_serve_file(c, hm, "../../web/path_canvas/app.js", &opts);
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

// testing handler removed

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
    if (mg_strcmp(uri, mg_str("/")) == 0)
    {
        return ROUTE_HOME;
    }

    if (mg_strcmp(uri, mg_str("/environment/InputEnvironment.js")) == 0)
    {
        return ROUTE_PATH_INPUT_ENVIRONMENT_SCRIPT;
    }
    if (mg_strcmp(uri, mg_str("/environment/InputEnvironment/export")) == 0)
    {
        return ROUTE_PATH_INPUT_ENVIRONMENT_EXPORT;
    }
    if (mg_strcmp(uri, mg_str("/environment/InputEnvironment/save")) == 0)
    {
        return ROUTE_PATH_INPUT_ENVIRONMENT_SAVE;
    }
    if (mg_strcmp(uri, mg_str("/environment/InputEnvironment/saves")) == 0)
    {
        return ROUTE_PATH_INPUT_ENVIRONMENT_SAVES_LIST;
    }
    if (mg_strcmp(uri, mg_str("/environment/InputEnvironment/load")) == 0)
    {
        return ROUTE_PATH_INPUT_ENVIRONMENT_LOAD;
    }
    if (mg_strcmp(uri, mg_str("/environment/InputEnvironment/delete")) == 0)
    {
        return ROUTE_PATH_INPUT_ENVIRONMENT_DELETE;
    }

    if (mg_strcmp(uri, mg_str("/services/CoordinateTransformer.js")) == 0)
    {
        return ROUTE_PATH_COORDINATE_TRANSFORMER_SCRIPT;
    }
    if (mg_strcmp(uri, mg_str("/services/DataService.js")) == 0)
    {
        return ROUTE_PATH_DATA_SERVICE_SCRIPT;
    }

    if (mg_strcmp(uri, mg_str("/ui/CanvasManager.js")) == 0)
    {
        return ROUTE_PATH_CANVAS_MANAGER_SCRIPT;
    }
    if (mg_strcmp(uri, mg_str("/ui/UIController.js")) == 0)
    {
        return ROUTE_PATH_UI_CONTROLLER_SCRIPT;
    }
    if (mg_strcmp(uri, mg_str("/ui/UIStateManager.js")) == 0)
    {
        return ROUTE_PATH_UI_STATE_MANAGER_SCRIPT;
    }

    if (mg_strcmp(uri, mg_str("/path-index.html")) == 0 ||
        mg_strcmp(uri, mg_str("/index.html")) == 0)
    {
        return ROUTE_PATH_INDEX;
    }
    if (mg_strcmp(uri, mg_str("/path-styles.css")) == 0 ||
        mg_strcmp(uri, mg_str("/styles.css")) == 0)
    {
        return ROUTE_PATH_STYLES;
    }
    if (mg_strcmp(uri, mg_str("/path-script.js")) == 0 ||
        mg_strcmp(uri, mg_str("/app.js")) == 0)
    {
        return ROUTE_PATH_APP_SCRIPT;
    }

    if (mg_strcmp(uri, mg_str("/send")) == 0)
    {
        return ROUTE_SEND;
    }

    if (mg_strcmp(uri, mg_str("/test")) == 0)
    {
        return ROUTE_TEST_INDEX;
    }
    if (mg_strcmp(uri, mg_str("/test-styles.css")) == 0)
    {
        return ROUTE_TEST_STYLES;
    }
    if (mg_strcmp(uri, mg_str("/test-script.js")) == 0)
    {
        return ROUTE_TEST_SCRIPT;
    }

    return ROUTE_UNKNOWN;
}

void handle_path_input_environment_export_route(struct mg_connection *c, struct mg_http_message *hm)
{
    char *body_str = (char *)malloc(hm->body.len + 1);
    if (!body_str)
    {
        mg_http_reply(c, 500, "Access-Control-Allow-Origin: *\r\n", "{\"error\":\"OOM\"}");
        return;
    }
    memcpy(body_str, hm->body.buf, hm->body.len);
    body_str[hm->body.len] = '\0';

    char *result_json = coverage_path_planning_process(body_str);

    free(body_str);

    const char *headers = "Content-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nAccess-Control-Allow-Methods: POST, GET, OPTIONS\r\nAccess-Control-Allow-Headers: Content-Type\r\n";
    if (result_json)
    {
        mg_http_reply(
            c,
            200,
            headers,
            "%s",
            result_json);
        free(result_json);
    }
    else
    {
        mg_http_reply(
            c,
            500,
            headers,
            "{\"status\":\"error\",\"message\":\"no result\"}");
    }
}

static void ensure_save_dir_exists(void)
{
    struct _stat st;
    if (_stat("../../save_files", &st) != 0)
    {
        _mkdir("../../save_files");
    }
}

// Save environment JSON body to ../../save_files/<name>.json
void handle_path_input_environment_save_route(struct mg_connection *c, struct mg_http_message *hm)
{
    // Expect POST /environment/InputEnvironment/save?name=<filename>
    struct mg_str name = mg_http_var(hm->query, mg_str("name"));
    if (name.len == 0)
    {
        mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\n", "{\"error\":\"missing name\"}");
        return;
    }
    // sanitize name: allow [A-Za-z0-9-_]
    char fname[256] = {0};
    size_t j = 0;
    for (size_t i = 0; i < name.len && j < sizeof(fname) - 1; i++)
    {
        char ch = name.buf[i];
        if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '-' || ch == '_')
        {
            fname[j++] = ch;
        }
    }
    if (j == 0)
    {
        mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\n", "{\"error\":\"invalid name\"}");
        return;
    }

    char path[512];
    snprintf(path, sizeof(path), "../../save_files/%s.json", fname);

    ensure_save_dir_exists();
    FILE *f = fopen(path, "wb");
    if (!f)
    {
        mg_http_reply(c, 500, "Access-Control-Allow-Origin: *\r\n", "{\"error\":\"cannot open file\"}");
        return;
    }
    fwrite(hm->body.buf, 1, hm->body.len, f);
    fclose(f);
    mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "{\"status\":\"ok\"}");
}

// List saved files as JSON array
void handle_path_input_environment_saves_list_route(struct mg_connection *c, struct mg_http_message *hm)
{
    ensure_save_dir_exists();
    // Build a JSON array by scanning directory
    cJSON *arr = cJSON_CreateArray();
    // Windows directory iteration
    WIN32_FIND_DATAA ffd;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    char pattern[MAX_PATH];
    snprintf(pattern, sizeof(pattern), "../../save_files/*.json");
    hFind = FindFirstFileA(pattern, &ffd);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                // strip .json
                char name[260];
                strncpy(name, ffd.cFileName, sizeof(name) - 1);
                name[sizeof(name) - 1] = '\0';
                char *dot = strrchr(name, '.');
                if (dot)
                    *dot = '\0';
                cJSON_AddItemToArray(arr, cJSON_CreateString(name));
            }
        } while (FindNextFileA(hFind, &ffd) != 0);
        FindClose(hFind);
    }
    char *out = cJSON_PrintUnformatted(arr);
    cJSON_Delete(arr);
    mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "%s", out ? out : "[]");
    if (out)
        free(out);
}

// Load file content: GET /environment/InputEnvironment/load?name=<filename>
void handle_path_input_environment_load_route(struct mg_connection *c, struct mg_http_message *hm)
{
    struct mg_str name = mg_http_var(hm->query, mg_str("name"));
    if (name.len == 0)
    {
        mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\n", "{\"error\":\"missing name\"}");
        return;
    }
    char fname[256] = {0};
    size_t j = 0;
    for (size_t i = 0; i < name.len && j < sizeof(fname) - 1; i++)
    {
        char ch = name.buf[i];
        if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '-' || ch == '_')
        {
            fname[j++] = ch;
        }
    }
    if (j == 0)
    {
        mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\n", "{\"error\":\"invalid name\"}");
        return;
    }
    char path[512];
    snprintf(path, sizeof(path), "../../save_files/%s.json", fname);
    FILE *f = fopen(path, "rb");
    if (!f)
    {
        mg_http_reply(c, 404, "Access-Control-Allow-Origin: *\r\n", "{\"error\":\"not found\"}");
        return;
    }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = (char *)malloc(sz + 1);
    if (!buf)
    {
        fclose(f);
        mg_http_reply(c, 500, "Access-Control-Allow-Origin: *\r\n", "{\"error\":\"OOM\"}");
        return;
    }
    fread(buf, 1, sz, f);
    fclose(f);
    buf[sz] = '\0';
    mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "%s", buf);
    free(buf);
}

// Delete file: POST /environment/InputEnvironment/delete?name=<filename>
void handle_path_input_environment_delete_route(struct mg_connection *c, struct mg_http_message *hm)
{
    struct mg_str name = mg_http_var(hm->query, mg_str("name"));
    if (name.len == 0)
    {
        mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\n", "{\"error\":\"missing name\"}");
        return;
    }
    char fname[256] = {0};
    size_t j = 0;
    for (size_t i = 0; i < name.len && j < sizeof(fname) - 1; i++)
    {
        char ch = name.buf[i];
        if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '-' || ch == '_')
        {
            fname[j++] = ch;
        }
    }
    if (j == 0)
    {
        mg_http_reply(c, 400, "Access-Control-Allow-Origin: *\r\n", "{\"error\":\"invalid name\"}");
        return;
    }
    char path[512];
    snprintf(path, sizeof(path), "../../save_files/%s.json", fname);
    int rc = remove(path);
    if (rc == 0)
    {
        mg_http_reply(c, 200, "Access-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n", "{\"status\":\"ok\"}");
    }
    else
    {
        mg_http_reply(c, 404, "Access-Control-Allow-Origin: *\r\n", "{\"error\":\"not found\"}");
    }
}
