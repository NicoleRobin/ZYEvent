/*******************************************************************************
* File Name		: epoll_et.cpp
* Author		: zjw
* Email			: emp3XzA3MjJAMTYzLmNvbQo= (base64 encode)
* Create Time	: 2017年04月15日 星期三 18时33分00秒
*******************************************************************************/
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <queue>
#include <map>
#include <errno.h>
using namespace std;

const int SERVER_PORT = 8080;
const int RECV_SIZE = 1024;
const int SEND_SIZE = 1040;
const int MAX_EVENTS = 1024;
typedef struct ClientInfo
{
	ClientInfo()
	{
		bzero(&addr, sizeof(addr));
	}
	sockaddr_in addr;
	queue<string> queueBuf;
}ClientInfo;

int main(int argc, char **argv)
{
	int server = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in addrServer;
	bzero(&addrServer, sizeof(addrServer));
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(SERVER_PORT);
	addrServer.sin_addr.s_addr = INADDR_ANY;

	if (bind(server, (struct sockaddr*)&addrServer, sizeof(addrServer)) < 0)
	{
		perror("bind failed!");
		return -1;
	}

	listen(server, 5);
	cout << "server listen on port:" << SERVER_PORT << " ..." << endl;
	
	int epollFd = epoll_create(MAX_EVENTS);
	if (epollFd <= 0)
	{
		perror("epoll_create failed,");
		return -1;
	}

	// set non block
	fcntl(server, F_SETFL, O_NONBLOCK);
	
	struct epoll_event ev;
	ev.data.fd = server;
	ev.events = EPOLLIN;

	// register event
	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, server, &ev))
	{
		perror("epoll_ctl failed!");
		return -1;
	}

	struct epoll_event events[100];
	int ret = 0;
	char recvBuf[RECV_SIZE + 1];
	char sendBuf[SEND_SIZE + 1];
	int client;
	map<int, struct ClientInfo> mapClientInfo;
	while (1)
	{
		// wait epoll
		ret = epoll_wait(epollFd, events, 100, 500);
		// process active event
		for (int i = 0; i < ret; i++)
		{
			if (events[i].data.fd == server)
			{ // a new client connect
				sockaddr_in addrClient;
				socklen_t len = sizeof(addrClient);
				client = accept(server, (struct sockaddr*)&addrClient, &len);
				if (client < 0)
				{
					perror("accept failed!");
					return -1;
				}
				cout << "accept a new client:" << inet_ntoa(addrClient.sin_addr) << endl;
				fcntl(client, F_SETFL, O_NONBLOCK);
				ev.data.fd = client;
				ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
				epoll_ctl(epollFd, EPOLL_CTL_ADD, client, &ev);
				struct ClientInfo info;
				info.addr = addrClient;
				mapClientInfo.insert(make_pair<int, struct ClientInfo>(client, info));
			}
			else if (events[i].events & EPOLLIN)
			{ // connectting user and sth can be read
				if ((client = events[i].data.fd) < 0)
				{
					continue;
				}

				memset(recvBuf, 0, RECV_SIZE + 1);
				memset(sendBuf, 0, SEND_SIZE + 1);
				if ((ret = recv(client, recvBuf, RECV_SIZE, 0)) < 0)
				{ // client closed
					if (errno == ECONNRESET)
					{
						close(client);
						events[i].data.fd = -1;
						cout << "client[" << inet_ntoa(mapClientInfo[client].addr.sin_addr) << "] exit!" << endl;
						mapClientInfo.erase(client);
					}
					else
					{
						cout << "read from client[" << "ip" << "] failed!" << endl;
					}
				}
				else if (ret = 0)
				{
					close(client);
					events[i].data.fd = -1;
					cout << "client[" << inet_ntoa(mapClientInfo[client].addr.sin_addr) << "] exit!" << endl;
					mapClientInfo.erase(client);
				}

				cout << "client[" << inet_ntoa(mapClientInfo[client].addr.sin_addr) << "] said:" << recvBuf;
				sprintf(sendBuf, "Your said:%s", recvBuf);
				mapClientInfo[client].queueBuf.push(sendBuf);

				// set write event
				// ev.data.fd = client;
				// ev.events = EPOLLOUT;
				// epoll_ctl(epollFd, EPOLL_CTL_MOD, client, &ev);
			}
			else if (events[i].events & EPOLLOUT)
			{ // write event
				client = events[i].data.fd;
				if (!mapClientInfo[client].queueBuf.empty())
				{
					string strTemp = mapClientInfo[client].queueBuf.front();
					mapClientInfo[client].queueBuf.pop();
					if ((ret = send(client, strTemp.c_str(), strTemp.length(), 0) < 0))
					{
						perror("send failed!");
						return -1;
					}

				}
				// ev.data.fd = client;
				// ev.events = EPOLLIN;
				// epoll_ctl(epollFd, EPOLL_CTL_MOD, client, &ev);
			}
		}
	}

	close(server);
	return 0;
}
