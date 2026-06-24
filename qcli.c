#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <curl/curl.h>

#define URL "http://localhost:8080/completion"

typedef struct {
    char buf[4096];
    int len;
    int token_count;
    int in_think;
    int first_token_seen;
    struct timespec t0;
    struct timespec tfirst;
} stream_state;

static int process_sse_line(stream_state *s, const char *line) {
    const char *prefix = "data: ";
    if (strncmp(line, prefix, 6) != 0) return 0;

    const char *json = line + 6;
    const char *key = "\"content\":\"";
    const char *start = strstr(json, key);
    if (!start) return 0;

    start += strlen(key);

    /* check for closing quote immediately (empty content) */
    if (*start == '"') return 0;

    /* detect <think> / </think> tags arriving as their own tokens */
    if (strncmp(start, "<think>\"", 8) == 0) {
        s->in_think = 1;
        fprintf(stdout, "\033[2m");
        fflush(stdout);
        return 1;
    }
    if (strncmp(start, "</think>\"", 9) == 0) {
        s->in_think = 0;
        fprintf(stdout, "\033[0m\n");
        fflush(stdout);
        return 1;
    }

    if (!s->first_token_seen) {
        clock_gettime(CLOCK_MONOTONIC, &s->tfirst);
        s->first_token_seen = 1;
    }

    int wrote = 0;
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
        wrote = 1;
        start++;
    }
    if (wrote) fflush(stdout);
    return wrote;
}

static size_t stream_callback(void *ptr, size_t size, size_t nmemb, void *userdata) {
    stream_state *s = userdata;
    size_t total = size * nmemb;
    const char *in = ptr;

    for (size_t i = 0; i < total; i++) {
        if (in[i] == '\n') {
            s->buf[s->len] = '\0';
            if (s->len > 0) s->token_count += process_sse_line(s, s->buf);
            s->len = 0;
        } else if (s->len < (int)sizeof(s->buf) - 1) {
            s->buf[s->len++] = in[i];
        }
    }
    return total;
}

int main(int argc, char **argv) {
    int no_think = 0;
    int prompt_idx = 1;

    if (argc >= 2 && strcmp(argv[1], "--no-think") == 0) {
        no_think = 1;
        prompt_idx = 2;
    }

    if (argc < prompt_idx + 1) {
        fprintf(stderr, "usage: qcli [--no-think] \"prompt\"\n");
        return 1;
    }

    CURL *curl = curl_easy_init();
    if (!curl) return 1;

    stream_state s = {0};
    clock_gettime(CLOCK_MONOTONIC, &s.t0);

    char json[4096];
    snprintf(json, sizeof(json),
        "{\"prompt\":\"<|im_start|>user\\n%s%s<|im_end|>\\n<|im_start|>assistant\\n\","
        "\"n_predict\":2048,\"stream\":true,\"stop\":[\"<|im_end|>\"]}",
        argv[prompt_idx],
        no_think ? " /no_think" : ""
    );

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, URL);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, stream_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);

    struct timespec t1;
    clock_gettime(CLOCK_MONOTONIC, &t1);

    /* ensure color is reset if response was interrupted mid-think */
    if (s.in_think) fprintf(stdout, "\033[0m");

    if (res != CURLE_OK) {
        fprintf(stderr, "curl error\n");
    } else {
        printf("\n");
        if (s.token_count > 0) {
            double total   = (t1.tv_sec      - s.t0.tv_sec)     + (t1.tv_nsec      - s.t0.tv_nsec)     / 1e9;
            double ttft    = (s.tfirst.tv_sec - s.t0.tv_sec)     + (s.tfirst.tv_nsec - s.t0.tv_nsec)    / 1e9;
            double tps     = s.token_count / total;
            fprintf(stderr, "[%d tok | TTFT %.2fs | %.1f t/s]\n", s.token_count, ttft, tps);
        }
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return 0;
}
