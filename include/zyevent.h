#ifndef ZY_EVENT_H__
#define ZY_EVENT_H__

#define MAX_EVENT 10000
#define BUF_SIZE 1024

typedef void (*callback)(int fd, int events, void *arg);

struct event_base;
struct event
{
	struct event_base *ev_base;
	int fd;
	int events;
	callback cb;
	void *arg;
	char buf[BUF_SIZE];
	int offset, len;
};

struct event_base
{
	int epfd;
};

struct event* event_new(struct event_base *ev_base, int fd, int events, callback cb, void *arg);
int event_assign(struct event *ev, struct event_base *ev_base, int fd, int events, callback cb, void *arg);

int event_add(struct event *ev, const struct timeval *tv);
int event_del(struct event *ev);

struct event_base* event_base_new(int max_event);
int event_base_loop(struct event_base *pEventBase, int timeout = -1);

#endif /* ZY_EVENT_H__ */

