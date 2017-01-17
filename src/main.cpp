/*
 * g++ -o main ./main.cpp -lzylog
 */
#include <unistd.h>
#include <libgen.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "ZYLog.h"

#define BZERO(data) { memset(&data, 0, sizeof(data)); }

#define ASSERT(expr) { if (!(expr)) { LOG_ERROR("ASSERT \"%s\" failed", #expr); }
#define ASSERT_RET(expr, args...) { if (!(expr)) { LOG_ERROR("ASSERT \"%s\" failed", #expr); return args; }}

void TrimStr(char *str)
{
	int iLen = strlen(str);
	for (int i = iLen - 1; i >= 0; --i)
	{
		if (!isspace(str[i]))
		{
			break;
		}
		str[i] = '\0';
	}
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		printf("Usage:%s port\n", basename(argv[0]));
		return -1;
	}

	int iPort = atoi(argv[1]);
	LOG_DEBUG("Port[%d]", iPort);

	int server = socket(AF_INET, SOCK_STREAM, 0);
	ASSERT_RET(server, -1);

	struct sockaddr_in addr;
	BZERO(addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(iPort);

	int iRet = bind(server, (struct sockaddr*)&addr, sizeof(addr));
	ASSERT_RET(iRet != -1, -1);

	iRet = listen(server, 5);
	ASSERT_RET(iRet != -1, -1);

	int epfd = epoll_create(500);
	ASSERT_RET(epfd != -1, -1);

	epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = server;

	epoll_ctl(epfd, EPOLL_CTL_ADD, server, &event);

	epoll_event events[500];
	while (true)
	{
		int fds = epoll_wait(epfd, events, 500, -1);
		ASSERT_RET(fds != -1, -1);
		LOG_DEBUG("fds[%d]", fds);

		for (int i = 0; i < fds; ++i)
		{
			int fd = events[i].data.fd;
			if (fd == server)
			{ // accept new conn
				struct sockaddr_in cli_addr;
				socklen_t cli_addr_len = sizeof(cli_addr);
				int client = accept(fd, (struct sockaddr*)&cli_addr, &cli_addr_len);
				if (client == -1)
				{
					LOG_ERROR("accept, errno[%d], error[%s]", errno, strerror(errno));
					continue;
				}

				LOG_DEBUG("Accept new conn, [%s:%d]", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
				event.events = EPOLLIN;
				event.data.fd = client;
				epoll_ctl(epfd, EPOLL_CTL_ADD, client, &event);
			}
			else if (events[i].events & EPOLLIN)
			{ // read event
				char buf[1024];
				BZERO(buf);
				int iRet = recv(fd, buf, 1024, 0);
				if (iRet == 0)
				{ // peer shutdown
					LOG_DEBUG("fd[%d] peer shutdown, close it", fd);
					epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
					close(fd);
					continue;
				}
				else if (iRet == -1)
				{
					if (errno == EAGAIN || errno == EWOULDBLOCK)
					{
						LOG_DEBUG("No data");
						continue;
					}
				}

				TrimStr(buf);
				LOG_DEBUG("Recv [%d] bytes data, content[%s]", iRet, buf);

				event.events = EPOLLOUT;
				event.data.fd = fd;
				epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &event);
			}
			else if (events[i].events & EPOLLOUT)
			{ // write event
				char buf[1024] = "Recv data!\n";
				int iRet = send(fd, buf, strlen(buf), 0);
				if (iRet == -1)
				{
					if (errno == EAGAIN || errno == EWOULDBLOCK)
					{
						LOG_DEBUG("No space");
						continue;
					}
				}
				else if (iRet < strlen(buf))
				{ // space is not not enough, should continue send the last data
					LOG_ERROR("Space is not enough");
					continue;
				}
				else
				{
					LOG_DEBUG("Send succ!");
				}

				event.events = EPOLLIN;
				event.data.fd = fd;
				epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &event);
			}
		}
	}

	close(server);
	close(epfd);

	return 0;
}
