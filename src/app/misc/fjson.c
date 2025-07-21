#include <stdio.h>
#include "../dependencies/cJSON/cJSON.h"

int main()
{
    cJSON *json = cJSON_Parse("{\"msg\":\"Hello, world!\"}");
    const cJSON *msg = cJSON_GetObjectItemCaseSensitive(json, "msg");
    if (cJSON_IsString(msg) && (msg->valuestring != NULL)) {
        printf("msg: %s\n", msg->valuestring);
    }
    cJSON_Delete(json);
    return 0;
}