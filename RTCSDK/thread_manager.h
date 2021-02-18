/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include "rtc_base/thread.h"
#include <unordered_map>
#include <mutex>
#include "utils/singleton.h"
#include <atomic>

namespace vi {
	enum class ThreadName : int{
		MAIN = 0,
		WORKER = 1,
		SERVICE = 2,
		MEDIA_STREAM = 3
	};

	class ThreadManager : public core::Singleton<ThreadManager>
	{
	public:
		~ThreadManager();

		void init();

		void destroy();

		rtc::Thread* thread(ThreadName name);

	private:
		ThreadManager();

		ThreadManager(const ThreadManager&) = delete;

		ThreadManager(ThreadManager&&) = delete;

		ThreadManager& operator=(const ThreadManager&) = delete;

	private:
		void stopAll();

	private:
		std::unordered_map<ThreadName, std::shared_ptr<rtc::Thread>> _threadsMap;
		
		std::mutex _mutex;

		rtc::Thread* _mainThread = nullptr;

		std::atomic_bool _destroy;

		friend class core::Singleton<ThreadManager>;
	};
}

#define TMgr vi::ThreadManager::instance()
