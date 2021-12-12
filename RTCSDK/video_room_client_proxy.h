#pragma once

#include "weak_proxy.h"

namespace vi {

	BEGIN_WEAK_PROXY_MAP(VideoRoomClient)
		WEAK_PROXY_THREAD_DESTRUCTOR()
		WEAK_PROXY_METHOD0(void, init)
		WEAK_PROXY_METHOD0(void, destroy)
		WEAK_PROXY_METHOD1(void, registerEventHandler, std::shared_ptr<IVideoRoomEventHandler>)
		WEAK_PROXY_METHOD1(void, unregisterEventHandler, std::shared_ptr<IVideoRoomEventHandler>)
		WEAK_PROXY_METHOD1(void, create, std::shared_ptr<vr::CreateRoomRequest>)
		WEAK_PROXY_METHOD1(void, join, std::shared_ptr<vr::PublisherJoinRequest>)
		WEAK_PROXY_METHOD1(void, leave, std::shared_ptr<vr::LeaveRequest>)
		WEAK_PROXY_METHOD0(std::shared_ptr<ParticipantsContrllerInterface>, participantsController)
		WEAK_PROXY_METHOD0(std::shared_ptr<MediaControllerInterface>, mediaContrller)
	END_WEAK_PROXY_MAP()

}