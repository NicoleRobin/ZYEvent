#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H

#include "Mutex.h"
#include "Condition.h"

#include <deque>

namespace ZY
{
	template<typename T>
	class BlockQueue
	{
	public:
		BlockQueue()
			:m_mutex(), 
			m_condNotEmpty(m_mutex), 
			m_queue()
		{}

		void Put(const T& val)
		{
			MutexLockGuard lock(m_mutex);
			m_queue.push_back(val);
			m_condNotEmpty.Notify();
		}

		T Get()
		{
			MutexLockGuard lock(m_mutex);
			
			while (m_queue.empty())
			{
				m_condNotEmpty.Wait();
			}

			T front(m_queue.front());
			m_queue.pop_front();
			return front;
		}

		int Size() const
		{
			MutexLockGuard lock(m_mutex);
			return m_queue.size();
		}

	private:
		mutable MutexLock m_mutex;
		Condition m_condNotEmpty;
		std::deque<T> m_queue;
	};
}

#endif /* BLOCK_QUEUE_H */
