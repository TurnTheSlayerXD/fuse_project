#include <stdio.h>
#include <stdlib.h> /* exit */

#include <curl/curl.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#define buffer_SIZE 1000

static char error_buffer[CURL_ERROR_SIZE];

static char stor[buffer_SIZE];

typedef struct
{
    char *ptr;
    size_t capacity;
    size_t size;
} buffer;

#define FILE_ID_SIZE 100
#define FILE_NAME_SIZE 100
typedef struct
{
    char file_id[FILE_ID_SIZE];
    char file_name[FILE_NAME_SIZE];
} tg_file;




buffer buffer_new()
{
    return (buffer){.ptr = malloc(100), .size = 0, .capacity = 100};
}

void buffer_push(buffer *st, char c)
{
    if (st->size >= st->capacity)
    {
        st->capacity = st->capacity * 2 + 1;
        st->ptr = realloc(st->ptr, st->capacity);
    }
    st->ptr[st->size++] = c;
}

void buffer_append(buffer *st, void *data, size_t size)
{
    if (st->size + size >= st->capacity)
    {
        st->capacity += size;
        st->ptr = realloc(st->ptr, st->capacity);
    }
    memcpy(st->ptr + st->size, data, size);
    st->size += size;
}

static size_t curl_writer(char *data, size_t size, size_t nmemb,
                          void *p)
{
    buffer *st = (buffer *)p;
    buffer_append(st, data, size * nmemb);
    return size * nmemb;
}

typedef struct
{
    char *chat_id;
    char *token;
} tg_config;

// return file_id

void parse_response_field_from_json(char *dst, buffer *src, const char *field)
{
    const char *p = strstr(src->ptr, field);
    p += strlen(field) + 2;

    bool is_quoted = false;
    is_quoted = *p == '\"';

    if (is_quoted)
    {
        strncpy(dst, p + 1, strchr(p, ',') - 1 - p);
    }
    else
    {
        strncpy(dst, p, strchr(p, ',') - p);
    }
}

char *tg_put_file(tg_config *config, buffer *data, const char *filename)
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

    code = curl_mime_filedata(part, "./test.txt");
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

    char file_id[1000];
    parse_response_field_from_json(file_id, &response_buf, "file_id");

    curl_easy_cleanup(hnd);
    curl_global_cleanup();
}

typedef struct input input;

static size_t read_callback(char *ptr, size_t size, size_t nmemb, void *userp)
{
    input *i = userp;
    size_t retcode = fread(ptr, size, nmemb, i->in);
    i->bytes_read += retcode;
    return retcode;
}

static int get_file_size(FILE *f)
{
    int cur_pos = ftell(f);

    fseek(f, 0, SEEK_END);
    int end = ftell(f);
    fseek(f, 0, SEEK_SET);
    int start = ftell(f);

    fseek(f, cur_pos, SEEK_SET);

    return end - start;
}

static abort_if_error(CURLcode code)
{
    if (code != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed on send: %s\n",
                curl_easy_strerror(code));
        assert(false);
    }
}

static writer(char *data, size_t size, size_t nmemb,
              void *p)
{
    memcpy(p, data, size * nmemb);
    return size * nmemb;
}

static void receive_file()
{

    CURL *hnd = curl_easy_init();

    const char *file_id = "BQACAgIAAyEGAASYXaGkAAMNaDNcyyCpoIErXO_8Vhdas5SZZd0AAm5vAAIoHJlJlX1URQlwtHA2BA";

    CURLcode code;
    if (hnd)
    {

        buffer st = buffer_new();

        curl_easy_setopt(hnd, CURLOPT_ERRORBUFFER, error_buffer);
        curl_easy_setopt(hnd, CURLOPT_FOLLOWLOCATION, 1L);

        curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, buffer_writer);
        curl_easy_setopt(hnd, CURLOPT_WRITEDATA, &st);
        curl_easy_setopt(hnd, CURLOPT_URL, "https://api.telegram.org/file/bot6947966209:AAGuVGoVPuU-KK3QjvY-3lr_D3zQgujRwyo/documents/file_0");

        code = curl_easy_perform(hnd);

        fprintf(stdout, "response len = %d\r\n",
                st.size);

        fprintf(stdout, "error buf:\r\n%s\n",
                error_buffer);

        curl_easy_cleanup(hnd);
    }
}

static void send_file(input *i)
{
    CURLcode code;

    const char *char_id = "-1002556273060";

    curl_easy_setopt(i->hnd, CURLOPT_ERRORBUFFER, error_buffer);
    curl_easy_setopt(i->hnd, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_setopt(i->hnd, CURLOPT_WRITEFUNCTION, writer);
    curl_easy_setopt(i->hnd, CURLOPT_WRITEDATA, stor);

    curl_mime *multipart = curl_mime_init(i->hnd);

    curl_mimepart *part = curl_mime_addpart(multipart);

    code = curl_mime_name(part, "document");
    abort_if_error(code);

    code = curl_mime_filedata(part, "./test.txt");
    abort_if_error(code);

    curl_easy_setopt(i->hnd, CURLOPT_MIMEPOST, multipart);
    curl_easy_setopt(i->hnd, CURLOPT_URL, "https://api.telegram.org/bot6947966209:AAGuVGoVPuU-KK3QjvY-3lr_D3zQgujRwyo/sendDocument?chat_id=-1002556273060");

    code = curl_easy_perform(i->hnd);

    abort_if_error(code);
    fprintf(stdout, "response len = %d, response:\r\n%s\n\n",
            strlen(stor),
            stor);

    fprintf(stdout, "error buf:\r\n%s\n",
            error_buffer);
}

int main()
{

    memset(stor, 0, buffer_SIZE);
    memset(error_buffer, 0, CURL_ERROR_SIZE);

    char token[] = "6947966209:AAEklSLutFkdTgdfIywyXhK3VrsNISYOWcc";
    char method[] = "getUpdates";

    curl_global_init(CURL_GLOBAL_DEFAULT);

    input i = {
        .hnd = curl_easy_init(),
        .in = fopen("./test.txt", "rb"),
        .num = 0,
        .bytes_read = 0,
    };

    send_file(&i);

    fclose(i.in);
    curl_easy_cleanup(i.hnd);

    // CURL *curl = curl_easy_init();
    // if (curl)
    // {
    //     curl_easy_setopt(curl, CURLOPT_URL, "https://api.telegram.org/bot6947966209:AAGuVGoVPuU-KK3QjvY-3lr_D3zQgujRwyo/getMe");

    //     curl_easy_setopt(curl, CURLOPT_HTTPGET, "");
    //     curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

    //     curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buffer);
    //     curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    //     curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
    //     curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);

    //     CURLcode res = curl_easy_perform(curl);

    //     if (res != CURLE_OK)
    //     {
    //         fprintf(stderr, "curl_easy_perform() failed: %s\n",
    //                 curl_easy_strerror(res));
    //     }
    //     else
    //     {
    //         fprintf(stderr, "response:\r\n%s\n",
    //                 buffer);
    //     }
    //     curl_easy_cleanup(curl);
    // }

    curl_global_cleanup();
    return 0;
}
