/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2020-10-01
 **/

#pragma once

#include <memory>
#include <functional>
#include <unordered_set>
#include <unordered_map>
#include <mutex>
#include "api/task_queue/queued_task.h"
#include "api/task_queue/task_queue_base.h"
#include "api/task_queue/task_queue_factory.h"
#include "api/task_queue/default_task_queue_factory.h"
#include "rtc_base/helpers.h"

namespace vi {
	class TaskScheduler;
	template<class Closure>
	class OneShotTask: public webrtc::QueuedTask {
	public:
		explicit OneShotTask(Closure&& closure, uint64_t id, std::weak_ptr<TaskScheduler> scheduler)
			: _closure(std::forward<Closure>(closure))
			, _id(id)
			, _scheduler(scheduler) {
		}

		bool Run() override {
			if (auto scheduler = _scheduler.lock()) {
				std::unordered_set<uint64_t> ids = scheduler->getTaskIds();
				if (ids.find(_id) != ids.end()) {
					_closure();
				}
			}
			return true;
		}

		uint16_t getTaskId() {
			return _id;
		}

	private:
		typename std::decay<Closure>::type _closure;
		const uint64_t _id;
		std::weak_ptr<TaskScheduler> _scheduler;
	};

	template<class Closure>
	class RepetitiveTask : public webrtc::QueuedTask {
	public:
		explicit RepetitiveTask(Closure&& closure, uint32_t milliseconds, uint64_t id, std::weak_ptr<TaskScheduler> scheduler, webrtc::TaskQueueBase* queue)
			: _closure(std::forward<Closure>(closure))
			, _milliseconds(milliseconds)
			, _id(id)
			, _scheduler(scheduler)
			, _queue(queue) {
		}

		bool Run() override {
			bool cancelled = true;
			if (auto scheduler = _scheduler.lock()) {
				std::unordered_set<uint64_t> ids = scheduler->getTaskIds();
				if (ids.find(_id) != ids.end()) {
					cancelled = false;
				}
			}

			if (!cancelled) {
				_closure();
				_queue->PostDelayedTask(absl::WrapUnique(this), _milliseconds);
				return false;
			}
			else {
				return true;
			}
		}

	private:
		typename std::decay<Closure>::type _closure;
		const uint32_t _milliseconds;
		const uint64_t _id;
		webrtc::TaskQueueBase* const _queue;
		std::weak_ptr<TaskScheduler> _scheduler;
	};

	class TaskScheduler : public std::enable_shared_from_this<TaskScheduler> {
	public:
		static std::shared_ptr<TaskScheduler> create();

		~TaskScheduler() {
			cancelAll();
		}

		template <class Closure>
		uint64_t schedule(Closure&& closure, uint32_t milliseconds = 0, bool repetitive = false) {
			if (!repetitive) {
				return scheduleOneShotTask(std::forward<Closure>(closure), milliseconds);
			}
			else {
				return scheduleRepetitiveTask(std::forward<Closure>(closure), milliseconds);
			}
		}

		void cancel(uint64_t id) {
			std::lock_guard<std::mutex> lock(_mutex);
			if (_taskIdSet.find(id) != _taskIdSet.end()) {
				_taskIdSet.erase(id);
			}
		}

		void cancelAll() {
			std::lock_guard<std::mutex> lock(_mutex);
			_taskIdSet.clear();
		}

		const std::unordered_set<uint64_t>& getTaskIds() {
			std::lock_guard<std::mutex> lock(_mutex);
			return _taskIdSet;
		}

	private:
		TaskScheduler() {
			init();
		}

		void init() {
			std::string schedulerId = "post-" + std::to_string((uint64_t)this);
			_taskQueue = _factory->CreateTaskQueue(schedulerId, webrtc::TaskQueueFactory::Priority::NORMAL);
		}

		template <class Closure>
		uint64_t scheduleOneShotTask(Closure&& closure, uint32_t milliseconds) {
			uint64_t id = rtc::CreateRandomId64();
			auto task = createOneShotTask(std::forward<Closure>(closure), id);
			{
				std::lock_guard<std::mutex> lock(_mutex);
				_taskIdSet.emplace(id);
			}
			_taskQueue->PostDelayedTask(std::move(task), milliseconds);

			return id;
		}

		template <class Closure>
		uint64_t scheduleRepetitiveTask(Closure&& closure, uint32_t milliseconds) {
			uint64_t id = rtc::CreateRandomId64();
			auto task = createRepetitiveTask(std::forward<Closure>(closure), milliseconds, id);
			{
				std::lock_guard<std::mutex> lock(_mutex);
				_taskIdSet.emplace(id);
			}
			_taskQueue->PostDelayedTask(std::move(task), milliseconds);

			return id;
		}

		template <typename Closure>
		std::unique_ptr<webrtc::QueuedTask> createOneShotTask(Closure&& closure, uint64_t id) {
			return std::make_unique<OneShotTask<Closure>>(std::forward<Closure>(closure), id, weak_from_this());
		}

		template <typename Closure>
		std::unique_ptr<webrtc::QueuedTask> createRepetitiveTask(Closure&& closure, uint32_t milliseconds, uint64_t id) {
			return std::make_unique<RepetitiveTask<Closure>>(std::forward<Closure>(closure), milliseconds, id, weak_from_this(), _taskQueue.get());
		}

	private:
		std::mutex _mutex;
		std::unordered_set<uint64_t> _taskIdSet;
		static std::unique_ptr<webrtc::TaskQueueFactory> _factory;
		std::unique_ptr<webrtc::TaskQueueBase, webrtc::TaskQueueDeleter> _taskQueue;
	};

}