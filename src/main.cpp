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

#include <map>
#include <string>

#include "ZYLog.h"

using namespace std;

#define BZERO(data) { memset(&data, 0, sizeof(data)); }

#define ASSERT(expr) { if (!(expr)) { LOG_ERROR("ASSERT \"%s\" failed", #expr); }
#define ASSERT_RET(expr, args...) { if (!(expr)) { LOG_ERROR("ASSERT \"%s\" failed", #expr); return args; }}

typedef void (*CallBack)(int epfd, int fd, int events, void *arg);

const int MAX_SIZE = 1024;

struct event
{
	int fd;
	int events;
	char buf[MAX_SIZE];
	int offset;
	int bufLen;
	CallBack cb;
};

void TrimStr(char *str);
void accept_cb(int epfd, int fd, int events, void *arg);
void read_cb(int epfd, int fd, int events, void *arg);
void write_cb(int epfd, int fd, int events, void *arg);

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

void accept_cb(int epfd, int fd, int events, void *arg)
{
	struct sockaddr_in cli_addr;
	BZERO(cli_addr);
	socklen_t cli_addr_len = sizeof(cli_addr);
	int client = accept(fd, (struct sockaddr*)&cli_addr, &cli_addr_len);
	ASSERT_RET(client);

	struct event *pEvent = new event;
	pEvent->fd = client;
	pEvent->events = EPOLLIN | EPOLLOUT;
	pEvent->cb = read_cb;

	epoll_event event;
	event.events = EPOLLIN;
	event.data.ptr = pEvent;

	epoll_ctl(epfd, EPOLL_CTL_ADD, client, &event);
}

void read_cb(int epfd, int fd, int events, void *arg)
{
	struct event *pEvent = (struct event*)arg;
	BZERO(pEvent->buf);
	pEvent->offset = 0;
	pEvent->bufLen = 0;
	pEvent->bufLen = recv(fd, pEvent->buf, MAX_SIZE, 0);
	if (pEvent->bufLen == 0)
	{
		LOG_ERROR("Peer shutdown, close it");
		epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
		close(fd);
	}
	else if (pEvent->bufLen == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			LOG_DEBUG("No data");
		}
		else
		{
			LOG_ERROR("errno:%d, error:%s", errno, strerror(errno));
			epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
			close(fd);
		}
	}
	else
	{
		TrimStr(pEvent->buf);
		LOG_DEBUG("Recv %d bytes data, content:[%s]", pEvent->bufLen, pEvent->buf);

		epoll_event event;
		BZERO(event);
		event.events = EPOLLOUT;
		event.data.ptr = pEvent;
		pEvent->cb = write_cb;
		epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &event);
	}
}

void write_cb(int epfd, int fd, int events, void *arg)
{
	struct event *pEvent = (struct event*)arg;
	int iRet = send(fd, pEvent->buf + pEvent->offset, pEvent->bufLen, 0);
	if (iRet == 0)
	{
		LOG_ERROR("Peer shutdown, close it");
		epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
		close(fd);
	}
	else if (iRet == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			LOG_ERROR("No space, try again");
		}
		else
		{
			LOG_ERROR("errno:%d, error:%s", errno, strerror(errno));
			epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
			close(fd);
		}
	}
	else if (iRet < pEvent->bufLen)
	{
		LOG_ERROR("Space not enough, only send %d bytes data, all data length is %d", iRet, pEvent->bufLen);
		pEvent->offset = iRet;
		pEvent->bufLen = pEvent->bufLen - iRet;
	}
	else
	{
		LOG_DEBUG("Send %d bytes data succ!", iRet);

		epoll_event event;
		BZERO(event);
		event.events = EPOLLIN;
		event.data.ptr = pEvent;
		pEvent->cb = read_cb;
		epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &event);
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

	struct event *pEvent = new event;
	BZERO(*pEvent);
	pEvent->fd = server;
	pEvent->events = EPOLLIN;
	pEvent->cb = accept_cb;

	epoll_event event;
	event.events = EPOLLIN;
	event.data.ptr = pEvent;

	epoll_ctl(epfd, EPOLL_CTL_ADD, server, &event);

	map<int, string> mapSndBuf;
	while (true)
	{
		epoll_event events[500];
		int fds = epoll_wait(epfd, events, 500, -1);
		ASSERT_RET(fds != -1, -1);
		LOG_DEBUG("fds[%d]", fds);

		for (int i = 0; i < fds; ++i)
		{
			struct event *pEvent = (struct event*)events[i].data.ptr;
			if ((events[i].events & EPOLLIN) && (pEvent->events & EPOLLIN))
			{
				pEvent->cb(epfd, pEvent->fd, events[i].events, pEvent);
			}
			else if ((events[i].events & EPOLLOUT) && (pEvent->events & EPOLLOUT))
			{
				pEvent->cb(epfd, pEvent->fd, events[i].events, pEvent);
			}
			/*
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

				mapSndBuf[fd] = buf;
				TrimStr(buf);
				LOG_DEBUG("Recv [%d] bytes data, content[%s]", iRet, buf);

				event.events = EPOLLOUT;
				event.data.fd = fd;
				epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &event);
			}
			else if (events[i].events & EPOLLOUT)
			{ // write event
				string strSndBuf = mapSndBuf[fd];
				int iRet = send(fd, strSndBuf.c_str(), strSndBuf.length(), 0);
				if (iRet == -1)
				{
					if (errno == EAGAIN || errno == EWOULDBLOCK)
					{
						LOG_DEBUG("No space");
						continue;
					}
				}
				else if (iRet < strSndBuf.length())
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
			*/
		}
	}

	close(server);
	close(epfd);

	return 0;
}
