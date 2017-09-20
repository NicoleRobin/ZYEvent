#ifndef CONDITION_H
#define CONDITION_H

#include <pthread.h>

#include "Mutex.h"

namespace ZY
{
	class Condition
	{
	public:
		Condition(MutexLock &mutex)
			: m_mutex(mutex)
		{
			pthread_cond_init(&m_cond, NULL);
		}

		~Condition()
		{
			pthread_cond_destroy(&m_cond);
		}

		void Wait()
		{
			pthread_cond_wait(&m_cond, m_mutex.GetPthreadMutex());	
		}

		void Notify()
		{
			pthread_cond_signal(&m_cond);
		}

		void NotifyAll()
		{
			pthread_cond_broadcast(&m_cond);
		}

	private:
		MutexLock& m_mutex;
		pthread_cond_t m_cond;
	};
}

#endif /* CONDITION_H */
