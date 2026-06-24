#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#define URL "http://localhost:8080/completion"

typedef struct {
    char buf[4096];
    int len;
} stream_state;

static void process_sse_line(const char *line) {
    const char *prefix = "data: ";
    if (strncmp(line, prefix, 6) != 0) return;

    const char *json = line + 6;
    const char *key = "\"content\":\"";
    const char *start = strstr(json, key);
    if (!start) return;

    start += strlen(key);

    while (*start) {
        if (*start == '"' && *(start - 1) != '\\') break;

        if (*start == '\\' && *(start + 1)) {
            start++;
            if (*start == 'n') fputc('\n', stdout);
            else if (*start == 't') fputc('\t', stdout);
            else if (*start == '"') fputc('"', stdout);
            else fputc(*start, stdout);
        } else {
            fputc(*start, stdout);
        }
        start++;
    }
    fflush(stdout);
}

static size_t stream_callback(void *ptr, size_t size, size_t nmemb, void *userdata) {
    stream_state *s = userdata;
    size_t total = size * nmemb;
    const char *in = ptr;

    for (size_t i = 0; i < total; i++) {
        if (in[i] == '\n') {
            s->buf[s->len] = '\0';
            if (s->len > 0) process_sse_line(s->buf);
            s->len = 0;
        } else if (s->len < (int)sizeof(s->buf) - 1) {
            s->buf[s->len++] = in[i];
        }
    }
    return total;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: qcli \"prompt\"\n");
        return 1;
    }

    CURL *curl = curl_easy_init();
    if (!curl) return 1;

    stream_state s = {0};

    char json[4096];
    snprintf(json, sizeof(json),
        "{\"prompt\":\"%s\",\"n_predict\":256,\"stream\":true}",
        argv[1]
    );

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, URL);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, stream_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK)
        fprintf(stderr, "curl error\n");
    else
        printf("\n");

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return 0;
}
