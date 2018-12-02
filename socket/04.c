/* 用于TIME服务的UDP服务器 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>


int	passiveUDP(const char *service);
int	errexit(const char *format, ...);

#define	UNIXEPOCH	2208988800UL	/*UNIX纪元，UCT秒	*/

/*------------------------------------------------------------------------
 * main - Iterative UDP server for TIME service
 *------------------------------------------------------------------------
 */
int
main(int argc, char *argv[])
{
	struct sockaddr_in fsin;	/* 来自客户的地址*/
	char	*service = "time";	/*服务名称或端口号*/
	char	buf[1];			/* “输入”缓冲区any size > 0*/
	int	sock;			/*服务器套接字*/
	time_t	now;			/*当前时间*/
	unsigned int	alen;		/*从地址长度*/

	switch (argc) {
	case	1:
		break;
	case	2:
		service = argv[1];
		break;
	default:
		errexit("usage: UDPtimed [port]\n");
	}

	sock = passiveUDP(service);

	while (1) {
		alen = sizeof(fsin);
		if (recvfrom(sock, buf, sizeof(buf), 0,(struct sockaddr *)&fsin, &alen) < 0)
			errexit("recvfrom: %s\n", strerror(errno));
		(void) time(&now);
		now = htonl((unsigned long)(now + UNIXEPOCH));//计算时间
		(void) sendto(sock, (char *)&now, sizeof(now), 0,(struct sockaddr *)&fsin, sizeof(fsin));
		//发送到客户端
	}
}
/* passiveUDP.c - passiveUDP */

int	passivesock(const char *service, const char *transport,
		int qlen);

/*------------------------------------------------------------------------
 * passiveUDP -创建一个在UDP服务器中使用的被动套接字
 *------------------------------------------------------------------------
 */
int passiveUDP(const char *service)
/*
 * Arguments:
 *      service - 与所需端口关联的服务
 */
{
	return passivesock(service, "udp", 0);
}


/* errexit.c - errexit */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

/*------------------------------------------------------------------------
 * errexit - print an error message and exit
 *------------------------------------------------------------------------
 */
int errexit(const char *format, ...)
{
	va_list	args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	exit(1);
}



int	errexit(const char *format, ...);

unsigned short	portbase = 0;	/* port base, for non-root servers	*/

/*------------------------------------------------------------------------
 * passivesock -使用TCP或UDP分配和绑定服务器套接字
 *------------------------------------------------------------------------
 */
int passivesock(const char *service, const char *transport, int qlen)
/*
 * Arguments:
 *      service   - 与所需端口关联的服务
 *      transport - 要使用的传输协议（“tcp”或“udp”）
 *      qlen      - 最大服务器请求队列长度
 */
{
	struct servent	*pse;	/*指向服务信息条目的指针*/
	struct protoent *ppe;	/*指向协议信息条目的指针*/
	struct sockaddr_in sin;	/*Internet端点地址*/
	int	s, type;	/*套接字描述符和套接字类型*/

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;

    /*将服务名称映射到端口号*/
	if ( pse = getservbyname(service, transport) )
		sin.sin_port = htons(ntohs((unsigned short)pse->s_port)
			+ portbase);
	else if ((sin.sin_port=htons((unsigned short)atoi(service))) == 0)
		errexit("can't get \"%s\" service entry\n", service);

    /*将协议名称映射到协议号*/
	if ( (ppe = getprotobyname(transport)) == 0)
		errexit("can't get \"%s\" protocol entry\n", transport);

    /* 使用协议选择套接字类型*/
	if (strcmp(transport, "udp") == 0)
		type = SOCK_DGRAM;
	else
		type = SOCK_STREAM;

    /*分配一个套接字*/
	s = socket(PF_INET, type, ppe->p_proto);
	if (s < 0)
		errexit("can't create socket: %s\n", strerror(errno));

    /*绑定套接字*/
	if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		errexit("can't bind to %s port: %s\n", service,strerror(errno));
	if (type == SOCK_STREAM && listen(s, qlen) < 0)
		errexit("can't listen on %s port: %s\n", service,strerror(errno));
	return s;
}

