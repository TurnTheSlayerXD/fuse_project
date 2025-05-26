#ifndef BUFFER_H
#define BUFFER_H

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef struct
{
    char *ptr;
    size_t capacity;
    size_t size;
} buffer;

#define FILE_ID_SIZE 100
#define FILE_NAME_SIZE 100
typedef struct
{
    char file_id[FILE_ID_SIZE];
    char file_name[FILE_NAME_SIZE];
} tg_file;

buffer buffer_new()
{
    return (buffer){.ptr = malloc(100), .size = 0, .capacity = 100};
}

buffer buffer_free(buffer *buf)
{
    free(buf->ptr);
    memset(buf, 0, sizeof(buffer));
}

void buffer_push(buffer *st, char c)
{
    if (st->size >= st->capacity)
    {
        st->capacity = st->capacity * 2 + 1;
        st->ptr = realloc(st->ptr, st->capacity);
    }
    st->ptr[st->size++] = c;
}

void buffer_append(buffer *st, void *data, size_t size)
{
    if (st->size + size >= st->capacity)
    {
        st->capacity += size;
        st->ptr = realloc(st->ptr, st->capacity);
    }
    memcpy(st->ptr + st->size, data, size);
    st->size += size;
}

void buffer_insert(buffer *st, size_t pos, void *data, size_t size)
{

    assert(false && "NOT IMPLEMENTED");
}

void parse_response_field_from_json(char *dst, buffer *src, const char *field)
{
    const char *p = strstr(src->ptr, field);
    p += strlen(field) + 2;

    bool is_quoted = false;
    is_quoted = *p == '\"';

    if (is_quoted)
    {
        strncpy(dst, p + 1, strchr(p, ',') - 1 - p);
    }
    else
    {
        strncpy(dst, p, strchr(p, ',') - p);
    }
}

#endif // BUFFER_H
