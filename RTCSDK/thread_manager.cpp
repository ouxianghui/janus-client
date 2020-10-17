/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#include "thread_manager.h"
#include <memory>
#include <mutex>
#include "logger/logger.h"

namespace vi {
	ThreadManager::ThreadManager()
	{

	}

	ThreadManager::~ThreadManager()
	{
		DLOG("~ThreadManager()");
		if (!_destroy) {
			stopAll();
		}
	}

	void ThreadManager::init()
	{
		std::lock_guard<std::mutex> lock(_mutex);

		_mainThread = rtc::ThreadManager::Instance()->CurrentThread();

		_threadsMap[ThreadName::WORKER] = rtc::Thread::Create();
		_threadsMap[ThreadName::WORKER]->SetName("vi::WORKER", nullptr);

		_threadsMap[ThreadName::SERVICE] = rtc::Thread::Create();
		_threadsMap[ThreadName::SERVICE]->SetName("vi::SERVICE", nullptr);

		for (const auto& thread : _threadsMap) {
			thread.second->Start();
		}

		_destroy = false;
	}

	void ThreadManager::destroy()
	{

	}

	void ThreadManager::stopAll()
	{
		std::lock_guard<std::mutex> lock(_mutex);

		for (const auto& thread : _threadsMap) {
			thread.second->Stop();
		}
		_threadsMap.clear();

		_destroy = true;
	}

	rtc::Thread* ThreadManager::thread(ThreadName name)
	{
		std::lock_guard<std::mutex> lock(_mutex);

		if (name == ThreadName::MAIN) {
			return _mainThread;
		} 
		else if (_threadsMap.find(name) != _threadsMap.end()) {
			return _threadsMap[name].get();
		}

		return nullptr;
	}
}