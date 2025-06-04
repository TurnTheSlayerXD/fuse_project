
#include <tg_storage.h>

tg_storage tg_storage_new()
{
    return (tg_storage){.files = (tg_file *)malloc(sizeof(tg_file) * 2), .size = 0, .cap = 2};
}

void tg_storage_free(tg_storage *st)
{
    free(st->files);
    memset(st, 0, sizeof(tg_storage));
}

void tg_storage_push(tg_storage *st, tg_file f)
{
    if (st->size >= st->cap)
    {
        st->cap = st->cap * 2 + 1;
        st->files = (tg_file *)realloc(st->files, sizeof(tg_file) * st->cap);
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
        st->files = (tg_file *)realloc(st->files, sizeof(tg_file) * st->cap);
    }

    return st->files[--st->size];
}

tg_file *tg_storage_find_by_path(tg_storage *st, const char *path)
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

tg_file tg_storage_remove_by_path(tg_storage *st, const char *path)
{
    if (st->size == 0)
    {
        return (tg_file){0};
    }

    for (int i = 0; i < st->size; ++i)
    {
        if (strcmp(st->files[i].path, path) == 0)
        {
            tg_file file = st->files[i];
            st->files[i] = st->files[--st->size];
            return file;
        }
    }

    return (tg_file){0};
}

int tg_storage_find_files_by_dir(tg_storage *st, const char *dir, int to_search_from)
{
    for (int i = to_search_from; i < st->size; ++i)
    {
        if (strcmp(st->files[i].path, dir) != 0)
        {
            char file_dir[100];
            tg_file_extract_directory(file_dir, &st->files[i]);
            fuse_log(FUSE_LOG_DEBUG, "tg_storage_find_files_by_dir [%s]\n", file_dir);
            if (strcmp(file_dir, dir) == 0)
            {
                return i;
            }
        }
    }

    return -1;
}
