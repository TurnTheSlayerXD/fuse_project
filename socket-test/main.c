#include <stdio.h>
#include <stdlib.h> /* exit */

#include <curl/curl.h>
#include <string.h>
#define STORAGE_SIZE 1000

static char error_buffer[CURL_ERROR_SIZE];

static char storage[STORAGE_SIZE];

static size_t writer(char *data, size_t size, size_t nmemb,
                     void *storage)
{
    memcpy(storage, data, size * nmemb);

    return size * nmemb;
}

int main()

{

    memset(storage, 0, STORAGE_SIZE);
    char token[] = "6947966209:AAEklSLutFkdTgdfIywyXhK3VrsNISYOWcc";
    char method[] = "getUpdates";

    CURL *curl;

    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();

    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.telegram.org/bot6947966209:AAGuVGoVPuU-KK3QjvY-3lr_D3zQgujRwyo/getMe");
        
        // curl_easy_setopt(curl, CURLOPT_HTTPGET, "");
        // curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buffer);
        // curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, storage);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        }
        else
        {
            fprintf(stderr, "response:\r\n%s\n",
                    storage);
        }
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    return 0;
}
