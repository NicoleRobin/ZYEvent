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

		void lock()
		{
			pthread_mutex_lock(mutex_);
		}

		void unlock()
		{
			pthread_mutex_unlock(&mutex_);
		}
	private:
		pthread_mutex_t mutex_;
		pid_t holder_;
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
			mutex_.lock();
		}

		~MutexLockGuard()
		{
			mutex_.unlock();
		}

	private:
		MutexLock &mutex_;
	};
}

#endif /* MUTEX_H__ */
