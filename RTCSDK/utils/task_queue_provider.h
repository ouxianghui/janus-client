/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include <unordered_map>
#include <mutex>
#include <string>
#include "singleton.h"
#include "api/task_queue/task_queue_factory.h"

namespace webrtc {
	class TaskQueueFactory;
	class TaskQueueBase;
	class TaskQueueDeleter;
}

namespace vi {

	class TaskQueueProvider : public Singleton<TaskQueueProvider>
	{
	public:
		~TaskQueueProvider();

		void init();

		void create(const std::unordered_map<std::string, webrtc::TaskQueueFactory::Priority>& pairs);

		webrtc::TaskQueueBase* queue(const std::string& name);

	private:
		TaskQueueProvider();

		TaskQueueProvider(const TaskQueueProvider&) = delete;

		TaskQueueProvider(TaskQueueProvider&&) = delete;

		TaskQueueProvider& operator=(const TaskQueueProvider&) = delete;

	private:
		std::unique_ptr<webrtc::TaskQueueFactory> _factory;

		std::unordered_map<std::string, std::unique_ptr<webrtc::TaskQueueBase, webrtc::TaskQueueDeleter>> _queuesMap;

		std::mutex _mutex;

		friend class Singleton<TaskQueueProvider>;
	};
}

#define QMgr vi::TaskQueueProvider::instance()

