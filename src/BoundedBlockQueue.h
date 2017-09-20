#ifndef BOUNDED_BLOCK_QUEUE_H
#define BOUNDED_BLOCK_QUEUE_H

#include <deque>

namespace ZY
{
	template<typename T>
	class BoundedBlockQueue
	{
	public:
		BoundedBlockQueue(int iCapacity)
			: m_iCapacity(iCapacity),  
			m_mutex(), 
			m_cond(mutex), 
			m_queue(m_iCapacity)
		{ }

		~BoundedBlockQueue()
		{ }

		void Put(const T &val)
		{ 
			MutexLockGuard(m_mutex);
			while (m_queue.size() > m_iCapacity)
			{
				m_condNotFull.wait();
			}

			m_queue.push_back(val);
			m_condNotEmpty.notify();
		}

		T Get()
		{
			MutexLockGuard(m_mutex);
			while (m_queue.empty())
			{
				m_condNotEmpty.wait();
			}

			T front(m_queue.front());
			m_queue.pop_front();
			m_condNotFull.notify();
			return front;
		}

		int Size() const
		{
			MutexLockGuard(m_mutex);
			return m_queue.size();
		}
	privae:
		m_iCapacity;
		MutexLock m_mutex;
		Condition m_cond;
		std::deque<T> m_queue;
	};
}
#endif /* BOUNDED_BLOCK_QUEUE_H */
