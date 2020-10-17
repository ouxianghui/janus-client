/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#include "task_queue_manager.h"
#include "api/task_queue/default_task_queue_factory.h"
#include "api/task_queue/task_queue_factory.h"
#include "logger/logger.h"

namespace vi {
	TaskQueueManager::TaskQueueManager()
	{
		init();
	}

	TaskQueueManager::~TaskQueueManager()
	{
		DLOG("~TaskQueueManager()");
	}

	void TaskQueueManager::init()
	{
		std::lock_guard<std::mutex> lock(_mutex);

		_factory = webrtc::CreateDefaultTaskQueueFactory();

		_queuesMap[QueueName::CORE] = _factory->CreateTaskQueue("vi::CORE", webrtc::TaskQueueFactory::Priority::NORMAL);
	}

	webrtc::TaskQueueBase* TaskQueueManager::queue(QueueName name)
	{
		std::lock_guard<std::mutex> lock(_mutex);

		if (_queuesMap.find(name) != _queuesMap.end()) {
			return _queuesMap[name].get();
		}

		return nullptr;
	}
}
