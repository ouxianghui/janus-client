/**
 * This file is part of janus_client project.
 * Author:    Jackie Ou
 * Created:   2021-3-13s
 **/

#include "task_scheduler.h"

namespace vi {
	std::unique_ptr<webrtc::TaskQueueFactory> TaskScheduler::_factory = webrtc::CreateDefaultTaskQueueFactory();
}