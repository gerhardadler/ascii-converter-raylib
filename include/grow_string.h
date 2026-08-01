#include <stdlib.h>
#define _POSIX_C_SOURCE 200809L

typedef struct {
    char* data;
    size_t len;  // current string length (not including '\0')
    size_t cap;  // allocated capacity
} String;

void string_init(String* s, size_t initial_cap);
void string_append(String* s, const char* text);
void string_free(String* s);
void string_append_fmt(String* s, const char* fmt, ...);
void string_append_char(String* s, char c);
int string_write_to_file(String* s, const char* filename);
