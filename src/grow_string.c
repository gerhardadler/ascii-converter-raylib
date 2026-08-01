#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char* data;
    size_t len;  // current string length (not including '\0')
    size_t cap;  // allocated capacity
} String;

void string_init(String* s, size_t initial_cap) {
    s->data = malloc(initial_cap);
    s->data[0] = '\0';
    s->len = 0;
    s->cap = initial_cap;
}

void string_append(String* s, const char* text) {
    size_t text_len = strlen(text);
    size_t needed = s->len + text_len + 1;  // +1 for '\0'

    if (needed > s->cap) {
        size_t new_cap = s->cap * 2;
        while (new_cap < needed) new_cap *= 2;
        char* new_data = realloc(s->data, new_cap);
        if (!new_data) {
            // handle allocation failure
            return;
        }
        s->data = new_data;
        s->cap = new_cap;
    }

    memcpy(s->data + s->len, text, text_len + 1);  // +1 copies the '\0' too
    s->len += text_len;
}

void string_free(String* s) {
    free(s->data);
    s->data = NULL;
    s->len = s->cap = 0;
}

void string_append_fmt(String* s, const char* fmt, ...) {
    va_list args;

    // First pass: figure out how much space we need
    va_start(args, fmt);
    int needed_len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    if (needed_len < 0) return;  // encoding error

    size_t needed = s->len + (size_t)needed_len + 1;  // +1 for '\0'

    if (needed > s->cap) {
        size_t new_cap = s->cap ? s->cap * 2 : 64;
        while (new_cap < needed) new_cap *= 2;
        char* new_data = realloc(s->data, new_cap);
        if (!new_data) return;
        s->data = new_data;
        s->cap = new_cap;
    }

    // Second pass: actually write into the buffer
    va_start(args, fmt);
    vsnprintf(s->data + s->len, (size_t)needed_len + 1, fmt, args);
    va_end(args);

    s->len += (size_t)needed_len;
}

void string_append_char(String* s, char c) {
    size_t needed = s->len + 2;  // +1 for c, +1 for '\0'

    if (needed > s->cap) {
        size_t new_cap = s->cap ? s->cap * 2 : 64;
        while (new_cap < needed) new_cap *= 2;
        char* new_data = realloc(s->data, new_cap);
        if (!new_data) return;
        s->data = new_data;
        s->cap = new_cap;
    }

    s->data[s->len] = c;
    s->len++;
    s->data[s->len] = '\0';
}

int string_write_to_file(String* s, const char* filename) {
    FILE* f = fopen(filename, "wb");
    if (!f) return -1;

    size_t written = fwrite(s->data, 1, s->len, f);
    int close_result = fclose(f);

    if (written != s->len || close_result != 0) return -1;
    return 0;
}
