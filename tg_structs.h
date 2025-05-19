#ifndef TG_STRUCTS_H
#define TG_STRUCTS_H

#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef struct
{
    const char *path;
    const char *contents;
} tg_file;

typedef struct
{
    tg_file *files;
    int size;
    int cap;
} tg_storage;

tg_storage tg_storage_new()
{
    return (tg_storage){.files = malloc(sizeof(tg_file) * 2), .size = 2, .cap = 2};
}

void tg_storage_free(tg_storage *st)
{
    for (int i = 0; i < st->size; ++i)
    {
        free(st->files[i].path);
        free(st->files[i].contents);
    }
    free(st->files);
    memset(st, 0, sizeof(tg_storage));
}

void tg_storage_push(tg_storage *st, tg_file f)
{
    if (st->size >= st->cap)
    {
        st->cap = st->cap * 2 + 1;
        st->files = realloc(st->files, sizeof(tg_file) * st->cap);
    }
    st->files[st->size++] = f;
}

tg_file tg_storage_pop(tg_storage *st)
{
    if (st->size <= 0)
    {
        assert(false && "ERROR: TRYING TO POP FROM EMPTY FILE STORAGE");
    }

    if (st->size < st->cap / 2)
    {
        st->cap /= 2;
        st->files = realloc(st->files, sizeof(tg_file) * st->cap);
    }

    return st->files[--st->size];
}

tg_file *tg_storage_find_by_path(tg_storage *st, char *path)
{
    for (int i = 0; i < st->size; ++i)
    {
        if (strcmp(st->files[i].path, path) == 0)
        {
            return &st->files[i];
        }
    }
    return NULL;
}

#endif // TG_STRUCTS_H