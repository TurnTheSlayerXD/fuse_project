#ifndef TG_FILE_H
#define TG_FILE_H

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <tg_config.h>
#include <buffer.h>
#include <curl.h>

static char error_buffer[CURL_ERROR_SIZE];

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
    char *path;
    char file_id[FILE_ID_SIZE];
    char message_id[MESSAGE_ID_SIZE];

    tg_file_type type;

} tg_file;

static tg_file tg_file_new_file(char *path)
{
    return (tg_file){.path = path, .type = TG_FILE};
}

static tg_file tg_file_new_dir(char *path)
{
    return (tg_file){.path = path, .type = TG_DIR};
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

char *tg_file_get_filename(tg_file *file)
{
    return strrchr(file->path, '/') + 1;
}

static size_t curl_writer(char *data, size_t size, size_t nmemb,
                          void *p)
{
    buffer_append((buffer *)p, data, size * nmemb);
    return size * nmemb;
}

char *tg_put_file(tg_file *dst, tg_config *config, buffer *data)
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURLcode code;

    CURL *hnd = curl_easy_init();

    if (!hnd)
    {
        assert(false && "Could not create curl handle");
    }

    buffer response_buf = buffer_new();

    curl_easy_setopt(hnd, CURLOPT_ERRORBUFFER, error_buffer);
    curl_easy_setopt(hnd, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, curl_writer);
    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, response_buf);

    curl_mime *multipart = curl_mime_init(hnd);
    curl_mimepart *part = curl_mime_addpart(multipart);

    code = curl_mime_name(part, "document");
    abort_if_error(code);

    code = curl_mime_filedata(part, tg_file_get_filename(dst));
    abort_if_error(code);

    curl_easy_setopt(hnd, CURLOPT_MIMEPOST, multipart);

    char url[1000];

    sprintf(url, "https://api.telegram.org/bot%s/sendDocument?chat_id=%s", config->token, config->chat_id);

    curl_easy_setopt(hnd, CURLOPT_URL, url);

    code = curl_easy_perform(hnd);

    if (code != CURLE_OK)
    {
        assert(false && "COULD NOT SEND PUT FILE REQUEST");
    }

    parse_response_field_from_json(dst->file_id, &response_buf, "file_id");
    parse_response_field_from_json(dst->message_id, &response_buf, "message_id");

    curl_easy_cleanup(hnd);
    curl_global_cleanup();
}

buffer tg_file_load_contents(tg_config *config, tg_file *src)
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURLcode code;

    CURL *hnd = curl_easy_init();

    if (!hnd)
    {
        assert(false && "Could not create curl handle");
    }

    buffer response_buf = buffer_new();

    curl_easy_setopt(hnd, CURLOPT_ERRORBUFFER, error_buffer);
    curl_easy_setopt(hnd, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, curl_writer);
    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &response_buf);

    char url[1000];
    sprintf(url, "https://api.telegram.org/bot%s/getFile?file_id=%s", config->token, src->file_id);

    curl_easy_setopt(hnd, CURLOPT_URL, url);

    code = curl_easy_perform(hnd);

    if (code != CURLE_OK)
    {
        assert(false && "COULD NOT SEND PUT FILE REQUEST");
    }

    char file_path[FILE_PATH_SIZE];
    parse_response_field_from_json(file_path, &response_buf, "file_path");

    response_buf.size = 0;
    sprintf(url, "https://api.telegram.org/file/bot%s/%s", config->token, file_path);

    curl_easy_cleanup(hnd);
    curl_global_cleanup();

    return response_buf;
}

#endif // TG_FILE_H