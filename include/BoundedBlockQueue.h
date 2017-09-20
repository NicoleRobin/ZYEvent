#ifndef BOUNDED_BLOCK_QUEUE_H
#define BOUNDED_BLOCK_QUEUE_H

#include <deque>
#include <iostream>
#include <iterator>

#include "Mutex.h"
#include "Condition.h"

namespace ZY
{
	template<typename T>
	class BoundedBlockQueue
	{
	public:
		BoundedBlockQueue(int iCapacity)
			: m_iCapacity(iCapacity),  
			m_mutex(), 
			m_condNotFull(m_mutex), 
			m_condNotEmpty(m_mutex)
		{ }

		~BoundedBlockQueue()
		{ }

		void Put(const T &val)
		{ 
			MutexLockGuard lock(m_mutex);
			while (m_queue.size() > m_iCapacity)
			{
				m_condNotFull.Wait();
			}

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
			m_condNotFull.Notify();
			return front;
		}

		int Size() const
		{
			MutexLockGuard lock(m_mutex);
			return m_queue.size();
		}

		void Dump()
		{
			MutexLockGuard lock(m_mutex);
			std::copy(m_queue.begin(), m_queue.end(), std::ostream_iterator<int>(std::cout, " "));
			std::cout << std::endl;
		}
	private:
		int m_iCapacity;
		mutable MutexLock m_mutex;
		Condition m_condNotFull;
		Condition m_condNotEmpty;
		std::deque<T> m_queue;
	};
}
#endif /* BOUNDED_BLOCK_QUEUE_H */
