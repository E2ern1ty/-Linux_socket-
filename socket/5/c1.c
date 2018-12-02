#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>

#define MYPORT  7000
#define BUFFER_SIZE 1024
int main()
{
    int sock_cli;
    fd_set rfds;//fd_set的数据结构，实际上是一long类型的数组
    struct timeval tv;//超时 结构体
    int retval, maxfd;

   
    sock_cli = socket(AF_INET,SOCK_STREAM, 0); ///定义sockfd
    
    struct sockaddr_in servaddr;///定义sockaddr_in 四元组
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(MYPORT);  ///服务器端口
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");  ///服务器ip

    //连接服务器，成功返回0，错误返回-1
    if (connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("connect error\n\n\n");
        exit(1);
    }
	printf("connect is OK\n");
	
	char name[20];char bbuf[100];
	printf("write ur name:  ");
        scanf("%s",&name);
	fgets(bbuf, sizeof(bbuf), stdin);
	printf("\nHello! %s u can chat now\n\n\n",name);
	strcat(name," say : ");
	printf("**********************************************\n");

    while(1){
        
        FD_ZERO(&rfds);/*把描述符集合清空*/
       
        FD_SET(0, &rfds); /*把标准输入的文件描述符加入到集合中*/

        maxfd = 0;
        
        FD_SET(sock_cli, &rfds);/*把sock_cli套接字描述符加入到集合中*/
          
        if(maxfd < sock_cli)/*找出套接字描述符集合中最大的套接字描述符*/  
            maxfd = sock_cli;
      
        tv.tv_sec = 5;  /*设置超时时间*/
        tv.tv_usec = 0;
       
        retval = select(maxfd+1, &rfds, NULL, NULL, &tv); /*等待聊天(检查套接字是否可读,有数据)*/
        if(retval == -1){
            printf("select error\n");//select出错
            break;
        }else if(retval == 0){
            //printf("no message，waiting...\n");
            continue;
        }else{ char buf[BUFFER_SIZE];char sendbuf[BUFFER_SIZE];
           
            if(FD_ISSET(sock_cli,&rfds)){ /*服务器发来了消息(检查套接字 ,不可读的会被去掉 ,检查sock_cli是否在集合里)*/
                int len;
                len = recv(sock_cli, buf, sizeof(buf),0);
		printf("----------------------------------\n");
		printf("->>>>     %s\n", buf);
		printf("----------------------------------\n");
                memset(buf, 0, sizeof(buf));
		
            }
            /*用户输入信息了,开始处理信息并发送*/
            if(FD_ISSET(0, &rfds)){
		
                fgets(sendbuf, sizeof(sendbuf), stdin);
		strcpy(bbuf,name);
		strcat(bbuf,sendbuf);
                send(sock_cli, bbuf, strlen(bbuf),0); //发送
		memset(bbuf, 0, sizeof(bbuf));//清空
		memset(sendbuf, 0, sizeof(sendbuf));
                
            }
        }
    }

    close(sock_cli);
    return 0;
}