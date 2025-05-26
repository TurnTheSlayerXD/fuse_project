#ifndef TG_CONFIG_H
#define TG_CONFIG_H

typedef struct
{
    const char *chat_id;
    const char *token;
} tg_config;

tg_config tg_config_new(const char *chat_id, const char *token)
{
    return (tg_config){chat_id, token};
}

#endif