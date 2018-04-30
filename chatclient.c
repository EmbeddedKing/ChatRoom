#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include "global.h"

SERMSG_TYPE ser_msg;

void *recv_sermsg(void *arg);

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		printf("Usage: %s <serverip> <serverport>\n", argv[0]);
		return -1;
	}

	int send_sockfd = 0;
	send_sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (send_sockfd < 0)
	{
		perror("socket");
		return -1;
	}

    struct sockaddr_in recv_addr;
	memset(&recv_addr, 0, sizeof(struct sockaddr_in));
	socklen_t recv_addrlen = sizeof(struct sockaddr_in);
	recv_addr.sin_family = AF_INET;
	recv_addr.sin_port = htons(atoi(argv[2]));
	recv_addr.sin_addr.s_addr = inet_addr(argv[1]);

    /* 开启接收线程 */
    pthread_t thread;
    pthread_create(&thread, NULL, recv_sermsg, (void *)&send_sockfd);

    /* 创建一个climsg结构体用来发送你要发送的信息并清空该结构体 */
    int num = 0;
    CLIMSG_TYPE cli_msg;
    memset(&cli_msg, 0, sizeof(CLIMSG_TYPE));

    /* 登陆操作，键盘输入登陆名，初始化该结构体发送人为public */
    cli_msg.opt = OPT_LOGIN;
    printf("input login name:");
    fgets(cli_msg.msg.fromuser, sizeof(cli_msg.msg.fromuser), stdin);
    cli_msg.msg.fromuser[strlen(cli_msg.msg.fromuser) - 1] = '\0';
    strcpy(cli_msg.msg.touser, "public");

    /* 发送给服务器登陆信息 */
    num = sendto(send_sockfd, &cli_msg, sizeof(CLIMSG_TYPE), 0, (struct sockaddr *)&recv_addr, recv_addrlen);
    if (num < 0)
    {
        perror("sendto");
        return -1;
    }

    char send_text[1024];
    cli_msg.opt = OPT_CHAT;
	while (1)
	{
        printf("请输入你要发送的信息 输入to <username>选择要发送的用户 输入logout退出聊天\n");
        printf(">%s:",cli_msg.msg.touser);
        fgets(send_text, sizeof(send_text), stdin);
        send_text[strlen(send_text) - 1] = '\0';
        /* 改变收信人 */
        if (!strncmp(send_text, "to", 2))
        {
            /* 更改用户 */
            strcpy(cli_msg.msg.touser, send_text+3);
        }
        /* 登出 */
        else if (!strcmp(send_text, "logout"))
        {
            cli_msg.opt = OPT_LOGOUT;
            sendto(send_sockfd, &cli_msg, sizeof(CLIMSG_TYPE), 0, (struct sockaddr *)&recv_addr, recv_addrlen);
            return 0;
        }
        /* 发信 */
        else
        {
            strcpy(cli_msg.msg.msg_text, send_text);
            sendto(send_sockfd, &cli_msg, sizeof(CLIMSG_TYPE), 0, (struct sockaddr *)&recv_addr, recv_addrlen);
        }
        /*printf("opt:%d\n", cli_msg.opt);
        printf("fromuser:%s\n", cli_msg.msg.fromuser);
        printf("touser:%s\n", cli_msg.msg.touser);
        printf("msg_text:%s\n", cli_msg.msg.msg_text);*/
	}
}

/* 收信 */
void *recv_sermsg(void *arg)
{
    int cli_sockfd = 0;
    cli_sockfd = *(int *)arg;
    struct sockaddr_in ser_addr;
    memset(&ser_addr, 0, sizeof(struct sockaddr));
    socklen_t ser_addrlen = sizeof(struct sockaddr_in);
    while (1)
    {
        memset(&ser_msg, 0, sizeof(SERMSG_TYPE));
        recvfrom(cli_sockfd, &ser_msg, sizeof(SERMSG_TYPE), 0, (struct sockaddr *)&ser_addr, &ser_addrlen);
        printf("\n(new msg)>name <%s>:%s\n", ser_msg.msg.fromuser, ser_msg.msg.msg_text);
    }
}
