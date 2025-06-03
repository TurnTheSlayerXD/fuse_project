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

    fuse_log(FUSE_LOG_DEBUG, "GETATTR CALLED\n");
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
    fuse_log(FUSE_LOG_DEBUG, "READDIR CALLED\n");

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
        tg_file *cur = &storage.files[i];

        char filename[100];
        tg_file_extract_filename(filename, cur);

        fuse_log(FUSE_LOG_DEBUG, "READDIR: reading [%s]\n", filename);

        filler(buf, filename, NULL, 0, FUSE_FILL_DIR_DEFAULTS);
    }

    return 0;
}

static int hello_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    (void)fi;
    fuse_log(FUSE_LOG_DEBUG, "CREATE CALLED\n");

    (void)mode;

    tg_file *file;

    if ((file = tg_storage_find_by_path(&storage, path)) && file->type == TG_DIR)
    {
        return -EISDIR;
    }
    tg_file new_file = tg_file_new_file(path, 0);

    tg_storage_push(&storage, new_file);
    return 0;
}

static int hello_open(const char *path, struct fuse_file_info *fi)
{
    (void)fi;
    fuse_log(FUSE_LOG_DEBUG, "OPEN CALLED\n");

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

    fuse_log(FUSE_LOG_DEBUG, "READ CALLED\n");

    (void)fi;

    tg_file *file = tg_storage_find_by_path(&storage, path);

    if (!file)
        return -ENOENT;

    buffer src = tg_file_load_contents(&config, file);

    if (src.size == 0)
    {
        return -ENOENT;
    }

    size_t len = src.size;
    if (offset < (off_t)len)
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
    (void)fi;

    fuse_log(FUSE_LOG_DEBUG, "WRITE CALLED\n");

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
    else if (file->file_size > 0)
    {
        data = tg_file_load_contents(&config, file);
    }
    else
    {
        data = buffer_new();
    }

    buffer_insert(&data, offset, buf, size);

    tg_put_file(file, &config, &data);

    file->file_size = data.size;

    buffer_free(&data);

    return size;
}

static void hello_destroy(void *private_data)
{

    (void)private_data;
    tg_storage_free(&storage);
}

static int hello_mkdir(const char *path, mode_t mode)
{

    (void)mode;

    fuse_log(FUSE_LOG_DEBUG, "Entered MKDIR cbk: path=[%s]\n", path);
    tg_file dir = tg_file_new_dir(path);
    tg_storage_push(&storage, dir);

    return 0;
}

int hello_unlink(const char *path)
{
    fuse_log(FUSE_LOG_DEBUG, "Entered UNLINK cbk: path=[%s]\n", path);
    tg_file file = tg_storage_remove_by_path(&storage, path);

    if (strlen(file.path) == 0)
    {
        return -ENOENT;
    }

    tg_remove_message_with_file(&config, &file);
    return 0;
}

static const struct fuse_operations hello_oper = {
    .init = hello_init,
    .getattr = hello_getattr,
    .readdir = hello_readdir,
    .open = hello_open,
    .read = hello_read,
    .write = hello_write,
    .destroy = hello_destroy,
    .create = hello_create,
    .mkdir = hello_mkdir,
    .unlink = hello_unlink,
};

int main(int argc, char **argv)
{
    int ret;
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

    config = tg_config_new("-1002556273060", "6947966209:AAGuVGoVPuU-KK3QjvY-3lr_D3zQgujRwyo");
    storage = tg_storage_new();

    tg_storage_push(&storage, tg_file_new_dir("/"));

    ret = fuse_main(args.argc, args.argv, &hello_oper, NULL);

    fuse_opt_free_args(&args);

    return ret;
}