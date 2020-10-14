/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include <unordered_map>
#include <mutex>

namespace webrtc {
	class TaskQueueFactory;
	class TaskQueueBase;
	class TaskQueueDeleter;
}

namespace vi {
	enum class QueueName : int {
		WORKER = 0
	};

	class TaskQueueManager
	{
	public:
		TaskQueueManager();
		~TaskQueueManager();

		void init();

		webrtc::TaskQueueBase* getQueue(QueueName name);

	private:
		std::unique_ptr<webrtc::TaskQueueFactory> _factory;

		std::unordered_map<QueueName, std::unique_ptr<webrtc::TaskQueueBase, webrtc::TaskQueueDeleter>> _queuesMap;

		std::mutex _mutex;
	};
}

