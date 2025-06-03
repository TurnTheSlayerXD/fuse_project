#ifndef BUFFER_H
#define BUFFER_H

#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef struct
{
    char *ptr;
    size_t capacity;
    size_t size;
} buffer;

buffer buffer_new();

void buffer_free(buffer *buf);
void buffer_push(buffer *st, char c);
void buffer_append(buffer *st, void *data, size_t size);
void buffer_insert(buffer *st, size_t pos, const void *data, size_t size);
int parse_response_field_from_json(char *dst, const char *src, const char *field);
bool check_ok_status_in_json(const char *json_response_buf);
#endif // BUFFER_H
