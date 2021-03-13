/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2021-3-13s
 **/

#include "task_scheduler.h"
#include "rtc_base/thread.h"

namespace vi {
	std::unique_ptr<webrtc::TaskQueueFactory> TaskScheduler::_factory = webrtc::CreateDefaultTaskQueueFactory();

	std::shared_ptr<TaskScheduler> TaskScheduler::create()
	{
		return std::shared_ptr<TaskScheduler>(new TaskScheduler(), [thread = rtc::Thread::Current()](TaskScheduler* ptr){
			thread->PostTask(RTC_FROM_HERE, [ptr]() {
				delete ptr;
			});
		});
	}
}