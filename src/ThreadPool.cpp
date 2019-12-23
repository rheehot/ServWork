#include "ThreadPool.h"
#include "Exception.h"

ThreadPool::ThreadPool()
		: threads(), tasks(), cv(), taskMutex(), isEnd(false)
{
		auto threadNum = std::thread::hardware_concurrency();
		if (threadNum == 0)
		{
			throw ThreadException{ "Hardware Concurrency is 0" };
		}

		threads.reserve(threadNum);

		while (threadNum--)
			  threads.emplace_back([this]() { ThreadWork(); });
}

ThreadPool::~ThreadPool()
{
		isEnd = true;
		cv.notify_all();

		for (auto& t : threads)
		    t.join();
}

void ThreadPool::ThreadWork() noexcept
{
		while (true)
		{
			  std::unique_lock<std::mutex> lock{ taskMutex };
			  cv.wait(lock, [this]() { return !tasks.empty() || isEnd; });
			  if (isEnd && tasks.empty()) return;

			  auto&& task = Move(tasks.front());
			  tasks.pop();
			  lock.unlock();
			  task();
		}
}
