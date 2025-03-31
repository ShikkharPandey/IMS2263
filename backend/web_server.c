#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mongoose.h"

#define INVENTORY_FILE "inventory.json"

int mg_strcmp_cstr(struct mg_str s, const char *str) {
    return (s.len == strlen(str) && strncmp(s.buf, str, s.len) == 0);
}

void clean_trailing_whitespace(char *str) {
    char *p = str + strlen(str) - 1;
    while (p > str && (*p == '\n' || *p == '\r' || *p == ' ' || *p == '\t')) *p-- = '\0';
}

static void handle_inventory(struct mg_connection *c, void *ev_data) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;

    if (mg_strcmp_cstr(hm->method, "GET")) {
        FILE *file = fopen(INVENTORY_FILE, "r");
        if (!file) {
            mg_http_reply(c, 200, "Content-Type: application/json\r\n", "[]");
            return;
        }
        char buffer[8192];
        size_t bytesRead = fread(buffer, 1, sizeof(buffer) - 1, file);
        buffer[bytesRead] = '\0';
        fclose(file);
        mg_http_reply(c, 200, "Content-Type: application/json\r\n", "%s", buffer);
    }

    else if (mg_strcmp_cstr(hm->method, "POST")) {
        FILE *file = fopen(INVENTORY_FILE, "r");
        char buffer[8192] = "[]"; // Default to empty array if file is empty
        if (file) {
            fread(buffer, 1, sizeof(buffer) - 1, file);
            fclose(file);
        }
    
        // Get the current highest ID
        int max_id = 0;
        char *ptr = buffer;
        while ((ptr = strstr(ptr, "\"id\":")) != NULL) {
            int id;
            if (sscanf(ptr, "\"id\":%d", &id) == 1 && id > max_id) max_id = id;
            ptr++;
        }
    
        int new_id = max_id + 1;
        char new_item[1024];
        const char *json_body = hm->body.buf;
    
        // Construct new item with the correct JSON format
        snprintf(new_item, sizeof(new_item), "{\"id\":%d,%s", new_id, json_body + 1); // Assumes json_body starts with '{'
        clean_trailing_whitespace(new_item);
    
        // Fix trailing commas and ensure proper array format
        char *end = strrchr(buffer, ']');
        if (!end) {
            strcpy(buffer, "[]");
            end = buffer + 1;
        }

        *end = '\0';  // Remove the last ']' to safely append the new item
    
        // Build the final updated array, ensuring proper JSON format
        char updated[8192];
        snprintf(updated, sizeof(updated), "%s,%s]", buffer, new_item); // Append the new item and close the array
    
        // Save the updated inventory to the file
        file = fopen(INVENTORY_FILE, "w");
        fwrite(updated, 1, strlen(updated), file); // Writing the correctly formatted JSON back to the file
        fclose(file);
    
        mg_http_reply(c, 200, "Content-Type: application/json\r\n", "%s", updated);
    }
    
    
    
    
    
    else if (mg_strcmp_cstr(hm->method, "PUT")) {
        int update_id;
        char name[128], upc[64], sku[64], dept[64], vendor[64];
        int quantity;
        float price;

        if (sscanf(hm->body.buf,
            "{\"id\":%d,\"name\":\"%127[^\"]\",\"quantity\":%d,\"price\":%f,"
            "\"upc\":\"%63[^\"]\",\"sku\":\"%63[^\"]\","
            "\"department\":\"%63[^\"]\",\"vendor\":\"%63[^\"]\"}",
            &update_id, name, &quantity, &price, upc, sku, dept, vendor) != 8) {
            mg_http_reply(c, 400, "", "Invalid product format");
            return;
        }

        FILE *file = fopen(INVENTORY_FILE, "r");
        if (!file) {
            mg_http_reply(c, 500, "", "Unable to open inventory file");
            return;
        }

        char buffer[8192];
        fread(buffer, 1, sizeof(buffer) - 1, file);
        fclose(file);

        char result[8192] = "[";
        int first = 1;
        char *start = buffer;

        while ((start = strstr(start, "{\"id\":")) != NULL) {
            char *end = strchr(start, '}');
            if (!end) break;
            end++;

            char item[1024] = {0};
            strncpy(item, start, end - start);

            int id = -1;
            sscanf(item, "{\"id\":%d", &id);

            if (!first) strcat(result, ",");

            if (id == update_id) {
                char updated[1024];
                snprintf(updated, sizeof(updated),
                    "{\"id\":%d,\"name\":\"%s\",\"quantity\":%d,\"price\":%.2f,"
                    "\"upc\":\"%s\",\"sku\":\"%s\",\"department\":\"%s\",\"vendor\":\"%s\"}",
                    id, name, quantity, price, upc, sku, dept, vendor);
                strcat(result, updated);
            } else {
                strcat(result, item);
            }

            first = 0;
            start = end;
        }

        strcat(result, "]");

        file = fopen(INVENTORY_FILE, "w");
        fwrite(result, 1, strlen(result), file);
        fclose(file);

        mg_http_reply(c, 200, "Content-Type: application/json\r\n", "%s", result);
    }

    else if (mg_strcmp_cstr(hm->method, "DELETE")) {
        char id_param[32];
        mg_http_get_var(&hm->query, "id", id_param, sizeof(id_param));
        int delete_id = atoi(id_param);

        FILE *file = fopen(INVENTORY_FILE, "r");
        if (!file) {
            mg_http_reply(c, 500, "", "[]");
            return;
        }

        char buffer[8192];
        fread(buffer, 1, sizeof(buffer) - 1, file);
        fclose(file);

        char result[8192] = "[";
        int first = 1;
        char *start = buffer;

        while ((start = strstr(start, "{\"id\":")) != NULL) {
            char *end = strchr(start, '}');
            if (!end) break;
            end++;

            char item[512] = {0};
            strncpy(item, start, end - start);

            int id = -1;
            sscanf(item, "{\"id\":%d", &id);

            if (id != delete_id) {
                if (!first) strcat(result, ",");
                strcat(result, item);
                first = 0;
            }

            start = end;
        }

        strcat(result, "]");

        file = fopen(INVENTORY_FILE, "w");
        fwrite(result, 1, strlen(result), file);
        fclose(file);

        mg_http_reply(c, 200, "Content-Type: application/json\r\n", "%s", result);
    }
}

static void fn(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        struct mg_http_serve_opts opts = {.root_dir = "../frontend"};

        if (mg_strcmp_cstr(hm->uri, "/") || mg_strcmp_cstr(hm->uri, "/index.html")) {
            mg_http_serve_file(c, hm, "../frontend/index.html", &opts);
        } else if (mg_strcmp_cstr(hm->uri, "/inventory")) {
            handle_inventory(c, ev_data);
        } else if (mg_strcmp_cstr(hm->uri, "/style.css")) {
            mg_http_serve_file(c, hm, "../frontend/style.css", &opts);
        } else if (mg_strcmp_cstr(hm->uri, "/app.js")) {
            mg_http_serve_file(c, hm, "../frontend/app.js", &opts);
        } else if (mg_strcmp_cstr(hm->uri, "/add.js")) {
            mg_http_serve_file(c, hm, "../frontend/add.js", &opts);
        } else if (mg_strcmp_cstr(hm->uri, "/add.html")) {
            mg_http_serve_file(c, hm, "../frontend/add.html", &opts);
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