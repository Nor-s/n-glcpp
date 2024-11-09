#ifndef ANIM_THREAD_THREAD_MANAGER_H
#define ANIM_THREAD_THREAD_MANAGER_H

#include <vector>
#include <thread>
#include <functional>
#include <utility>
#include <tuple>
#include <atomic>

namespace anim
{
class ITask
{
};

template <typename t_task, typename t_callback>
class Task : ITask
{
private:
	void work(t_task task, t_callback thread_end_callback)
	{
		if (threads_.size() < THREAD_MAX_COUNT)
		{
			thread_end_callbacks_.emplace_back((size_t) threads_.size(), std::atomic_bool(false), thread_end_callback);
			;
			threads_.emplace_back([task, thread_end_callback]() { task(); });
		}
	}

private:
	std::vector<std::tuple<size_t, std::atomic_bool, t_callback>> thread_end_callbacks_;
};

class ThreadManager final
{
private:
	const size_t THREAD_MAX_COUNT = 4;

public:
	~ThreadManager();
	ThreadManager(const ThreadManager&) = delete;
	ThreadManager& operator=(const ThreadManager&) = delete;
	ThreadManager(ThreadManager&&) = delete;
	ThreadManager& operator=(ThreadManager&&) = delete;

	static ThreadManager& get_instance()
	{
		static ThreadManager instance;
		return instance;
	}

private:
	ThreadManager() = default;

private:
	std::vector<std::thread> threads_;
};

}	 // namespace anim

#endif