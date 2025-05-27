#ifndef TG_CONFIG_H
#define TG_CONFIG_H

#define CHAT_ID_SIZE 100
#define TOKEN_SIZE 100

typedef struct
{
    char chat_id[CHAT_ID_SIZE];
    char token[TOKEN_SIZE];
} tg_config;

tg_config tg_config_new(const char *chat_id, const char *token)
{
    tg_config config;
    strcpy(config.chat_id, chat_id);
    strcpy(config.token, token);
    return config;
}

#endif