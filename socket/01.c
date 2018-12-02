
#include <unistd.h>

#include <stdlib.h>

#include <string.h>

#include <stdio.h>

#include <errno.h>

#include <stdarg.h>

#include <sys/types.h>

#include <sys/socket.h>
//针对ECHO服务的TCP客户软件的实现
#include <netinet/in.h>

#include <arpa/inet.h>

#include <netdb.h>

#ifndef	INADDR_NONE

#define	INADDR_NONE	0xffffffff

#endif	/* INADDR_NONE */

int	TCPecho(const char *host, const char *service);

int	errexit(const char *format, ...);

int	connectTCP(const char *host, const char *service);

#define	LINELEN		128

int

main(int argc, char *argv[])

{

	char	*host = "localhost";	/* host to use if none supplied	*/

	char	*service = "echo";	/* default service name		*/



	switch (argc) {//处理命令行参数

	case 1:

		host = "localhost";//默认的地址是本机

		break;

	case 3:

		service = argv[2];

		/* FALL THROUGH */

	case 2:

		host = argv[1];

		break;

	default:

		fprintf(stderr, "usage: TCPecho [host [port]]\n");

		exit(1);

	}

	TCPecho(host, service);

	exit(0);

}



/*------------------------------------------------------------------------

* TCPecho - ECHO服务 将发送的数据发送回来

*------------------------------------------------------------------------

*/

int  TCPecho(const char *host, const char *service)//传入了地址和服务

{

	char	buf[LINELEN + 1];		/* buffer for one line of text	*/

	int	s, n;			/* socket 描述符, read 计数*/

	int	outchars, inchars;	/* 发送和接受的字数*/



	s = connectTCP(host, service);

	while (fgets(buf, sizeof(buf), stdin)) {

		buf[LINELEN] = '\0';	/* insure line null-terminated	*/

		outchars = strlen(buf);

		(void)write(s, buf, outchars);


		/* 回写 */ 

		for (inchars = 0; inchars < outchars; inchars += n) {

			n = read(s, &buf[inchars], outchars - inchars);

			if (n < 0)

				errexit("socket read failed: %s\n",strerror(errno));

		}

		fputs(buf, stdout);

	}

}



/* errexit.c - errexit */

/*------------------------------------------------------------------------

* errexit - print an error message and exit

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

/* connectTCP.c - connectTCP */



int connectsock(const char *host, const char *service,const char *transport);


int connectTCP(const char *host, const char *service)

{

	return connectsock(host, service, "tcp");

}

/* connectsock.c - connectsock */



int connectsock(const char *host, const char *service, const char *transport)


{

	struct hostent	*phe;	/* 指向主机信息的指针*/

	struct servent	*pse;	/* 指向服务信息的指针*/

	struct protoent *ppe;	/* 指向端口信息的指针*/

	struct sockaddr_in sin;	/* Internet端点地址*/

	int	s, type;	/* 套接字描述符和套接字类型*/


	memset(&sin, 0, sizeof(sin));

	sin.sin_family = AF_INET;//ipv4类型的协议


	/* Map service name to port number */

	if (pse = getservbyname(service, transport))//获得服务的端口

		sin.sin_port = pse->s_port;

	else if ((sin.sin_port = htons((unsigned short)atoi(service))) == 0)//转换字节序

		errexit("can't get \"%s\" service entry\n", service);



	/* Map host name to IP address, allowing for dotted decimal */

	if (phe = gethostbyname(host))//获得IP地址

		memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);

	else if ((sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE)

		errexit("can't get \"%s\" host entry\n", host);



	/* 将传输协议名称映射到协议号 */

	if ((ppe = getprotobyname(transport)) == 0)

		errexit("can't get \"%s\" protocol entry\n", transport);



	/* 使用协议选择套接字类型 */

	if (strcmp(transport, "udp") == 0)

		type = SOCK_DGRAM;

	else

		type = SOCK_STREAM;



	/*分配一个套接字 */

	s = socket(PF_INET, type, ppe->p_proto);

	if (s < 0) errexit("can't create socket: %s\n", strerror(errno));


	/* 连接套接字 */

	if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)

		errexit("can't connect to %s.%s: %s\n", host, service,strerror(errno));

	return s;

}

