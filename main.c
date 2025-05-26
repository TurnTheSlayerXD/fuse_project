#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include <tg_file.h>
#include <tg_storage.h>

static tg_storage storage;
static tg_config config;

static void *hello_init(struct fuse_conn_info *conn,
                        struct fuse_config *cfg)
{
    int x;

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
            stbuf->st_size = file->file_size;
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

    if ((tg_dir = tg_storage_find_by_path(&storage, path)))
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
    (void)fi;

    tg_file *file = tg_storage_find_by_path(&storage, path);

    if (!file)
        return -ENOENT;

    buffer src = tg_file_load_contents(&config, file);

    size_t len = src.size;
    if (offset < len)
    {
        if (offset + size > len)
            size = len - offset;
        memcpy(buf, src.ptr + offset, size);
    }
    else
    {
        size = 0;
    }

    buffer_free(&src);

    return size;
}

int hello_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{

    tg_file *file;
    tg_file if_no;

    buffer data;

    if (!(file = tg_storage_find_by_path(&storage, path)))
    {

        if_no = tg_file_new_file(path, offset + size);
        tg_storage_push(&storage, if_no);
        file = &if_no;

        data = buffer_new();
    }
    else if (file->type == TG_DIR)
    {
        return -ENOENT;
    }
    else
    {
        data = tg_file_load_contents(&config, file);
    }

    buffer_insert(&data, offset, buf, size);

    tg_put_file(file, &config, &data);

    buffer_free(&data);

    return size;
}

static void hello_destroy(void *private_data)
{
    (void)private_data;
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

int main(int argc, char **argv)
{
    int ret;
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

    storage = tg_storage_new();

    config = tg_config_new("-1002556273060", "6947966209:AAEklSLutFkdTgdfIywyXhK3VrsNISYOWcc");

    tg_storage_push(&storage, tg_file_new_dir("/"));

    fprintf(stderr, "MARK\n");

    fprintf(stderr, "END\n");

    tg_storage_free(&storage);

    ret = fuse_main(args.argc, args.argv, &hello_oper, NULL);

    fuse_opt_free_args(&args);

    return ret;
}