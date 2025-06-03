#ifndef TG_FILE_H
#define TG_FILE_H

#include <tg_logger.h>
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

static tg_file tg_file_new_file(const char *path, size_t file_size)
{
    tg_file file = (tg_file){.type = TG_FILE, .file_size = file_size};

    strcpy(file.path, path);
    memset(file.file_id, 0, FILE_ID_SIZE);
    memset(file.message_id, 0, MESSAGE_ID_SIZE);

    return file;
}


// static tg_file tg_file_full_new_file(const char *path, const char *file_id, const char *message_id, size_t file_size)
// {
//     tg_file file = (tg_file){.type = TG_FILE, .file_size = file_size};

//     strcpy(file.path, path);
//     strcpy(file.file_id, file_id);
//     strcpy(file.message_id, message_id);

//     return file;
// }

static tg_file tg_file_new_dir(const char *path)
{

    tg_file file = (tg_file){.type = TG_DIR};
    strcpy(file.path, path);
    return file;
}

static void abort_if_error(CURLcode code)
{
    if (code != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed on send: %s\n",
                curl_easy_strerror(code));
        assert(false);
    }
}

void tg_file_extract_filename(char *dst, tg_file *file)
{
    char *ptr = strrchr(file->path, '/');
    int64_t n = file->path + strlen(file->path) - ptr;
    memcpy(dst, ptr + 1, n);
    dst[n] = '\0';
}

void tg_file_extract_directory(char *dst, tg_file *file)
{
    const char *start = strchr(file->path, '/');
    const char *end = strchr(start + 1, '/');
    if (end == NULL)
    {
        dst[0] = '/';
        dst[1] = '\0';
    }
    else
    {
        int64_t n = end - start;
        memcpy(dst, start, n);
        dst[n] = '\0';
    }
}

static size_t curl_response_file_writer(char *data, size_t size, size_t nmemb,
                                        void *p)
{
    buffer_append((buffer *)p, data, size * nmemb);
    return size * nmemb;
}

static size_t curl_response_json_writer(char *data, size_t size, size_t nmemb,
                                        void *p)
{
    memcpy(p, data, size * nmemb);
    return size * nmemb;
}

void tg_put_file(tg_file *file, tg_config *config, buffer *data)
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURLcode code;

    CURL *hnd = curl_easy_init();

    if (!hnd)
    {
        assert(false && "Could not create curl handle");
    }

    curl_easy_setopt(hnd, CURLOPT_ERRORBUFFER, error_buffer);
    curl_easy_setopt(hnd, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, curl_response_json_writer);
    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, json_response_buf);

#define URL_SIZE 1000

    curl_mime *multipart = curl_mime_init(hnd);
    curl_mimepart *part = curl_mime_addpart(multipart);
    code = curl_mime_name(part, "document");
    abort_if_error(code);
    code = curl_mime_data(part, data->ptr, data->size);
    abort_if_error(code);

    char filename[100];
    tg_file_extract_filename(filename, file);
    code = curl_mime_filename(part, filename);
    abort_if_error(code);
    curl_easy_setopt(hnd, CURLOPT_MIMEPOST, multipart);

    while (true)
    {
        if (strlen(file->file_id) == 0)
        {
            char url[URL_SIZE];

            char directory[100];
            tg_file_extract_directory(directory, file);

            sprintf(url, "https://api.telegram.org/bot%s/sendDocument?chat_id=%s&caption=%s",
                    config->token, config->chat_id, directory);

            curl_easy_setopt(hnd, CURLOPT_URL, url);

            code = curl_easy_perform(hnd);

            if (code != CURLE_OK)
            {
            }

            if (!check_ok_status_in_json(json_response_buf))
            {
                break;
            }
            parse_response_field_from_json(file->file_id, json_response_buf, "file_id");
            parse_response_field_from_json(file->message_id, json_response_buf, "message_id");
        }
        else
        {
            const char *media = "{\"type\":\"document\",\"media\":\"attach://document\"}";
            char url[URL_SIZE];

            char directory[100];
            tg_file_extract_directory(directory, file);

            sprintf(url, "https://api.telegram.org/bot%s/editMessageMedia?chat_id=%s&message_id=%s&media=%s&caption=%s",
                    config->token, config->chat_id, file->message_id, media, directory);

            curl_easy_setopt(hnd, CURLOPT_URL, url);
            code = curl_easy_perform(hnd);

            if (code != CURLE_OK)
            {
                assert(false && "COULD NOT SEND PUT FILE REQUEST");
            }

            if (!check_ok_status_in_json(json_response_buf))
            {
                fuse_log(FUSE_LOG_DEBUG, "False status on FILE EDIT REQUEST: %s\n\n", json_response_buf);
                break;
            }
            memset(file->file_id, 0, FILE_ID_SIZE);
            parse_response_field_from_json(file->file_id, json_response_buf, "file_id");
        }
        break;
    }

    curl_mime_free(multipart);
    curl_easy_cleanup(hnd);
    curl_global_cleanup();
}

