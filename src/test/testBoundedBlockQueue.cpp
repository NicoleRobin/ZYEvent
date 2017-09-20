#include "BoundedBlockQueue.h"

#include <unistd.h>
#include <pthread.h>

#include <stdio.h>

ZY::BoundedBlockQueue<int> g_queue(50);

void* producer(void *arg)
{
	int i = 0;
	while (true)
	{
		printf("producer:put %d to queue\n", i);
		g_queue.Put(i++);
		printf("producer:queue.size() = %d\n", g_queue.Size());
		// g_queue.Dump();
		// sleep(1);
	}
}

void* consumer(void *arg)
{
	int i = 0;
	while (true)
	{
		i = g_queue.Get();
		printf("consumer:get %d from queue\n", i);
		printf("consumer:queue.size() = %d\n", g_queue.Size());
		// g_queue.Dump();
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
