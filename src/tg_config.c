#include <tg_config.h>

tg_config tg_config_new(const char *chat_id, const char *token)
{
    tg_config config;
    strcpy(config.chat_id, chat_id);
    strcpy(config.token, token);
    return config;
}