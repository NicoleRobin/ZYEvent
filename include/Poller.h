#ifndef POLLER_H
#define POLLER_H

namespace ZY
{
	class Poller
	{
	public:
		// Poller类至少要包含一个Poll函数，该函数的目的是阻塞的等待关注的事件有效，并返回有效的事件
		void Poll();

		// 还应该包括添加、删除事件的功能
		bool AddEvent();
		bool ModEvent(); // 修改事件，不确定是否必要
		bool DelEvent();
	};
}

#endif /* POLLER_H */
