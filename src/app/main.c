#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "webserver.h"

int main()
{
    struct mg_mgr mgr;
    
    webserver_init(&mgr, "http://localhost:8000");
    
    for (;;) mg_mgr_poll(&mgr, 1000);
    
    mg_mgr_free(&mgr);
    return 0;
}

