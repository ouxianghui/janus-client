/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#include "thread_provider.h"
#include <memory>
#include <mutex>
#include "rtc_base/win32_socket_init.h"
#include "rtc_base/physical_socket_server.h"
#include "logger/logger.h"

namespace vi {
	ThreadProvider::ThreadProvider() : _destroy(true), _inited(false)
	{

	}

	ThreadProvider::~ThreadProvider()
	{
		DLOG("~ThreadProvider()");
		if (!_destroy) {
			stopAll();
		}
	}

	void ThreadProvider::init()
	{
		std::lock_guard<std::mutex> lock(_mutex);

		_mainThread = rtc::ThreadManager::Instance()->CurrentThread();

		_inited = true;
	}

	void ThreadProvider::destroy()
	{
		_destroy = true;
	}

	void ThreadProvider::create(const std::list<std::string>& threadNames)
	{
		std::lock_guard<std::mutex> lock(_mutex);

		if (!_inited) {
			DLOG("_inited == false");
			return;
		}

		for (const auto& name : threadNames) {
			_threadsMap[name] = rtc::Thread::Create();
			_threadsMap[name]->SetName(name, nullptr);
			_threadsMap[name]->Start();
		}
	}

	void ThreadProvider::stopAll()
	{
		std::lock_guard<std::mutex> lock(_mutex);

		for (const auto& thread : _threadsMap) {
			thread.second->Stop();
		}
		_threadsMap.clear();

		_destroy = true;
	}

	rtc::Thread* ThreadProvider::thread(const std::string& name)
	{
		std::lock_guard<std::mutex> lock(_mutex);

		if (name == "main") {
			return _mainThread;
		} 
		else if (_threadsMap.find(name) != _threadsMap.end()) {
			return _threadsMap[name].get();
		}

		return nullptr;
	}
}