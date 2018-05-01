#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include "global.h"

/* 创建一个climsg结构体和一个sermsg结构体用来发送和接收消息 */
SERMSG_TYPE ser_msg;
CLIMSG_TYPE cli_msg;

void *recv_sermsg(void *arg);

int main(int argc, char **argv)
{
	/* 参数一定要为三个，一个文件名，一个服务器IP地址，一个服务器端口号 */
	if (argc != 3)
	{
		printf("Usage: %s <serverip> <serverport>\n", argv[0]);
		return -1;
	}

	/* 创建套接字 */
	int cli_sockfd = 0;
	cli_sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (cli_sockfd < 0)
	{
		perror("socket");
		return -1;
	}

    struct sockaddr_in ser_addr;
	memset(&ser_addr, 0, sizeof(struct sockaddr_in));
	socklen_t ser_addrlen = sizeof(struct sockaddr_in);
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_port = htons(atoi(argv[2]));
	ser_addr.sin_addr.s_addr = inet_addr(argv[1]);

    /* 创建接收线程，该线程用来接收服务器发过来的消息 */
    pthread_t thread;
    pthread_create(&thread, NULL, recv_sermsg, (void *)&cli_sockfd);

    /* 登陆操作，键盘输入登陆名，初始化该结构体发送人为public */
    cli_msg.opt = OPT_LOGIN;
    printf("input login name:");
    fgets(cli_msg.msg.fromuser, sizeof(cli_msg.msg.fromuser), stdin);
    cli_msg.msg.fromuser[strlen(cli_msg.msg.fromuser) - 1] = '\0';
    strcpy(cli_msg.msg.touser, "public");

    /* 发送给服务器登陆信息 */
	int num = 0;
    num = sendto(cli_sockfd, &cli_msg, sizeof(CLIMSG_TYPE), 0, (struct sockaddr *)&ser_addr, ser_addrlen);
    if (num < 0)
    {
        perror("sendto");
        return -1;
    }

	printf("欢迎使用聊天室系统\n");
	printf("请在> <username>:下面输入你要发送的信息\n");
	printf("输入to <username>选择要发送的用户\n");
	printf("输入logout退出聊天\n");
    char textbuf[1024];
    cli_msg.opt = OPT_CHAT;
	while (1)
	{
        printf("> <%s>:\n", cli_msg.msg.touser);
        fgets(textbuf, sizeof(textbuf), stdin);
        textbuf[strlen(textbuf) - 1] = '\0';
        /* 改变收信人 */
        if (!strncmp(textbuf, "to", 2))
        {
            strcpy(cli_msg.msg.touser, textbuf+3);
        }

        /* 登出 */
        else if (!strcmp(textbuf, "logout"))
        {
            cli_msg.opt = OPT_LOGOUT;
            sendto(cli_sockfd, &cli_msg, sizeof(CLIMSG_TYPE), 0, (struct sockaddr *)&ser_addr, ser_addrlen);
            return 0;
        }

        /* 发信 */
        else
        {
            strcpy(cli_msg.msg.msg_text, textbuf);
            sendto(cli_sockfd, &cli_msg, sizeof(CLIMSG_TYPE), 0, (struct sockaddr *)&ser_addr, ser_addrlen);
        }
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
        printf("> name <%s>:%s\n", ser_msg.msg.fromuser, ser_msg.msg.msg_text);
		printf("> <%s>:\n", cli_msg.msg.touser);
    }
}
