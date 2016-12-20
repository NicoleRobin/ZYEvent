/*******************************************************************************
* File Name		: select.cpp
* Author		: zjw
* Email			: emp3XzA3MjJAMTYzLmNvbQo= (base64 encode)
* Create Time	: 2015年07月15日 星期三 11时52分07秒
*******************************************************************************/

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <map>
using namespace std;

const int SERVER_PORT = 8080;
const int FD_SIZE = 1024;
const int RECV_SIZE = 1024;
const int SEND_SIZE = 1040;

int main(int argc, char **argv)
{
	int server = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in addr_server;
	addr_server.sin_family = AF_INET;
	addr_server.sin_port = htons(SERVER_PORT);
	addr_server.sin_addr.s_addr = INADDR_ANY;
	memset(addr_server.sin_zero, 0, sizeof(addr_server.sin_zero));

	if (bind(server, (struct sockaddr*)&addr_server, sizeof(addr_server)))
	{
		perror("bind failed!");
		return -1;
	}

	listen(server, 5);

	fd_set fdsr;
	int fd[FD_SIZE] = { 0 };
	int maxsock = server;
	char recvBuf[RECV_SIZE + 1];
	char sendBuf[SEND_SIZE + 1];
	map<int, sockaddr_in> mapClients;

	while (1)
	{
		FD_ZERO(&fdsr);
		FD_SET(server, &fdsr);

		struct timeval tv;
		tv.tv_sec = 5;
		tv.tv_usec = 0;
	
		for (int i = 0; i < 1024; i++)
		{
			if (fd[i] != 0)
			{
				FD_SET(fd[i], &fdsr);
			}
		}

		int ret = select(maxsock + 1, &fdsr, NULL, NULL, &tv);
		if (ret == -1)
		{
			perror("select error!");
			return -1;
		}
		else if (ret == 0)
		{
			cout << "timeout!" << endl;
			continue;
		}
		
		// check every fd in the fdset
		for (int i = 0; i < FD_SIZE; i++)
		{
			if (FD_ISSET(fd[i], &fdsr))
			{
				memset(recvBuf, 0, RECV_SIZE + 1);
				memset(sendBuf, 0, SEND_SIZE + 1);
				ret = recv(fd[i], recvBuf, RECV_SIZE, 0);
				sprintf(sendBuf, "Your said:%s", recvBuf);
				send(fd[i], sendBuf, SEND_SIZE, 0);
				if (ret <= 0)
				{
					cout << "client " << fd[i] << " close" << endl;
					mapClients.erase(fd[i]);
					close(fd[i]);
					FD_CLR(fd[i], &fdsr);
					fd[i] = 0;
				}
				else
				{
					cout << "client " << fd[i] << " ip[" << inet_ntoa(mapClients[fd[i]].sin_addr) << "]" << " said:" << recvBuf << endl;
				}
			}
		}

		// check server socket
		if (FD_ISSET(server, &fdsr))
		{
			int client;
			sockaddr_in addr_client;
			socklen_t len = sizeof(addr_client);
			client = accept(server, (struct sockaddr*)&addr_client, &len);
			if (client == -1)
			{
				perror("accept error!");
				return -1;
			}

			for (int i = 0; i < FD_SIZE; i++)
			{
				if (fd[i] == 0)
				{
					fd[i] = client;
					mapClients.insert(make_pair<int, sockaddr_in>(client, addr_client));
					break;
				}
			}
		}
		maxsock = server;
		for (int i = 0; i < FD_SIZE; i++)
		{
			if (fd[i] > maxsock)
			{
				maxsock = fd[i];
			}
		}
	}
	close(server);

	return 0;
}
