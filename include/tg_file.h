#ifndef TG_FILE_H
#define TG_FILE_H

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <tg_config.h>
#include <buffer.h>
#include <curl.h>
#include <inttypes.h>

#define JSON_RESPONSE_BUF_SIZE 1024
static char error_buffer[CURL_ERROR_SIZE];

static char json_response_buf[JSON_RESPONSE_BUF_SIZE];

typedef enum
{
    TG_FILE,
    TG_DIR
} tg_file_type;

#define FILE_ID_SIZE 100
#define MESSAGE_ID_SIZE 100
#define FILE_PATH_SIZE 100

typedef struct
{
    char path[FILE_PATH_SIZE];
    char file_id[FILE_ID_SIZE];
    char message_id[MESSAGE_ID_SIZE];

    size_t file_size;

    tg_file_type type;

} tg_file;

tg_file tg_file_new_file(const char *path, size_t file_size);

// static tg_file tg_file_full_new_file(const char *path, const char *file_id, const char *message_id, size_t file_size)
// {
//     tg_file file = (tg_file){.type = TG_FILE, .file_size = file_size};

//     strcpy(file.path, path);
//     strcpy(file.file_id, file_id);
//     strcpy(file.message_id, message_id);

//     return file;
// }

tg_file tg_file_new_dir(const char *path);
void abort_if_error(CURLcode code);
void tg_file_extract_filename(char *dst, tg_file *file);
void tg_file_extract_directory(char *dst, tg_file *file);
size_t curl_response_file_writer(char *data, size_t size, size_t nmemb,
                                 void *p);
static size_t curl_response_json_writer(char *data, size_t size, size_t nmemb,
                                        void *p);
void tg_put_file(tg_file *file, tg_config *config, buffer *data);
buffer tg_file_load_contents(tg_config *config, tg_file *src);
int tg_remove_message_with_file(tg_config *config, tg_file *file);

#endif // TG_FILE_H