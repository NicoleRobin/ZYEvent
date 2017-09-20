#include "BlockQueue.h"

#include <unistd.h>
#include <pthread.h>

#include <stdio.h>

ZY::BlockQueue<int> g_queue;

void* producer(void *arg)
{
	int i = 0;
	while (true)
	{
		printf("put %d to queue\n", i);
		g_queue.Put(i++);
		printf("queue.size() = %d\n", g_queue.Size());
		sleep(1);
	}
}

void* consumer(void *arg)
{
	int i = 0;
	while (true)
	{
		i = g_queue.Get();
		printf("get %d from queue\n", i);
		printf("queue.size() = %d\n", g_queue.Size());
		sleep(1);
	}
}

int main(void)
{
	pthread_t pid_producer, pid_consumer;
	pthread_create(&pid_producer, NULL, producer, NULL);
	pthread_create(&pid_consumer, NULL, consumer, NULL);
	
	pthread_join(pid_consumer, NULL);
	pthread_join(pid_consumer, NULL);

	return 0;
}
