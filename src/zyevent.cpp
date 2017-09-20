#include "zyevent.h"

#include <sys/epoll.h>
#include <sys/types.h>

#include <string.h>
#include <errno.h>

struct event* event_new(struct event_base *ev_base, int fd, int events, callback cb, void *arg)
{
	struct event *ev = new event;
	memset(ev, 0, sizeof(*ev));
	ev->ev_base = ev_base;
	ev->fd = fd;
	ev->events = events;
	ev->cb = cb;
	ev->arg = arg;

	return ev;
}

int event_assign(struct event *ev, struct event_base *ev_base, int fd, int events, callback cb, void *arg)
{
	ev->ev_base = ev_base;
	ev->fd = fd;
	ev->events = events;
	ev->cb = cb;
	ev->arg = arg;
}

int event_add(struct event *ev, const struct timeval *tv)
{
	struct event_base *ev_base = ev->ev_base;
	struct epoll_event epev;
	epev.events = ev->events;
	epev.data.ptr = ev;
	epoll_ctl(ev_base->epfd, EPOLL_CTL_ADD, ev->fd, &epev);
}

int event_del(struct event *ev)
{
	struct event_base *ev_base = ev->ev_base;
	epoll_ctl(ev_base->epfd, EPOLL_CTL_DEL, ev->fd, NULL);
}

struct event_base* event_base_new(int max_event)
{
	int epfd = epoll_create(max_event);
	if (epfd == -1)
	{
		return NULL;
	}

	struct event_base *ev_base = new event_base;
	memset(ev_base, 0, sizeof(*ev_base));
	ev_base->epfd = epfd;

	return ev_base;
}

int event_base_loop(struct event_base *ev_base, int timeout)
{
	struct epoll_event events[MAX_EVENT];
	if (ev_base == NULL)
	{
		return -1;
	}

	while (true)
	{
		int fds = epoll_wait(ev_base->epfd, events, MAX_EVENT, timeout);
		if (fds == -1)
		{
			return errno;
		}

		for (int i = 0; i < fds; ++i)
		{
			struct event *ev = (struct event*)events[i].data.ptr;
			ev->cb(ev->fd, events[i].events, ev);
		}
	}

	return 0;
}
