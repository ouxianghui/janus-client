/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#include "task_queue_provider.h"
#include "api/task_queue/default_task_queue_factory.h"
#include "api/task_queue/task_queue_factory.h"
#include "logger/logger.h"

namespace vi {
	TaskQueueProvider::TaskQueueProvider()
	{

	}

	TaskQueueProvider::~TaskQueueProvider()
	{
		DLOG("~TaskQueueProvider()");
	}

	void TaskQueueProvider::init()
	{
		std::lock_guard<std::mutex> lock(_mutex);

		_factory = webrtc::CreateDefaultTaskQueueFactory();
	}

	void TaskQueueProvider::create(const std::unordered_map<std::string, webrtc::TaskQueueFactory::Priority>& pairs)
	{
		std::lock_guard<std::mutex> lock(_mutex);

		if (_factory) {
			for (const auto& pair : pairs) {
				_queuesMap[pair.first] = _factory->CreateTaskQueue(pair.first, pair.second);
			}
		}
		else {
			DLOG("_factory == nullptr");
		}
	}

	webrtc::TaskQueueBase* TaskQueueProvider::queue(const std::string& name)
	{
		std::lock_guard<std::mutex> lock(_mutex);

		if (_queuesMap.find(name) != _queuesMap.end()) {
			return _queuesMap[name].get();
		}

		return nullptr;
	}
}
