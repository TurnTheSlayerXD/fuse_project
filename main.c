#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include "tg_structs.h"

static tg_storage storage;

static void *hello_init(struct fuse_conn_info *conn,
                        struct fuse_config *cfg)
{
    (void)conn;
    cfg->kernel_cache = 1;

    /* Test setting flags the old way */
    fuse_set_feature_flag(conn, FUSE_CAP_ASYNC_READ);
    fuse_unset_feature_flag(conn, FUSE_CAP_ASYNC_READ);

    return NULL;
}

static int hello_getattr(const char *path, struct stat *stbuf,
                         struct fuse_file_info *fi)
{
    (void)fi;
    int res = 0;

    tg_file *file;

    memset(stbuf, 0, sizeof(struct stat));
    if (strcmp(path, "/") == 0)
    {
        stbuf->st_mode = S_IFDIR | 0777;
        stbuf->st_nlink = 2;
    }
    else if ((file = tg_storage_find_by_path(&storage, path)))
    {
        if (file->type == TG_FILE)
        {
            stbuf->st_mode = S_IFREG | 0777;
            stbuf->st_nlink = 1;
            stbuf->st_size = strlen(file->contents);
        }
        else if (file->type == TG_DIR)
        {
            stbuf->st_mode = S_IFDIR | 0777;
            stbuf->st_nlink = 1;
        }
    }
    else
    {
        res = -ENOENT;
    }
    return res;
}

static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi,
                         enum fuse_readdir_flags flags)
{
    (void)offset;
    (void)fi;
    (void)flags;

    tg_file *tg_dir;

    if (strcmp(path, "/") == 0)
    {
        for (int i = 0; i < storage.size; ++i)
        {
            if (strrchr(storage.files[i].path, '/') == storage.files[i].path)
            {
                filler(buf, storage.files[i].path + 1, NULL, 0, FUSE_FILL_DIR_DEFAULTS);
            }
        }
        return 0;
    }

    else if ((tg_dir = tg_storage_find_by_path(&storage, path)))
    {
        if (tg_dir->type != TG_DIR)
        {
            return -ENOENT;
        }
    }
    else
    {
        return -ENOENT;
    }

    filler(buf, ".", NULL, 0, FUSE_FILL_DIR_DEFAULTS);
    filler(buf, "..", NULL, 0, FUSE_FILL_DIR_DEFAULTS);

    int i = -1;
    while ((i = tg_storage_find_files_by_dir(&storage, path, i + 1)) != -1)
    {
        filler(buf, tg_file_get_filename(&storage.files[i]), NULL, 0, FUSE_FILL_DIR_DEFAULTS);
    }

    return 0;
}

static int hello_open(const char *path, struct fuse_file_info *fi)
{

    tg_file *file = tg_storage_find_by_path(&storage, path);
    if (!file)
        return -ENOENT;

    // if ((fi->flags & O_ACCMODE) != O_RDONLY)
    //     return -EACCES;

    return 0;
}

static int hello_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi)
{
    size_t len;
    (void)fi;

    tg_file *file = tg_storage_find_by_path(&storage, path);

    if (!file)
        return -ENOENT;

    len = strlen(file->contents);
    if (offset < len)
    {
        if (offset + size > len)
            size = len - offset;
        memcpy(buf, file->contents + offset, size);
    }
    else
    {
        size = 0;
    }

    return size;
}

int hello_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{

    tg_file *file;
    if (!(file = tg_storage_find_by_path(&storage, path)) || file->type == TG_DIR)
    {
        return -ENOENT;
    }

    int cur_len = strlen(file->contents);

    if (cur_len < offset + size)
    {
        file->contents = realloc(file->contents, sizeof(char) * (offset + size + cur_len));
    }
    memcpy(file->contents + offset, buf, size);
    file->contents[offset + size] = '\0';

    return size;
}

static void hello_destroy(void *private_data)
{
    tg_storage_free(&storage);
}

static const struct fuse_operations hello_oper = {
    .init = hello_init,
    .getattr = hello_getattr,
    .readdir = hello_readdir,
    .open = hello_open,
    .read = hello_read,
    .write = hello_write,
    .destroy = hello_destroy,
};
static void show_help(const char *progname)
{
    printf("usage: %s [options] <mountpoint>\n\n", progname);
    printf("File-system specific options:\n"
           "    --name=<s>          Name of the \"hello\" file\n"
           "                        (default: \"hello\")\n"
           "    --contents=<s>      Contents \"hello\" file\n"
           "                        (default \"Hello, World!\\n\")\n"
           "\n");
}

int main(int argc, char **argv)
{
    int ret;
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

    storage = tg_storage_new();

    tg_storage_push(&storage, (tg_file){
                                  .path = strdup("/hello.txt"),
                                  .contents = strdup("hello world\n"),
                                  .type = TG_FILE,
                              });
    tg_storage_push(&storage, (tg_file){
                                  .path = strdup("/by.txt"),
                                  .contents = strdup("by world\n"),
                                  .type = TG_FILE,
                              });
    tg_storage_push(&storage, (tg_file){
                                  .path = strdup("/tg_dir"),
                                  .contents = NULL,
                                  .type = TG_DIR,
                              });

    tg_storage_push(&storage, (tg_file){
                                  .path = strdup("/tg_dir/hello.txt"),
                                  .contents = strdup("by world\n"),
                                  .type = TG_FILE,
                              });

    fprintf(stderr, "MARK\n");

    for (int i = 0; i < storage.size; ++i)
    {
        fprintf(stderr, "%s\n", storage.files[i].contents);
    }
    fprintf(stderr, "END\n");
    // tg_storage_free(&storage);

    ret = fuse_main(args.argc, args.argv, &hello_oper, NULL);

    fuse_opt_free_args(&args);

    return ret;
}