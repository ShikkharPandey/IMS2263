/* Inventory Management Web Server in C (Using Mongoose) */

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mongoose.h"

#define INVENTORY_FILE "backend/inventory.json"

// Function to compare mg_str (string) with a C string
int mg_strcmp_cstr(struct mg_str s, const char *str) {
    return (s.len == strlen(str) && strncmp(s.buf, str, s.len) == 0);
}

// Function to serve the main web page
static void serve_index(struct mg_connection *c, struct mg_http_message *hm) {
    struct mg_http_serve_opts opts = {0};
    opts.root_dir = "frontend";  // Set root directory for serving files
    mg_http_serve_file(c, hm, "frontend/index.html", &opts);
}

// Function to serve CSS and JavaScript files
static void serve_static(struct mg_connection *c, struct mg_http_message *hm) {
    struct mg_http_serve_opts opts = {0};
    opts.root_dir = "frontend";

    if (mg_strcmp_cstr(hm->uri, "/style.css")) {
        mg_http_serve_file(c, hm, "frontend/style.css", &opts);
    } else if (mg_strcmp_cstr(hm->uri, "/app.js")) {
        mg_http_serve_file(c, hm, "frontend/app.js", &opts);
    }
}

// Function to read inventory.json and return it as a JSON response
static void handle_inventory(struct mg_connection *c, void *ev_data) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;

    if (mg_strcmp_cstr(hm->method, "GET")) {
        // Handle GET request - return inventory list
        FILE *file = fopen(INVENTORY_FILE, "r");
        if (!file) {
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
        // Handle POST request - add new inventory item
        FILE *file = fopen(INVENTORY_FILE, "r");
        char buffer[4096];
        size_t bytesRead = fread(buffer, 1, sizeof(buffer) - 1, file);
        fclose(file);
        buffer[bytesRead] = '\0';

        // 🔹 Step 1: Find the Highest Existing ID
        int last_id = 0;
        char *ptr = buffer;
        while ((ptr = strstr(ptr, "\"id\":")) != NULL) { // Find each occurrence of "id"
            int id;
            if (sscanf(ptr, "\"id\":%d", &id) == 1) {
                if (id > last_id) last_id = id;  // Keep track of the highest ID
            }
            ptr++; // Move pointer forward
        }

        // 🔹 Step 2: Generate a New ID Starting from the Last ID in JSON
        int new_id = last_id + 1;

        // 🔹 Step 3: Format the New Item with the Correct ID
        char new_item[512];
        // Extract body content without an "id" field
char *body_start = strchr(hm->body.buf, '{'); // Find where JSON starts
if (!body_start) {
    mg_http_reply(c, 400, "Content-Type: text/plain\r\n", "Invalid request body");
    return;
}

// Format new item correctly, ensuring only ONE "id" field
snprintf(new_item, sizeof(new_item), "{\"id\":%d,%s", new_id, body_start + 1);

        // 🔹 Step 4: Ensure JSON Starts Correctly
        if (bytesRead <= 2) {
            strcpy(buffer, "[");  // If the file is empty or just has brackets
        } else {
            buffer[bytesRead - 1] = ',';  // Replace closing `]` with `,`
        }

        strcat(buffer, new_item);
        strcat(buffer, "]");

        // 🔹 Step 5: Save Updated JSON
        file = fopen(INVENTORY_FILE, "w");
        fwrite(buffer, 1, strlen(buffer), file);
        fclose(file);

        mg_http_reply(c, 200, "Content-Type: application/json\r\n", "%s", buffer);
    }
}



// Function to handle HTTP requests
static void fn(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        struct mg_http_serve_opts opts = {.root_dir = "frontend"};  // Set frontend folder

        if (mg_strcmp_cstr(hm->uri, "/")) {
            mg_http_serve_file(c, hm, "frontend/index.html", &opts);
        } else if (mg_strcmp_cstr(hm->uri, "/style.css")) {
            mg_http_serve_file(c, hm, "frontend/style.css", &opts);
        } else if (mg_strcmp_cstr(hm->uri, "/app.js")) {
            mg_http_serve_file(c, hm, "frontend/app.js", &opts);
        } else if (mg_strcmp_cstr(hm->uri, "/inventory")) {
            handle_inventory(c, ev_data);
        } else {
            mg_http_reply(c, 404, "Content-Type: text/plain\r\n", "Not Found");
        }
    }
}


int main() {
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, "http://localhost:8000", fn, NULL);

    printf("Server running at http://localhost:8000\n");
    for (;;) mg_mgr_poll(&mgr, 1000);

    mg_mgr_free(&mgr);
    return 0;
}