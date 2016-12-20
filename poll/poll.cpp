/*******************************************************************************
* File Name		: poll.cpp
* Author		: zjw
* Email			: emp3XzA3MjJAMTYzLmNvbQo= (base64 encode)
* Create Time	: 2015年07月17日 星期五 09时42分11秒
*******************************************************************************/
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
using namespace std;

const int SERVER_PORT = 8080;
const int EVENT_SIZE = 1024;
const int RECV_SIZE = 1024;
const int SEND_SIZE = 1040;

int main(int argc, char **argv)
{
	int server = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in addrServer;
	bzero(&addrServer, sizeof(addrServer));
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(SERVER_PORT);
	addrServer.sin_addr.s_addr = INADDR_ANY;

	if (bind(server, (struct sockaddr*)&addrServer, sizeof(addrServer)))
	{
		perror("bind failed,");
		return -1;
	}

	listen(server, 10);
	cout << "listening on port:" << SERVER_PORT << " ..." << endl;

	struct pollfd fds[EVENT_SIZE] = {{0, 0, 0}};
	fds[server].fd = server;
	fds[server].events = POLLIN;

	int ret = 0;
	char recvBuf[RECV_SIZE + 1];
	char sendBuf[SEND_SIZE + 1];
	while (1)
	{
		if ((ret = poll(fds, EVENT_SIZE, 0)) < 0)
		{
			perror("poll failed,");
			return -1;
		}

		for (int i = 0; i < EVENT_SIZE; i++)
		{
			if (fds[i].fd == 0)
			{
				continue;
			}

			if (fds[i].fd == server && (fds[i].revents & POLLIN))
			{ // accept a new client
				struct sockaddr_in addrClient;
				socklen_t len = sizeof(addrClient);
				int client = accept(server, (struct sockaddr*)&addrClient, &len);
				if (client == -1)
				{
					perror("accept failed,");
					return -1;
				}
				else
				{
					cout << "accep an new client " << client << ", ip:" << inet_ntoa(addrClient.sin_addr) << endl;
					fds[client].fd = client;
					fds[client].events = POLLIN;
				}
			}
			else if (fds[i].revents & POLLIN)
			{ // read event
				memset(recvBuf, 0, RECV_SIZE + 1);
				if ((ret = recv(fds[i].fd, recvBuf, RECV_SIZE, 0)) < 0)
				{
					perror("recv failed,");
					return -1;
				}
				else if (ret == 0)
				{ // client close
					close(fds[i].fd);
					fds[i].fd = 0;
				}
				else
				{ // recv data
					cout << "client " << fds[i].fd << " said:" << recvBuf << endl;	
					memset(sendBuf, 0, SEND_SIZE + 1);
					sprintf(sendBuf, "You said:%s", recvBuf);
					if ((ret = send(fds[i].fd, sendBuf, SEND_SIZE, 0)) < 0)
					{
						perror("send failed,");
						return -1;
					}
				}
			}
			else
			{ // write event
				
			}
		}
	}
	close(server);

	return 0;
}
