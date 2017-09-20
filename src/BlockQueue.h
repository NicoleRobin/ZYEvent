#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H

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
			MutexLockGuard(m_mutex);
			m_queue.push_back(val);
			m_condNotEmpty().notify();
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
			return front;
		}

		int Size() const
		{
			MutexLockGuard(m_mutex);
			return m_queue.size();
		}

	private:
		MutexLock m_mutex;
		Condition m_condNotEmpty;
		std::deque<T> m_queue;
	};
}

#endif /* BLOCK_QUEUE_H */
