/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include <unordered_map>
#include <mutex>
#include "utils/singleton.h"

namespace webrtc {
	class TaskQueueFactory;
	class TaskQueueBase;
	class TaskQueueDeleter;
}

namespace vi {
	enum class QueueName : int {
		CORE = 0
	};

	class TaskQueueManager : public core::Singleton<TaskQueueManager>
	{
	public:
		~TaskQueueManager();

		webrtc::TaskQueueBase* queue(QueueName name);

	private:
		void init();

	private:
		TaskQueueManager();

		TaskQueueManager(const TaskQueueManager&) = delete;

		TaskQueueManager(TaskQueueManager&) = delete;

		TaskQueueManager& operator=(const TaskQueueManager&) = delete;

	private:
		std::unique_ptr<webrtc::TaskQueueFactory> _factory;

		std::unordered_map<QueueName, std::unique_ptr<webrtc::TaskQueueBase, webrtc::TaskQueueDeleter>> _queuesMap;

		std::mutex _mutex;

		friend class core::Singleton<TaskQueueManager>;
	};
}

#define QMgr vi::TaskQueueManager::instance()

