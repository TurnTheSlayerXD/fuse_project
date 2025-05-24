#include <stdio.h>
#include <stdlib.h> /* exit */

#include <curl/curl.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#define STORAGE_SIZE 1000

static char error_buffer[CURL_ERROR_SIZE];

static char storage[STORAGE_SIZE];

static size_t writer(char *data, size_t size, size_t nmemb,
                     void *storage)
{
    memcpy(storage, data, size * nmemb);

    return size * nmemb;
}

struct input
{
    FILE *in;
    size_t bytes_read; /* count up */
    CURL *hnd;
    int num;
};

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

static void send_file(input *i)
{
    CURLcode code;

    const char *char_id = "-1002556273060";

    curl_easy_setopt(i->hnd, CURLOPT_ERRORBUFFER, error_buffer);
    curl_easy_setopt(i->hnd, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_setopt(i->hnd, CURLOPT_WRITEFUNCTION, writer);
    curl_easy_setopt(i->hnd, CURLOPT_WRITEDATA, storage);

    curl_mime *multipart = curl_mime_init(i->hnd);

    curl_mimepart *part = curl_mime_addpart(multipart);

    code = curl_mime_name(part, "document");
    abort_if_error(code);

    code = curl_mime_filedata(part, "./socket_test");
    abort_if_error(code);

    curl_easy_setopt(i->hnd, CURLOPT_MIMEPOST, multipart);
    curl_easy_setopt(i->hnd, CURLOPT_URL, "https://api.telegram.org/bot6947966209:AAGuVGoVPuU-KK3QjvY-3lr_D3zQgujRwyo/sendDocument?chat_id=-1002556273060");

    code = curl_easy_perform(i->hnd);

    abort_if_error(code);
    fprintf(stdout, "response len = %d, file_size = %d, response:\r\n%s\n",
            strlen(storage),
            get_file_size(i->in),
            storage);

    fprintf(stdout, "error buf:\r\n%s\n",
            error_buffer);
}

int main()
{

    memset(storage, 0, STORAGE_SIZE);
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
    //     curl_easy_setopt(curl, CURLOPT_WRITEDATA, storage);

    //     CURLcode res = curl_easy_perform(curl);

    //     if (res != CURLE_OK)
    //     {
    //         fprintf(stderr, "curl_easy_perform() failed: %s\n",
    //                 curl_easy_strerror(res));
    //     }
    //     else
    //     {
    //         fprintf(stderr, "response:\r\n%s\n",
    //                 storage);
    //     }
    //     curl_easy_cleanup(curl);
    // }

    curl_global_cleanup();
    return 0;
}
