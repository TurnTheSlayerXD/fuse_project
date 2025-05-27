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

void buffer_insert(buffer *st, size_t pos, const void *data, size_t size)
{

    if (pos + size >= st->capacity)
    {
        st->capacity = pos + size;
        st->ptr = realloc(st->ptr, st->capacity);
    }
    memcpy(st->ptr + pos, data, size);

    if (st->size < pos + size)
    {
        st->size = pos + size;
    }
}

int parse_response_field_from_json(char *dst, const char *src, const char *field)
{

    const char *p = strstr(src, field);
    p += strlen(field) + 2;

    bool is_quoted = *p == '\"';

    if (is_quoted)
    {
        char *e = strpbrk(p, ",}");
        if (!e)
        {
            strcpy(dst, "ERROR PARSING JSON");
        }
        else
        {
            strncpy(dst, p + 1, e - p - 2);
        }
    }
    else
    {
        char *e = strpbrk(p, ",}");
        if (!e)
        {
            strcpy(dst, "ERROR PARSING JSON");
        }
        else
        {
            strncpy(dst, p, e - p);
        }
    }
}

bool check_ok_status_in_json(const char *json_response_buf)
{
    char is_ok[100];
    memset(is_ok, 0, 100);
    parse_response_field_from_json(is_ok, json_response_buf, "ok");

    fuse_log(FUSE_LOG_DEBUG, "%s\n", is_ok);
    if (strcmp(is_ok, "true") == 0)
    {
        return true;
    }
    return false;
}

#endif // BUFFER_H
