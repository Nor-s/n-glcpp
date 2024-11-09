#include "thread_manager.h"

namespace anim
{

ThreadManager::~ThreadManager()
{
	for (auto& thread : threads_)
	{
		if (thread.joinable())
			thread.join();
	}
	threads_.clear();
}

}	 // namespace anim
