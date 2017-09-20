#ifndef MUTEX_H__
#define MUTEX_H__

#include <pthread.h>

namespace ZY
{
	/*
	used as a data member of a class
	class foo
	{
	public:
		int size() const;
	private:
		MutexLock mutex_;
		std::vector<int> data_; // guarded by mutex_
	}
	*/
	class MutexLock
	{
	public:
		MutexLock()
			:holder(0)
		{
			pthread_mutex_init(&mutex_, NULL);
		}

		~MutexLock()
		{
			pthread_mutex_destroy(&mutex_);
		}

		void Lock()
		{
			pthread_mutex_lock(mutex_);
		}

		void Unlock()
		{
			pthread_mutex_unlock(&mutex_);
		}

		pthread_mutex_t* GetPthreadMutex()
		{
			return &m_mutex;
		}
	private:
		pthread_mutex_t m_mutex;
		pid_t m_holder;
	};

	/*
	use as a stack variable
	int foo::size() const
	{
		MutexLockGuard lock(mutex_);
		return data_.size();
	}
	*/
	class MutexLockGuard
	{
	public:
		explicit MutexLockGuard(MutexLock &mutex)
			:mutex_(mutex)
		{
			mutex_.Lock();
		}

		~MutexLockGuard()
		{
			mutex_.Unlock();
		}

	private:
		MutexLock &mutex_;
	};
}

#endif /* MUTEX_H__ */
