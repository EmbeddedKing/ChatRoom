#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include "global.h"

void *recv_climsg(void *arg);

/* 创建实时在线列表，最大只能有ONLINE_MAX个人同时在线 */
ONLINE_TYPE online_list[ONLINE_MAX];

int main(int argc, char **argv)
{
    /* 参数要等于3个 */
	if (argc != 3)
	{
		printf("Usage: %s <serverip> <serverport>\n", argv[0]);
		return -1;
	}

    /* 创建套接字 */
	int ser_sockfd = 0;
	ser_sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (ser_sockfd < 0)
	{
		perror("socket");
		return -1;
	}

    /* 绑定端口 */
	unsigned short port = atoi(argv[2]);
	struct sockaddr_in ser_addr;
	socklen_t ser_addrlen = sizeof(struct sockaddr_in);
	memset(&ser_addr, 0, sizeof(struct sockaddr_in));
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_port = htons(atoi(argv[2]));
	ser_addr.sin_addr.s_addr = inet_addr(argv[1]);
	int ret = 0;
	ret = bind(ser_sockfd, (struct sockaddr *)&ser_addr, ser_addrlen);
	if (ret < 0)
	{
		perror("bind");
		return -1;
	}

    printf("欢迎使用聊天室系统！\n");
	/* 注册线程，该线程用来接收消息 */
    pthread_t recvmsg_thread;
    pthread_create(&recvmsg_thread, NULL, recv_climsg, (void *)&ser_sockfd);
    while(1);
    return 0;
}

void *recv_climsg(void *arg)
{
    int ser_sockfd = 0;
    ser_sockfd = *(int *)arg;

    /* 创建一个sockaddr_in结构体用来保存客户端的地址 */
    struct sockaddr_in cli_addr;
    memset(&cli_addr, 0, sizeof(struct sockaddr_in));
    socklen_t cli_addrlen = sizeof(struct sockaddr_in);
    /* 创建一个sermsg结构体用来返回操作结果，清空该结构体 */
    SERMSG_TYPE ser_msg;
    memset(&ser_msg, 0, sizeof(SERMSG_TYPE));
    /* 创建一个climsg结构体用来接收客户端发送过来的操作，清空该结构体 */
    CLIMSG_TYPE cli_msg;
    memset(&cli_msg, 0, sizeof(CLIMSG_TYPE));

    int i = 0;
    int num = 0;
    while (1)
	{
        /* 接收信息，放入cli_msg结构体内,该信息是什么类型由结构体决定 */
        num = recvfrom(ser_sockfd, &cli_msg, sizeof(CLIMSG_TYPE), 0, (struct sockaddr *)&cli_addr, &cli_addrlen);
        if (num < 0)
        {
            perror("recvfrom");
            exit(-1);
        }

        ser_msg.ret = RET_SUCCESS;
        ser_msg.msg = cli_msg.msg;

        switch (cli_msg.opt) {
            /* 登录操作 */
            case OPT_LOGIN:
                for (i = 0; i < ONLINE_MAX; i++) {
                    if (online_list[i].onlineflag == 1) {
                        continue;
                    }
                    online_list[i].onlineflag = 1;
                    strcpy(online_list[i].username, cli_msg.msg.fromuser);
                    memcpy(&online_list[i].cli_addr, &cli_addr, sizeof(struct sockaddr_in));
                    break;
                }
                if (i == ONLINE_MAX) {
                    printf("最大在线人数已满");
                } else {
                    printf("%s加入聊天\n", online_list[i].username);
                }
                break;

            case OPT_LOGOUT:
                for (i = 0; i < ONLINE_MAX; i++)
                {
                    /*如果不在线，则跳过 */
                    if (online_list[i].onlineflag != 1) {
                        continue;
                    }

                    /* 如果在线但名字不匹配则跳过 */
                    else if (strcmp(online_list[i].username, ser_msg.msg.fromuser) != 0) {
                        continue;
                    }

                    /* 如果在线且名字匹配则将该节点清空 */
                    else {
                        printf("%s退出聊天\n", online_list[i].username);
                        memset(online_list+i, 0, sizeof(ONLINE_TYPE));
                    }
                }
                break;

            case OPT_CHAT:
                for (i = 0; i < ONLINE_MAX; i++) {
                    /* 不在线则跳过 */
                    if (online_list[i].onlineflag != 1)
                        continue;

                    /* 如果对方名字为public，则发给每一个人，除了自己 */
                    else if ((!strcmp(ser_msg.msg.touser, "public")) && (strcmp(ser_msg.msg.fromuser, online_list[i].username) != 0)) {
                        num = sendto(ser_sockfd, &ser_msg, sizeof(SERMSG_TYPE), 0, (struct sockaddr *)&online_list[i].cli_addr, sizeof(struct sockaddr_in));
                        if (num < 0)
                        {
                            perror("sendto");
                            exit(-1);
                        }
                    }

                    /* 如果对方名字不为pubulic且当前的表项不为对方 */
                    else if (strcmp(online_list[i].username, ser_msg.msg.touser) != 0)
                        continue;
                    else {
                        num = sendto(ser_sockfd, &ser_msg, sizeof(SERMSG_TYPE), 0, (struct sockaddr *)&online_list[i].cli_addr, sizeof(struct sockaddr_in));
                        if (num < 0)
                        {
                            perror("sendto");
                        }
                        break;
                    }
                }
                break;
            default:
                break;
        }
    }
}
