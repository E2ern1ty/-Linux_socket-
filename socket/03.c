//udp时间客户端
#include <sys/types.h>

#include <unistd.h>

#include <stdlib.h>

#include <string.h>

#include <stdio.h>

#include <time.h>

#include <errno.h>

#include <stdarg.h>

#include <stdlib.h>

#include <sys/socket.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include <netdb.h>




#ifndef	INADDR_NONE
#define	INADDR_NONE	0xffffffff
#endif	/* INADDR_NONE */

#define	BUFSIZE 64

#define	UNIXEPOCH	2208988800UL	/* UNIX epoch, in UCT secs*/

#define	MSG		"what time is it?\n"
int	connectUDP(const char *host, const char *service);

int	errexit(const char *format, ...);
/*------------------------------------------------------------------------

* main - UDP client for TIME service that prints the resulting time

*------------------------------------------------------------------------

*/

int

main(int argc, char *argv[])

{

	char	*host = "QiHongCheng";	/* 如果没有提供地址，则使用主机名*/

	char	*service = "daytime";	/* 默认服务名称*/

	time_t	now;			/* 保存时间的32位整数(实际上是长整形)*/

	int	s, n;			/* 套接字描述符，读取计数*/



	switch (argc) {

	case 1:

		host = "QiHongCheng";

		break;

	case 3:

		service = argv[2];

		/* FALL THROUGH */

	case 2:

		host = argv[1];

		break;

	default:

		fprintf(stderr, "usage: UDPtime [host [port]]\n");

		exit(1);

	}



	s = connectUDP(host, service);



	(void)write(s, MSG, strlen(MSG));//向time服务器发送msg查询时间



	/* Read the time */



	n = read(s, (char *)&now, sizeof(now));//读取时间

	if (n < 0)

		errexit("read failed: %s\n", strerror(errno));

	now = ntohl((unsigned long)now);	/* put in host order转换字节序	*/

	now -= UNIXEPOCH;		/* 将UCT转换为UNIX纪元	*/

	printf("%s", ctime(&now));

	exit(0);

}

/* errexit.c - errexit */


/*------------------------------------------------------------------------

* errexit - 打印错误消息并退出

*------------------------------------------------------------------------

*/

int

errexit(const char *format, ...)

{

	va_list	args;



	va_start(args, format);

	vfprintf(stderr, format, args);

	va_end(args);

	exit(1);

}


int	connectsock(const char *host, const char *service,const char *transport);
/*------------------------------------------------------------------------
 * connectUDP - 连接到指定主机上的指定UDP服务
 *------------------------------------------------------------------------
 */
int connectUDP(const char *host, const char *service )
/*
 * Arguments:
 *      host    -需要连接的主机的名称
 *      service - 与所需端口关联的服务
 */
{
	return connectsock(host, service, "udp");
}


int
connectsock(const char *host, const char *service, const char *transport )
/*
 * Arguments:
 *      host      - name of host to which connection is desired
 *      service   - service associated with the desired port
 *      transport - name of transport protocol to use ("tcp" or "udp")
 */
{
	struct hostent	*phe;	/*指向主机信息条目的指针*/
	struct servent	*pse;	/* 指向服务信息条目的指针*/
	struct protoent *ppe;	/* 指向协议信息条目的指针*/
	struct sockaddr_in sin;	/*Internet端点地址*/
	int	s, type;	/*套接字描述符和套接字类型*/


	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;

    /* 将服务名称映射到端口号 */
	if ( pse = getservbyname(service, transport) )
		sin.sin_port = pse->s_port;
	else if ((sin.sin_port=htons((unsigned short)atoi(service))) == 0)
		errexit("can't get \"%s\" service entry\n", service);

    /* 将主机名映射到IP地址，允许点分十进制*/
	if ( phe = gethostbyname(host) )
		memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);
	else if ( (sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE )
		errexit("can't get \"%s\" host entry\n", host);

    /*将传输协议名称映射到协议号*/
	if ( (ppe = getprotobyname(transport)) == 0)
		errexit("can't get \"%s\" protocol entry\n", transport);

    /*使用协议选择套接字类型*/
	if (strcmp(transport, "udp") == 0)
		type = SOCK_DGRAM;
	else
		type = SOCK_STREAM;

    /*分配一个套接字*/
	s = socket(PF_INET, type, ppe->p_proto);
	if (s < 0)
		errexit("can't create socket: %s\n", strerror(errno));

    /* Connect the socket */
	if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		errexit("can't connect to %s.%s: %s\n", host, service,
			strerror(errno));
	return s;
}


