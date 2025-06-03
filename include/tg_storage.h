
#ifndef TG_STORAGE_H

#define TG_STORAGE_H

#include <tg_file.h>
#include <stdlib.h>

typedef struct
{
    tg_file *files;
    int size;
    int cap;
} tg_storage;
tg_storage tg_storage_new();
void tg_storage_free(tg_storage *st);
void tg_storage_push(tg_storage *st, tg_file f);
tg_file tg_storage_pop(tg_storage *st);
tg_file *tg_storage_find_by_path(tg_storage *st, const char *path);
tg_file tg_storage_remove_by_path(tg_storage *st, const char *path);
int tg_storage_find_files_by_dir(tg_storage *st, const char *dir, int to_search_from);
#endif // TG_STORAGE_H