buffer tg_file_load_contents(tg_config *config, tg_file *src)
{
    memset(json_response_buf, 0, JSON_RESPONSE_BUF_SIZE);

    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURLcode code;

    CURL *hnd = curl_easy_init();

    buffer file_response_buf = buffer_new();

    if (!hnd)
    {
        fuse_log(FUSE_LOG_DEBUG, "COULD NOT INIT HND for CURL\n\n");
        return file_response_buf;
    }

    while (true)
    {

        curl_easy_setopt(hnd, CURLOPT_ERRORBUFFER, error_buffer);
        curl_easy_setopt(hnd, CURLOPT_FOLLOWLOCATION, 1L);

        curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, curl_response_json_writer);
        curl_easy_setopt(hnd, CURLOPT_WRITEDATA, json_response_buf);

        char url[1000];
        sprintf(url, "https://api.telegram.org/bot%s/getFile?file_id=%s", config->token, src->file_id);

        curl_easy_setopt(hnd, CURLOPT_URL, url);

        code = curl_easy_perform(hnd);

        if (code != CURLE_OK)
        {
            fuse_log(FUSE_LOG_DEBUG, "COULD NOT SEND getFile REQUEST\n\n");

            break;
        }

        if (!check_ok_status_in_json(json_response_buf))
        {
            fuse_log(FUSE_LOG_DEBUG, "FAILED CHECKING OK STATUS getFile REQUEST\n\n");
            break;
        }

        char file_path[FILE_PATH_SIZE];
        memset(file_path, 0, FILE_PATH_SIZE);
        parse_response_field_from_json(file_path, json_response_buf, "file_path");

        file_response_buf = buffer_new();
        curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, curl_response_file_writer);
        curl_easy_setopt(hnd, CURLOPT_WRITEDATA, (void *)&file_response_buf);

        sprintf(url, "https://api.telegram.org/file/bot%s/%s", config->token, file_path);
        curl_easy_setopt(hnd, CURLOPT_URL, url);

        code = curl_easy_perform(hnd);

        if (code != CURLE_OK)
        {
            fuse_log(FUSE_LOG_DEBUG, "COULD NOT FETCH FILE\n\n");
            break;
        }

        break;
    }

    curl_easy_cleanup(hnd);
    curl_global_cleanup();

    return file_response_buf;
}

int tg_remove_message_with_file(tg_config *config, tg_file *file)
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURLcode code;

    CURL *hnd = curl_easy_init();

    if (!hnd)
    {
        assert(false && "Could not create curl handle");
    }

    curl_easy_setopt(hnd, CURLOPT_ERRORBUFFER, error_buffer);
    curl_easy_setopt(hnd, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, curl_response_json_writer);
    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, json_response_buf);

    while (true)
    {
#define URL_SIZE 1000

        char url[URL_SIZE];
        sprintf(url, "https://api.telegram.org/bot%s/deleteMessage?chat_id=%s&message_id=%s",
                config->token, config->chat_id, file->message_id);

        curl_easy_setopt(hnd, CURLOPT_URL, url);

        code = curl_easy_perform(hnd);

        if (code != CURLE_OK)
        {
            fuse_log(FUSE_LOG_DEBUG, "CURL RETURNED FALSE deleteMessage REQUEST\n\n");
            break;
        }

        if (!check_ok_status_in_json(json_response_buf))
        {
            fuse_log(FUSE_LOG_DEBUG, "FAILED CHECKING OK STATUS deleteMessage REQUEST\n\n");
            break;
        }

        break;
    }

    curl_easy_cleanup(hnd);
    curl_global_cleanup();

    return 0;
}

#endif // TG_FILE_H