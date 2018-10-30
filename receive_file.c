#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "transfer.h"

void writefile(int sockfd, FILE *fp);

int main(int argc, char *argv[]) 
{
    //创建TCP套接字
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);  //ipv4 address, TCP type, default connection mode 
    if (sockfd == -1)    //TCP套接字 create failed
    {
        perror("Can't allocate sockfd");
        exit(1);
    }
    
    //配置服务器套接字地址
    struct sockaddr_in clientaddr, serveraddr;   //define structural variable type
    memset(&serveraddr, 0, sizeof(serveraddr));  //将serveraddr中当前位置后面的n个字节用0替换并返回severaddr, n = sizeof(serveraddr)  
    serveraddr.sin_family = AF_INET;   //该字段指定地址家族,对于TCP/IP协议的，必须设置为AF_INET
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);   //以主机字节序表示的32位整型数,ip address
    serveraddr.sin_port = htons(SERVERPORT);   //设置端口

    //绑定套接字与地址
    if (bind(sockfd, (const struct sockaddr *) &serveraddr, sizeof(serveraddr)) == -1)   //绑定套接字 failed   sockfd:表示已建立的socket编号  serveraddr:指向要绑给sockfd的协议地址
    {
        perror("Bind Error");
        exit(1);
    }

    //转换为监听套接字
    if (listen(sockfd, LINSTENPORT) == -1) //转换监听套接字 failed  LISTENPORT:等待队列连接的最大长度
    {
        perror("Listen Error");
        exit(1);
    }

    //等待连接完成
    socklen_t addrlen = sizeof(clientaddr);
    int connfd = accept(sockfd, (struct sockaddr *) &clientaddr, &addrlen); //已连接套接字  sockfd:套接字描述符该套接口在listen()后监听连接   clientaddr:指向一缓冲区,其中接收为通讯层所知的连接实体的地址
    if (connfd == -1) 
    {
        perror("Connect Error");
        exit(1);
    }
    close(sockfd); //关闭监听套接字

    //获取文件名
    char filename[BUFFSIZE] = {0}; //文件名
    if (recv(connfd, filename, BUFFSIZE, 0) == -1) 
    {
        perror("Can't receive filename");
        exit(1);
    }

    //创建文件
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL) 
    {
        perror("Can't open file");
        exit(1);
    }
    
    //把数据写入文件
    char addr[INET_ADDRSTRLEN];
    printf("Start receive file: %s from %s\n", filename, inet_ntop(AF_INET, &clientaddr.sin_addr, addr, INET_ADDRSTRLEN));  //转换网络二进制结构转到ASC II类型的地址
    writefile(connfd, fp);
    puts("Receive Success");

    //关闭文件和已连接套接字
    fclose(fp);
    close(connfd);
    return 0;
}

void writefile(int sockfd, FILE *fp)
{
    ssize_t n; //每次接受数据数量
    char buff[MAX_LINE] = {0}; //数据缓存
    while ((n = recv(sockfd, buff, MAX_LINE, 0)) > 0) 
    {
        if (n == -1)
        {
            perror("Receive File Error");
            exit(1);
        }
        
        //将接受的数据写入文件
        if (fwrite(buff, sizeof(char), n, fp) != n)
        {
            perror("Write File Error");
            exit(1);
        }
        memset(buff, 0, MAX_LINE); //清空缓存
    }
}
