#ifndef __GLOBAL_H__
#define __GLOBAL_H__

typedef enum {
  OPT_LOGIN  = 1,
  OPT_LOGOUT =2,
  OPT_CHAT   = 3
}chat_opt;

typedef enum {
    RET_SUCCESS  = 1,
    RET_ERR      = 2
}chat_ret;

typedef struct {
  char fromuser[50];
  char touser[50];
  char msg_text[1024];
}MSG_TYPE;

typedef struct {
  chat_opt opt;
  MSG_TYPE msg;
}CLIMSG_TYPE;

typedef struct {
    chat_ret ret;
    MSG_TYPE msg;
}SERMSG_TYPE;

typedef struct {
  char onlineflag;
  char username[50];
  struct sockaddr_in cli_addr;
}ONLINE_TYPE;

#endif
