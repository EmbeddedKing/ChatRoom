#ifndef __GLOBAL_H__
#define __GLOBAL_H__

/* 最大可同时在线人数 */
#define ONLINE_MAX 50

/* 客户端操作选项 */
typedef enum {
  OPT_LOGIN  = 1,
  OPT_LOGOUT =2,
  OPT_CHAT   = 3
}chat_opt;

/* 服务器响应 */
typedef enum {
    RET_SUCCESS  = 1,
    RET_ERR      = 2
}chat_ret;

/* 消息结构体 */
typedef struct {
  char fromuser[50];
  char touser[50];
  char msg_text[1024];
}MSG_TYPE;

/* 客户端消息 */
typedef struct {
  chat_opt opt;
  MSG_TYPE msg;
}CLIMSG_TYPE;

/* 服务器消息 */
typedef struct {
    chat_ret ret;
    MSG_TYPE msg;
}SERMSG_TYPE;

/* 在线消息类型 */
typedef struct {
  char onlineflag;
  char username[50];
  struct sockaddr_in cli_addr;
}ONLINE_TYPE;

#endif
