#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mongoose.h"

// Assuming the inventory file is a simple JSON array format
#define INVENTORY_FILE "inventory.json"

// A basic struct to hold product information
struct Product {
    int id;
    char name[100];
    int quantity;
    float price;
    char upc[100];        
    char sku[100];        
    char department[100]; 
    char vendor[100];     
};


// Function to parse a JSON string and extract product information
int parse_inventory(const char *json_data, struct Product *products, int max_products) {
    int count = 0;
    const char *ptr = json_data;

    // Loop through and manually parse the products from the JSON-like string
    while (ptr && count < max_products) {
        // Look for the opening bracket for each product entry
        ptr = strstr(ptr, "{\"id\":");
        if (!ptr) break;

        // Extract the id value
        int id;
        if (sscanf(ptr, "{\"id\":%d", &id) != 1) {
            ptr++;
            continue;  // If we can't find id, move on to the next
        }

            products[count].id = id;

        // Extract the name field (we expect the name to be a string)
        char name[100];
        if (sscanf(ptr, "{\"id\":%d,\"name\":\"%99[^\"]\"", &id, name) == 2) {
            strncpy(products[count].name, name, sizeof(products[count].name));
        }

        // Extract the quantity and price (you can extend this as needed)
        int quantity;
        float price;
        sscanf(ptr, "\"quantity\":%d,\"price\":%f", &quantity, &price);
        products[count].quantity = quantity;
        products[count].price = price;

        count++;
        ptr++;  // Move the pointer to the next product
    }

    return count;  // Return the number of products parsed
}

// Function to handle search requests
static void handle_search_request(struct mg_connection *c, void *ev_data) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;

    // Extract the search query
    char query[256];
    mg_http_get_var(&hm->query, "query", query, sizeof(query));
    printf("Search Query: %s\n", query);  // Log the query to check if it's correct

    // Open the inventory file
    FILE *file = fopen(INVENTORY_FILE, "r");
    if (!file) {
        mg_http_reply(c, 500, "Content-Type: application/json\r\n", "[]");
        return;
    }

    // Read the inventory file into a buffer
    char buffer[8192];
    size_t bytesRead = fread(buffer, 1, sizeof(buffer) - 1, file);
    buffer[bytesRead] = '\0';
    fclose(file);

    // Parse inventory
    struct Product products[100];
    int product_count = parse_inventory(buffer, products, sizeof(products) / sizeof(products[0]));

    // Search for matching products
    struct Product matching_products[100];
    int match_count = 0;
    for (int i = 0; i < product_count; i++) {
        // Convert both the product name and query to lowercase for case-insensitive search
        char product_name[100];
        strncpy(product_name, products[i].name, sizeof(product_name));
        for (int j = 0; product_name[j]; j++) {
            product_name[j] = tolower(product_name[j]);
        }

        char search_query[256];
        strncpy(search_query, query, sizeof(search_query));
        for (int j = 0; search_query[j]; j++) {
            search_query[j] = tolower(search_query[j]);
        }

        // If the search query is found in the product name (case-insensitive)
        if (strstr(product_name, search_query)) {
            matching_products[match_count++] = products[i];
        }
    }

    // Create the response JSON with all fields including upc, sku, department, vendor
    char response[8192];
    snprintf(response, sizeof(response), "[");
    for (int i = 0; i < match_count; i++) {
        if (i > 0) strcat(response, ", ");
        snprintf(response + strlen(response), sizeof(response) - strlen(response), 
            "{\"id\":%d, \"name\":\"%s\", \"quantity\":%d, \"price\":%.2f, \"upc\":\"%s\", \"sku\":\"%s\", \"department\":\"%s\", \"vendor\":\"%s\"}",
            matching_products[i].id, matching_products[i].name,
            matching_products[i].quantity, matching_products[i].price,
            matching_products[i].upc, matching_products[i].sku,
            matching_products[i].department, matching_products[i].vendor);
    }
    strcat(response, "]");

    // Send the JSON response
    mg_http_reply(c, 200, "Content-Type: application/json\r\n", "%s", response);
}




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

        // Check the request URI and serve the appropriate content
        if (mg_strcmp_cstr(hm->uri, "/") || mg_strcmp_cstr(hm->uri, "/index.html")) {
            mg_http_serve_file(c, hm, "../frontend/index.html", &opts);
        } else if (mg_strcmp_cstr(hm->uri, "/inventory")) {
            handle_inventory(c, ev_data); 
        } else if (mg_strcmp_cstr(hm->uri, "/search")) {
            handle_search_request(c, ev_data);  // Handle search requests
        } else if (mg_strcmp_cstr(hm->uri, "/search.html")) {
            mg_http_serve_file(c, hm, "../frontend/search.html", &opts);  // Serve search.html page
        }else if (mg_strcmp_cstr(hm->uri, "/search.js")) {
            mg_http_serve_file(c, hm, "../frontend/search.js", &opts); // Correct path to search.js
        }else if (mg_strcmp_cstr(hm->uri, "/style.css")) {
            mg_http_serve_file(c, hm, "../frontend/style.css", &opts);
        } else if (mg_strcmp_cstr(hm->uri, "/app.js")) {
            mg_http_serve_file(c, hm, "../frontend/app.js", &opts);
        } else if (mg_strcmp_cstr(hm->uri, "/add.js")) {
            mg_http_serve_file(c, hm, "../frontend/add.js", &opts);
        } else if (mg_strcmp_cstr(hm->uri, "/add.html")) {
            mg_http_serve_file(c, hm, "../frontend/add.html", &opts);
        } else {
            mg_http_reply(c, 404, "Content-Type: text/plain\r\n", "You've reached somewhere you shouldn't be\n");
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