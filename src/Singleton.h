#ifndef SINGLETON_H__
#define SINGLETON_H__

namespace ZY
{
	template <typename T>
	class Singleton<T>
	{
	public:
		T& instance()
		{
			static T instance;
			return instance;
		}
	private:
		Singleton();
		~Singleton();
	};
}
#endif /* SINGLETON_H__ */
