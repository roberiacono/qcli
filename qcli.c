#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#define URL "http://localhost:8080/completion"

struct response {
    char *data;
    size_t size;
};



static size_t write_callback(void *ptr, size_t size, size_t nmemb, struct response *r) {
    size_t total = size * nmemb;
    r->data = realloc(r->data, r->size + total + 1);
    memcpy(r->data + r->size, ptr, total);
    r->size += total;
    r->data[r->size] = '\0';
    return total;
}

char *extract_content(const char *json) {
    const char *key = "\"content\":\"";
    char *start = strstr(json, key);
    if (!start) return NULL;

    start += strlen(key);

    char *out = malloc(strlen(json));
    int i = 0;

    while (*start) {
        if (*start == '"' && *(start - 1) != '\\') break;

        if (*start == '\\' && *(start + 1)) {
            start++;
            if (*start == 'n') out[i++] = '\n';
            else if (*start == 't') out[i++] = '\t';
            else if (*start == '"') out[i++] = '"';
            else out[i++] = *start;
        } else {
            out[i++] = *start;
        }

        start++;
    }

    out[i] = '\0';
    return out;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: qcli \"prompt\"\n");
        return 1;
    }

    CURL *curl = curl_easy_init();
    if (!curl) return 1;

    struct response r = {0};

    char json[4096];
    snprintf(json, sizeof(json),
        "{\"prompt\":\"%s\",\"n_predict\":256,\"stream\":false}",
        argv[1]
    );

    curl_easy_setopt(curl, CURLOPT_URL, URL);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &r);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);

    if (res == CURLE_OK) {
       char *content = extract_content(r.data);

        if (content) {
            printf("%s\n", content);
            free(content);
        } else {
            fprintf(stderr, "parse error\n");
        }
    } else {
        fprintf(stderr, "curl error\n");
    }

    curl_easy_cleanup(curl);
    free(r.data);

    return 0;
}
