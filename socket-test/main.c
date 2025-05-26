#include <stdio.h>
#include <stdlib.h> /* exit */

#include <curl/curl.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#define RESPONSE_BUF_SIZE 1000

static char error_buffer[CURL_ERROR_SIZE];

static char response_buf[RESPONSE_BUF_SIZE];

static void abort_if_error(CURLcode code)
{
    if (code != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed on send: %s\n",
                curl_easy_strerror(code));
        assert(false);
    }
}

size_t get_file_size(FILE *f)
{

    long pos = ftell(f);

    fseek(f, 0, SEEK_SET);
    long b = ftell(f);

    fseek(f, 0, SEEK_END);

    long e = ftell(f);

    fseek(f, pos, SEEK_SET);

    return e - b;
}
static size_t curl_writer(char *data, size_t size, size_t nmemb,
                          void *p)
{
    memcpy(response_buf, data, size * nmemb);
    return size * nmemb;
}

static void edit_message_data(const char *path)
{
    CURL *hnd = curl_easy_init();

    CURLcode code;

    const char *char_id = "-1002556273060";

    curl_easy_setopt(hnd, CURLOPT_ERRORBUFFER, error_buffer);
    curl_easy_setopt(hnd, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, curl_writer);
    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, response_buf);

    FILE *f = fopen(path, "rb");
    size_t size = get_file_size(f);
    char *data = malloc(size);
    fread(data, size, 1, f);
    fclose(f);

    curl_mime *multipart = curl_mime_init(hnd);
    curl_mimepart *part = curl_mime_addpart(multipart);
    code = curl_mime_name(part, "file");
    abort_if_error(code);
    code = curl_mime_data(part, data, size);
    abort_if_error(code);
    code = curl_mime_filename(part, "new_file.txt");
    abort_if_error(code);

    const char *media = "{\"type\":\"document\",\"media\":\"attach://file\"}";
    curl_easy_setopt(hnd, CURLOPT_MIMEPOST, multipart);

    char url[1000];
    const char *chat_id = "-1002556273060";
    const char *message_id = "14";
    sprintf(url, "https://api.telegram.org/bot6947966209:AAGuVGoVPuU-KK3QjvY-3lr_D3zQgujRwyo/editMessageMedia?chat_id=%s&message_id=%s&media=%s",
            char_id, message_id, media);
    curl_easy_setopt(hnd, CURLOPT_URL, url);
    printf("URL:%s\n\n", url);

    code = curl_easy_perform(hnd);

    abort_if_error(code);
    fprintf(stdout, "response len = %d, response:\r\n%s\n\n",
            strlen(response_buf),
            response_buf);

    fprintf(stdout, "error buf:\r\n%s\n",
            error_buffer);

    free(data);
    curl_mime_free(multipart);
    curl_easy_cleanup(hnd);
}

static void send_file(const char *path)
{
    CURL *hnd = curl_easy_init();

    CURLcode code;

    const char *char_id = "-1002556273060";

    curl_easy_setopt(hnd, CURLOPT_ERRORBUFFER, error_buffer);
    curl_easy_setopt(hnd, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, curl_writer);
    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, response_buf);

    FILE *f = fopen(path, "rb");
    size_t size = get_file_size(f);
    char *data = malloc(size);
    fread(data, size, 1, f);
    fclose(f);

    curl_mime *multipart = curl_mime_init(hnd);
    curl_mimepart *part = curl_mime_addpart(multipart);
    code = curl_mime_name(part, "document");
    abort_if_error(code);
    code = curl_mime_data(part, data, size);
    abort_if_error(code);
    code = curl_mime_filename(part, "some_file.txt");
    abort_if_error(code);

    curl_easy_setopt(hnd, CURLOPT_MIMEPOST, multipart);
    curl_easy_setopt(hnd, CURLOPT_URL, "https://api.telegram.org/bot6947966209:AAGuVGoVPuU-KK3QjvY-3lr_D3zQgujRwyo/sendDocument?chat_id=-1002556273060");

    code = curl_easy_perform(hnd);

    abort_if_error(code);
    fprintf(stdout, "response len = %d, response:\r\n%s\n\n",
            strlen(response_buf),
            response_buf);

    fprintf(stdout, "error buf:\r\n%s\n",
            error_buffer);

    free(data);
    curl_mime_free(multipart);
    curl_easy_cleanup(hnd);
}

int main()
{
    memset(response_buf, 0, RESPONSE_BUF_SIZE);
    memset(error_buffer, 0, CURL_ERROR_SIZE);

    char token[] = "6947966209:AAEklSLutFkdTgdfIywyXhK3VrsNISYOWcc";
    char method[] = "getUpdates";

    curl_global_init(CURL_GLOBAL_DEFAULT);

    edit_message_data("./hello.txt");

    curl_global_cleanup();
    return 0;
}
