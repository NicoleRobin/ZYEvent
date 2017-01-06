#include <sys/epoll.h>
#include <errno.h>

#define BZERO(data) bzero(&data, sizeof(data));

const int BACK_LOG = 5;
const int MAX_EVENT = 500;

typedef void (*call_back)(int fd, int events, void *arg);

const int BUF_SIZE = 1024;
struct event
{
	int fd;
	call_back func;
	int events;
	void *arg;
	char buf[BUF_SIZE + 1];
};

void Usage()
{
	printf("Usage: echo port\n");
}

void AccecptConn(int fd, int events, void *arg)
{
}

void RecvData(int fd, int events, void *arg)
{
}

void SendData(int fd, int events, void *arg)
{

}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		Usage();
		return -1;
	}


	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1)
	{
		perror("socket");
		return -1;
	}

	struct sockaddr_in addr;
	BZERO(addr);
	addr.sin_family = AF_INET;
	addr.sin_addr = SOCKADDR_ANY;
	addr.sin_port = htons(port);

	if (0 != bind(fd, (struct sockaddr*)&addr, sizeof(addr)))
	{
		perror("bind");
		return -1;
	}

	if (0 != listen(fd, BACK_LOG))
	{
		perror("listen");
		return -1;
	}

	printf("Listening on port:%d\n", port);

	int epfd = epoll_create(MAX_EVENT);
	if (epfd == -1)
	{
		perror("epoll_create");
		return -1;
	}
	struct epoll_event events[MAX_EVENT];
	

	while()
	{
		int fds = epoll_wait(epfd, &events, MAX_EVENT, -1);
		if (fds < 0)
		{
			perror("epoll_wait");
			break;
		}
		for (int i = 0; i < fds; ++i)
		{
			event *pEvent = events[i].data.ptr;
			if ((events[i].events & EVENTIN) && (pEvent->events & EVENTIN))
			{
				
			}
			if ((events[i].events & EVENTOUT) && (pEvent->events & EVENTOUT))
			{

			}
		}
	}


	return 0;
}
