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

static tg_file tg_file_new_dir(char *path)
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

char *tg_put_file(tg_file *file, tg_config *config, buffer *data)
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

#define URL_SIZE 1000
    char url[URL_SIZE];

    curl_mime *multipart = curl_mime_init(hnd);
    curl_mimepart *part = curl_mime_addpart(multipart);
    code = curl_mime_name(part, "document");
    abort_if_error(code);
    code = curl_mime_data(part, data->ptr, data->size);
    abort_if_error(code);
    code = curl_mime_filename(part, tg_file_get_filename(file));
    abort_if_error(code);
    curl_easy_setopt(hnd, CURLOPT_MIMEPOST, multipart);

    if (strlen(file->file_id) == 0)
    {
        sprintf(url, "https://api.telegram.org/bot%s/sendDocument?chat_id=%s", config->token, config->chat_id);
        curl_easy_setopt(hnd, CURLOPT_URL, url);

        code = curl_easy_perform(hnd);

        if (code != CURLE_OK)
        {
            assert(false && "COULD NOT SEND PUT FILE REQUEST");
        }
        fprintf(stderr, "%s\n\n", response_buf);

        parse_response_field_from_json(file->file_id, &response_buf, "file_id");
        parse_response_field_from_json(file->message_id, &response_buf, "message_id");
    }
    else
    {
        const char *media = "{\"type\":\"document\",\"media\":\"attach://document\"}";

        sprintf(url, "https://api.telegram.org/bot%s/editMessageMedia?chat_id=%smessage_id=%smedia=%s",
                config->token, config->chat_id, file->message_id, media);
        curl_easy_setopt(hnd, CURLOPT_URL, url);
        code = curl_easy_perform(hnd);

        if (code != CURLE_OK)
        {
            assert(false && "COULD NOT SEND PUT FILE REQUEST");
        }

        fprintf(stderr, "%s\n\n", response_buf);
    }

    curl_mime_free(multipart);
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
        assert(false && "COULD NOT SEND getFile REQUEST");
    }

    char file_path[FILE_PATH_SIZE];
    parse_response_field_from_json(file_path, &response_buf, "file_path");

    response_buf.size = 0;
    sprintf(url, "https://api.telegram.org/file/bot%s/%s", config->token, file_path);
    curl_easy_setopt(hnd, CURLOPT_URL, url);

    code = curl_easy_perform(hnd);

    if (code != CURLE_OK)
    {
        assert(false && "COULD NOT FETCH FILE");
    }

    curl_easy_cleanup(hnd);
    curl_global_cleanup();

    return response_buf;
}

#endif // TG_FILE_H