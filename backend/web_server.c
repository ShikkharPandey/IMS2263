/* Inventory Management Web Server in C (Using Mongoose) */

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mongoose.h"

#define INVENTORY_FILE "inventory.json"

// Compare mg_str and C string
int mg_strcmp_cstr(struct mg_str s, const char *str) {
    return (s.len == strlen(str) && strncmp(s.buf, str, s.len) == 0);
}

// Handle inventory.json GET and POST
static void handle_inventory(struct mg_connection *c, void *ev_data) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;

    if (mg_strcmp_cstr(hm->method, "GET")) {
        FILE *file = fopen(INVENTORY_FILE, "r");
        if (!file) {
            perror("Failed to open inventory file");
            mg_http_reply(c, 500, "Content-Type: application/json\r\n", "[]");
            return;
        }

        char buffer[4096];
        size_t bytesRead = fread(buffer, 1, sizeof(buffer) - 1, file);
        fclose(file);
        buffer[bytesRead] = '\0';

        mg_http_reply(c, 200, "Content-Type: application/json\r\n", "%s", buffer);
    }

    else if (mg_strcmp_cstr(hm->method, "POST")) {
        FILE *file = fopen(INVENTORY_FILE, "r");
        char buffer[4096];
        size_t bytesRead = fread(buffer, 1, sizeof(buffer) - 1, file);
        fclose(file);
        buffer[bytesRead] = '\0';

        // Step 1: Find last used ID
        int last_id = 0;
        char *ptr = buffer;
        while ((ptr = strstr(ptr, "\"id\":")) != NULL) {
            int id;
            if (sscanf(ptr, "\"id\":%d", &id) == 1 && id > last_id) {
                last_id = id;
            }
            ptr++;
        }

        int new_id = last_id + 1;

        // Step 2: Format new item
        char new_item[512];
        char *body_start = strchr(hm->body.buf, '{');
        if (!body_start) {
            mg_http_reply(c, 400, "Content-Type: text/plain\r\n", "Invalid request body");
            return;
        }

        snprintf(new_item, sizeof(new_item), "{\"id\":%d,%s", new_id, body_start + 1);

        // Step 3: Append to JSON array
        if (bytesRead <= 2) {
            strcpy(buffer, "[");
        } else {
            buffer[bytesRead - 1] = ',';
        }

        strcat(buffer, new_item);
        strcat(buffer, "]");

        // Step 4: Save updated file
        file = fopen(INVENTORY_FILE, "w");
        fwrite(buffer, 1, strlen(buffer), file);
        fclose(file);

        mg_http_reply(c, 200, "Content-Type: application/json\r\n", "%s", buffer);
    }
}

// Route handling
static void fn(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        struct mg_http_serve_opts opts = {.root_dir = "../frontend"};

        if (mg_strcmp_cstr(hm->uri, "/inventory")) {
            handle_inventory(c, ev_data);
        }
        else if (mg_strcmp_cstr(hm->uri, "/")) {
            mg_http_serve_file(c, hm, "../frontend/index.html", &opts);
        }
        else if (mg_strcmp_cstr(hm->uri, "/style.css")) {
            mg_http_serve_file(c, hm, "../frontend/style.css", &opts);
        }
        else if (mg_strcmp_cstr(hm->uri, "/app.js")) {
            mg_http_serve_file(c, hm, "../frontend/app.js", &opts);
        }
        else {
            mg_http_reply(c, 404, "", "Not Found");
        }
    }
}

// Entry point
int main() {
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, "http://localhost:8000", fn, NULL);

    printf("Server running at http://localhost:8000\n");
    for (;;) mg_mgr_poll(&mgr, 1000);

    mg_mgr_free(&mgr);
    return 0;
}
