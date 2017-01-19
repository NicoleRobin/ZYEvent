/*
 * g++ -o main ./main.cpp -lzylog -lzyevent
 */
#include <unistd.h>
#include <libgen.h>
#include <fcntl.h>
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
#include "zyevent.h"

using namespace std;

#define BZERO(data) { memset(&data, 0, sizeof(data)); }

#define ASSERT(expr) { if (!(expr)) { LOG_ERROR("ASSERT \"%s\" failed", #expr); }
#define ASSERT_RET(expr, args...) { if (!(expr)) { LOG_ERROR("ASSERT \"%s\" failed", #expr); return args; }}

void accept_cb(int fd, int events, void *arg);
void read_cb(int fd, int events, void *arg);
void write_cb(int fd, int events, void *arg);

int setnonblock(int fd)
{
	int iOldVal = fcntl(fd, F_GETFL, 0);
	int iNewVal = iOldVal | O_NONBLOCK;
	int iRet = fcntl(fd, F_SETFL, iNewVal);
	if (iRet == -1)
	{
		LOG_ERROR("errno[%d], error[%s]", errno, strerror(errno));
	}

	return iRet;
}

void accept_cb(int fd, int events, void *arg)
{
	struct event_base *ev_base = ((struct event*)arg)->ev_base;

	struct sockaddr_in cli_addr;
	BZERO(cli_addr);
	socklen_t cli_addr_len = sizeof(cli_addr);
	int client = accept(fd, (struct sockaddr*)&cli_addr, &cli_addr_len);
	ASSERT_RET(client);

	setnonblock(client);
	LOG_DEBUG("Accept new conn, [%s:%d]", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

	struct event *ev = event_new(ev_base, client, EPOLLIN, read_cb, ev_base);
	ASSERT_RET(ev != NULL);

	event_add(ev, NULL);
}

void read_cb(int fd, int events, void *arg)
{
	struct event *ev = (struct event*)arg;

	BZERO(ev->buf);
	ev->offset = 0;
	ev->len = 0;
	ev->len = recv(fd, ev->buf, BUF_SIZE, 0);
	if (ev->len == 0)
	{
		LOG_ERROR("Peer shutdown, close it");
		event_del(ev);
		delete ev;
		close(fd);
	}
	else if (ev->len == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			LOG_DEBUG("No data");
		}
		else
		{
			LOG_ERROR("errno:%d, error:%s", errno, strerror(errno));
			event_del(ev);
			delete ev;
			close(fd);
		}
	}
	else
	{
		// TrimStr(pEvent->buf);
		LOG_DEBUG("Recv %d bytes data, content:[%s]", ev->len, ev->buf);

		event_del(ev);
		event_assign(ev, ev->ev_base, fd, EPOLLOUT, write_cb, NULL);
		event_add(ev, NULL);
	}
}

void write_cb(int fd, int events, void *arg)
{
	struct event *ev = (struct event*)arg;
	int iRet = send(fd, ev->buf + ev->offset, ev->len, 0);
	if (iRet == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			LOG_ERROR("No space, try again");
		}
		else
		{
			LOG_ERROR("errno:%d, error:%s", errno, strerror(errno));
			event_del(ev);
			delete ev;
			close(fd);
		}
	}
	else if (iRet < ev->len)
	{
		LOG_ERROR("Space not enough, only send %d bytes data, all data length is %d", iRet, ev->len);
		ev->offset += iRet;
		ev->len -= iRet;
	}
	else
	{
		LOG_DEBUG("Send %d bytes data succ!", iRet);

		event_del(ev);
		event_assign(ev, ev->ev_base, fd, EPOLLIN, read_cb, NULL);
		event_add(ev, NULL);
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

	// set SOADDR_REUSE
	int iOptVal = 1;
	setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &iOptVal, sizeof(iOptVal));

	// set nonblock
	setnonblock(server);

	struct sockaddr_in addr;
	BZERO(addr);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(iPort);

	int iRet = bind(server, (struct sockaddr*)&addr, sizeof(addr));
	ASSERT_RET(iRet != -1, -1);

	iRet = listen(server, 5);
	ASSERT_RET(iRet != -1, -1);

	struct event_base *ev_base = event_base_new(500);
	ASSERT_RET(ev_base != NULL, -1);

	struct event *ev = event_new(ev_base, server, EPOLLIN, accept_cb, ev_base);
	ASSERT_RET(ev != NULL, -1);

	event_add(ev, NULL);

	event_base_loop(ev_base);

	return 0;
}
